#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <nrf_drv_ppi.h>
#include <nrf_drv_timer.h>
#include <nrf_gpio.h>
#include <nrf_gpiote.h>
#include <app_util_platform.h>

#include <servo.h>



/*
For info: In Spektrum receivers 11 bit data (0..2047) is mapped from 900us..2100us.


In the headless transmitter the normalized input channels are clamped to
-10000..0..10000, while the output channels can go up to -18000..0..18000
(-180%..0%..180%), corresponding to receiver pulses of 600us..1500us..2400us

Over the air we map the -18000..18000 range into 12 bits (because 12 is 3 nibbles
and therefore easy to handle). We treat the 12 bits as unsigned value, so the output
channel value -18000 translates to over-the-air value 0, and +18000 to value 4095,
with center being output channel value 0 and over-the-air value 2048.

To convert we need the following formula:

    over-the-air = (output_channel + 18000) * 4096 / 36000;
=>
    over-the-air = (output_channel + 18000) << 7 / 1125;


On the receiver side we need to convert 0..4095 into 600..2400us. Since we run
the timer at 2 MHz (0.5 us), we need to work in half-us units

    us = 600 + (value * 1800 / 4096)
=>
    half_us = 1200 + ((value * 225) >> 8)

*/

#define US_TO_TIMER(x) (x*2)

#define NUMBER_OF_SERVO_BANKS 2

#define SERVO_PULSE_PERIOD (US_TO_TIMER(16000)/NUMBER_OF_SERVO_BANKS)
#define SERVO_PULSE_MIN (US_TO_TIMER(600))
#define SERVO_PULSE_CENTER (US_TO_TIMER(1500))
#define SERVO_PULSE_MAX (US_TO_TIMER(2400))

static const nrf_drv_timer_t pwm_timer = NRF_DRV_TIMER_INSTANCE(2);

extern volatile uint32_t milliseconds;

static uint8_t state;
static nrf_ppi_channel_t ppi_channels[4];

static uint16_t servo[4] = {US_TO_TIMER(1500), US_TO_TIMER(1200), US_TO_TIMER(1700), US_TO_TIMER(2000)};

static const uint8_t cc_channels[] = {NRF_TIMER_CC_CHANNEL0, NRF_TIMER_CC_CHANNEL1, NRF_TIMER_CC_CHANNEL2, NRF_TIMER_CC_CHANNEL3};

static uint8_t servo_bank_select = 0;
static const uint32_t servo_banks[NUMBER_OF_SERVO_BANKS][4] = {
    {GPIO_SERVO_1, GPIO_SERVO_2, GPIO_SERVO_3, GPIO_SERVO_4},
    {GPIO_SERVO_5, GPIO_SERVO_6, GPIO_SERVO_7, GPIO_SERVO_8},
};

static bool running = false;


// ****************************************************************************
/*

The nRF51822 does not have direct PWM hardware. Instead PWM can be implemented
using the TIMER, PPI and GPIOTE modules.

However, resources are very limited: the GPIOTE module only has 4 instances.

For our servo pulse output application we can work around that with "manual
labour".

Initialization:
    // This step is only done once when starting up the PWM. We assime that that
    // all GPIO are HIGH.
    Set CC0 to 5 ms and enable NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK
    Force all GPIO to LOW via GPIOTE
    Set all GPIOTE0..3 to set the GPIO to TOGGLE
    Set PPI0..3 to trigger on CC0
    Enable CC0 interrupt
    Clear the timer
    Start the timer
    set State to 1

State 1 (at timer count 0):
    // Triggered when the compare interrupt fires
    // Servo outputs all go HIGH because CC0..3 were set to the same value.
    Set the CC0..3 to the respective servo pulse duration
    set State to 2

State 2 (at time of servo pulse CC0):
    // Triggered when the compare interrupt fires because the servo pulse CC0 is
    // done. We reuse CC0 to time the remainder servo pulse frequency.
    Set CC0 to SERVO_PULSE_MAX + 1000
    Disable PPI0 being trigged by CC0
    set State to 3

State 3 (at time of servo pulse CC0):
    // Triggered when the compare interrupt fires. By now all servo pulses must
    // have been completed (GPIO set back to LOW), because the longest servo pulse we output is SERVO_PULSE_MAX
    // and State 3 fired at SERVO_PULSE_MAX + 1000
    Set CC0 to SERVO_PULSE_PERIOD and enable NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK
    Set CC1..3 to SERVO_PULSE_PERIOD
    Enable PPI0 being trigged by CC0
    set State to 1


This sequence can easily be extended to more servo channels by chaning GPIOTE
in Step 3. Of course the repeat frequency halfes, but for up to 12 channels
this is not an issue.


IMPORTANT:
This implementation does not use the GPIOTE driver, but manages GPIOTE directly.
This is necessary as the GPIOTE driver sets GPIOs back to disconnected when we
swap between different ports, causing glitches on the pins.

*/
// ****************************************************************************

