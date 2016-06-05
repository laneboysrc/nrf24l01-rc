#include <stdint.h>
#include <stdio.h>

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <config.h>
#include <inputs.h>
#include <inputs_stm32f103c8t6.h>
#include <systick.h>


#define WINDOW_SIZE 10
#define SAMPLE_COUNT NUMBER_OF_ADC_CHANNELS * WINDOW_SIZE

static uint16_t adc_array_oversample[SAMPLE_COUNT];
static uint16_t adc_array_raw[NUMBER_OF_ADC_CHANNELS];
static uint16_t adc_array_calibrated[NUMBER_OF_ADC_CHANNELS];
static uint8_t adc_channel_selection[NUMBER_OF_ADC_CHANNELS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 17};

static int32_t normalized_inputs[NUMBER_OF_ADC_CHANNELS];


// ****************************************************************************
// static uint32_t adc_read_channel(unsigned channel)
// {
//     uint8_t channel_array[1];
//     // Select the channel we want to convert
//     channel_array[0] = channel;
//     adc_set_regular_sequence(ADC1, 1, channel_array);


//     // If the ADC_CR2_ON bit is already set -> setting it another time
//     // starts the conversion.

//     adc_start_conversion_direct(ADC1);

//     // Wait for end of conversion.
//     while (! adc_eoc(ADC1));

//     return adc_read_regular(ADC1);
// }



// ****************************************************************************
static void adc_set_conversion_sequence(void)
{
    adc_set_regular_sequence(ADC1, NUMBER_OF_ADC_CHANNELS, (uint8_t *)adc_channel_selection);
    adc_set_continuous_conversion_mode(ADC1);
    adc_start_conversion_direct(ADC1);
}


// ****************************************************************************
static void adc_init(void)
{
    rcc_periph_clock_enable(RCC_ADC1);

    adc_power_off(ADC1);

    rcc_periph_reset_pulse(RST_ADC1);
    rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV2);     // 12MHz ADC clock

    /* We configure to scan the entire group each time conversion is requested. */
    adc_enable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_disable_discontinuous_mode_regular(ADC1);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);

    // 239.5 + 12.5 cycles @ 12 MHz ADC clock means 21 us per conversion
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_239DOT5CYC);

    adc_power_on(ADC1);
    adc_reset_calibration(ADC1);
    adc_calibration(ADC1);

    // Enable the voltage reference and temperature sensor inputs
    adc_enable_temperature_sensor(ADC1);

    adc_set_conversion_sequence();


    // DMA set-up. This enables reading all ADC channels round-robin,
    // filling up adc_array_oversample and then starting all over. No software
    // involvement needed.
    // Whenever the software wants ADC data, it reads adc_array_oversample.
    //
    // The code is based on code from http://code.google.com/p/rayaairbot
    rcc_periph_clock_enable(RCC_DMA1);

    dma_enable_circular_mode(DMA1, DMA_CHANNEL1);
    dma_set_read_from_peripheral(DMA1, DMA_CHANNEL1);
    dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
    dma_set_peripheral_address(DMA1, DMA_CHANNEL1,(uint32_t) &ADC_DR(ADC1));
    dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);
    dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t) &adc_array_oversample);
    dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);
    dma_set_number_of_data(DMA1, DMA_CHANNEL1, SAMPLE_COUNT);

    // The DMA is now ready to go, waiting for the ADC to provide data
    dma_enable_channel(DMA1, DMA_CHANNEL1);
    adc_enable_dma(ADC1);
}


// ****************************************************************************
void INPUTS_init(void)
{
    adc_init();

    // Configure the analog inputs
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);

    // FIXME: this needs to be dynamically based on whether an input is
    // used at all, used as analog input, or used as digital input.
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO2);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO3);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO4);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO5);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO6);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO7);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
}


