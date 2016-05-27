#include <stdint.h>

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <inputs.h>


#define NUMBER_OF_ADC_CHANNELS 10
#define WINDOW_SIZE 10
#define SAMPLE_COUNT NUMBER_OF_ADC_CHANNELS * WINDOW_SIZE

static uint32_t adc_array_oversample[SAMPLE_COUNT];
static uint32_t adc_array_raw[NUMBER_OF_ADC_CHANNELS];
static uint8_t adc_channel_selection[NUMBER_OF_ADC_CHANNELS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};


// ****************************************************************************
static void adc_filter(void)
{
    for (int i = 0; i < NUMBER_OF_ADC_CHANNELS; i++) {
        uint32_t result = 0;
        int idx = i;

        for (int j = 0; j < WINDOW_SIZE; j++) {
            result += adc_array_oversample[idx];
            idx += NUMBER_OF_ADC_CHANNELS;
        }

        result /= WINDOW_SIZE;
        adc_array_raw[i] = result;
    }
}


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


    // // Build a RNG seed using ADC 14, 16, 17
    // adc_enable_temperature_sensor(ADC1);

    // for(int i = 0; i < 8; i++) {
    //     uint32_t seed;
    //     seed = ((adc_read_channel(16) & 0x03) << 2) | (adc_read_channel(17) & 0x03); //Get 2bits of RNG from Temp and Vref
    //     seed ^= adc_read_channel(0) << i; //Get a couple more random bits from Voltage sensor
    //     // rand32_r(0, seed);
    // }
    // // printf("RNG Seed: %08x\n", (int)rand32());

    //This is important.  We're using the temp value as a buffer because otherwise the channel data
    //Can bleed into the voltage-sense data.
    //By disabling the temperature, we always read a consistent value
    // adc_disable_temperature_sensor(ADC1);

    adc_set_conversion_sequence();


    // DMA set-up
    /* The following is based on code from here: http://code.google.com/p/rayaairbot */
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
void init_inputs(void)
{
    adc_init();

    // Configure the analog inputs
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);

    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO2);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO3);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO4);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO5);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO6);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO7);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);


    // FIXME: just put this here to prevent compiler warning
    adc_filter();
}