// ****************************************************************************
// Timer interrupt, called by the nrf timer driver
static void pwm_timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    switch (state) {
        case 1:
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, servo[0], 0, true);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, servo[1], 0, false);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, servo[2], 0, false);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, servo[3], 0, false);
            state = 2;
            break;

        case 2:
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_MAX + 1000, 0, true);
            nrf_drv_ppi_channel_disable(ppi_channels[0]);
            state = 3;
            break;

        case 3:
        default:
            servo_bank_select = (servo_bank_select + 1) % NUMBER_OF_SERVO_BANKS;

            for (int i = 0; i < 4; i++) {
                nrf_gpiote_task_configure(i, servo_banks[servo_bank_select][i], NRF_GPIOTE_POLARITY_TOGGLE, 0) ;
                nrf_gpiote_task_enable(i);
            }

            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_PERIOD, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, SERVO_PULSE_PERIOD, 0, false);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, SERVO_PULSE_PERIOD, 0, false);
            nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, SERVO_PULSE_PERIOD, 0, false);
            nrf_drv_ppi_channel_enable(ppi_channels[0]);

            state = 1;
            break;
    }
}


// ****************************************************************************
void SERVO_init(void)
{
    nrf_drv_timer_config_t timer_config  = {
        .frequency          = NRF_TIMER_FREQ_2MHz,
        .mode               = NRF_TIMER_MODE_TIMER,
        .bit_width          = NRF_TIMER_BIT_WIDTH_16,
        .interrupt_priority = APP_IRQ_PRIORITY_LOW,
        .p_context          = NULL,
    };

    nrf_drv_ppi_init();

    for (int i = 0; i < 4; i++) {
        nrf_gpiote_task_configure(i, servo_banks[servo_bank_select][i], NRF_GPIOTE_POLARITY_TOGGLE, 0) ;
        nrf_gpiote_task_enable(i);

        nrf_drv_ppi_channel_alloc(&ppi_channels[i]);
        nrf_drv_ppi_channel_assign(ppi_channels[i],
            nrf_drv_timer_compare_event_address_get(&pwm_timer, cc_channels[i]),
            nrf_gpiote_task_addr_get(NRF_GPIOTE_TASKS_OUT_0 + (sizeof(uint32_t) * i)));
        nrf_drv_ppi_channel_enable(ppi_channels[i]);
    }

    nrf_drv_timer_init(&pwm_timer, &timer_config, pwm_timer_handler);
    nrf_drv_timer_clear(&pwm_timer);

    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_PERIOD, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, SERVO_PULSE_PERIOD, 0, false);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, SERVO_PULSE_PERIOD, 0, false);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, SERVO_PULSE_PERIOD, 0, false);
}


// ****************************************************************************
void SERVO_start(void)
{
    if (running) {
        return;
    }

    running = true;

    nrf_drv_timer_enable(&pwm_timer);
    state = 1;

    for (int i = 0; i < 4; i++) {
        nrf_gpio_pin_clear(servo_banks[0][i]);
        nrf_gpio_pin_clear(servo_banks[1][i]);
    }
}


// ****************************************************************************
void SERVO_set(uint8_t index, uint16_t value_12bit)
{
    uint32_t half_us;

    // Optimized form of
    // half_us = 2 * (600 + (value * 1800 / 4096))
    half_us = 1200 + (((uint32_t)value_12bit * 225) >> 8);
    servo[index] = half_us;
}


// ****************************************************************************
void SERVO_process(void)
{
    static uint32_t next = 50;

    if (milliseconds > 1000) {
        if (!running) {
            SERVO_set(0, 0);
            SERVO_set(1, 0xfff);
            SERVO_set(2, 0x800);
            SERVO_set(3, 0xa00);
        }
        SERVO_start();
    }


    if (milliseconds >= next) {
        int i;
        static bool up[4] = {true, false, true, false};

        next += 5;

        for (i = 0; i < 4; i++) {
            if (up[i]) {
                if (servo[i] < US_TO_TIMER(2000)) {
                    ++servo[i];
                }
                else {
                    up[i] = false;
                    --servo[i];
                }
            }
            else {
                if (servo[i] > US_TO_TIMER(1000)) {
                    --servo[i];
                }
                else {
                    up[i] = true;
                    ++servo[i];
                }
            }
        }
    }
}