// ****************************************************************************
// Returns the battery voltage in millivolts
// Since the ADC uses VDD (3.3V) as reference, we measure the internal 1.2V
// reference (ADC17) of the STM32 to determine how much voltage one bit
// represents without having to rely on the 3.3V accuracy.
//
// We then multiply the measured battery voltage on ADC0 by this value and
// scale the result by the resistor divider (2k2 over 3k3).
// The calulation is carried out in uV to get maximum resolution.
//
// one_bit_voltage = 1.2V / ADC(17)
// battery_voltage = ADC(0) * one_bit_voltage * ((2200 + 3300) / 3300)
//
// This algorithm was verified with a Keithley 2700 6.5 digit DMM over the
// whole discharge of a Li-Ion battery, from 4.15V down to 2.5V. It is
// surprisingly accurate!
//
uint32_t INPUTS_get_battery_voltage(void)
{
    return adc_array_raw[0] * (33 + 22) * 1200000 / adc_array_raw[10] / 33000;
}


// ****************************************************************************
void INPUTS_filter_and_normalize(void)
{
    for (int i = 0; i < NUMBER_OF_ADC_CHANNELS; i++) {
        uint16_t filtered_result = 0;
        int idx = i;

        for (int j = 0; j < WINDOW_SIZE; j++) {
            filtered_result += adc_array_oversample[idx];
            idx += NUMBER_OF_ADC_CHANNELS;
        }

        filtered_result /= WINDOW_SIZE;
        adc_array_raw[i] = filtered_result;
    }

    for (int i = 0; i < MAX_TRANSMITTER_INPUTS; i++) {
        uint32_t raw;

        transmitter_input_t *t = &config.tx.transmitter_inputs[i];
        switch (t->type) {
            case ANALOG_WITH_CENTER:
            case ANALOG_NO_CENTER:
            case ANALOG_NO_CENTER_POSITIVE_ONLY:
                raw = adc_array_raw[t->input];
                if (raw < t->calibration[0]) {
                    adc_array_calibrated[t->input] = 0;
                }
                else if (raw > t->calibration[2]) {
                    adc_array_calibrated[t->input] = ADC_VALUE_MAX;
                }
                else if (raw == t->calibration[1]) {
                    adc_array_calibrated[t->input] = ADC_VALUE_HALF;
                }
                else if (raw > t->calibration[1]) {
                    adc_array_calibrated[t->input] = ADC_VALUE_HALF + (raw - t->calibration[1]) * ADC_VALUE_HALF / (t->calibration[2] - t->calibration[1]);
                }
                else {
                    adc_array_calibrated[t->input] = (raw - t->calibration[0]) * ADC_VALUE_HALF / (t->calibration[1] - t->calibration[0]);
                }

                normalized_inputs[t->input] = (adc_array_calibrated[t->input] - ADC_VALUE_HALF) * CHANNEL_100_PERCENT / ADC_VALUE_HALF;
                break;

            case TRANSMITTER_INPUT_NOT_USED:
            case DIGITAL_ACTIVE_LOW:
            case DIGITAL_ACTIVE_HIGH:
            default:
                break;
        }
    }
}


// ****************************************************************************
int32_t INPUTS_get_input(label_t input)
{
    for (unsigned i = 0; i < MAX_LOGICAL_INPUTS; i++) {
        logical_input_t *li = &config.tx.logical_inputs[i];

        for (unsigned j = 0; j < MAX_LABELS; j++) {
            if (li->labels[j] == input) {
                return normalized_inputs[li->inputs[0]];
            }
        }
    }

    return 0;
}


// ****************************************************************************
void INPUTS_dump_adc(void)
{
    // printf("BAT: %lumV  ", INPUTS_get_battery_voltage());
    // for (int i = 1; i <= 4; i++) {
    //     printf("CH%d:%4ld%% (%4u->%4u)  ", i, CHANNEL_TO_PERCENT(normalized_inputs[i]), adc_array_raw[i], adc_array_calibrated[i]);
    // }
    // printf("\n");

    printf("%lu, %u, %u\n", INPUTS_get_battery_voltage(), adc_array_raw[0], adc_array_raw[10]);

}
