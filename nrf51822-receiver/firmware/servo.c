#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <nrf_drv_gpiote.h>
#include <nrf_drv_ppi.h>
#include <nrf_drv_timer.h>
#include <nrf_gpio.h>
#include <nrf_gpiote.h>
#include <app_util_platform.h>

#include <servo.h>

#define SERVO_1 (19)
#define SERVO_2 (20)
#define SERVO_3 (21)
#define SERVO_4 (22)

#define PWM_TIMER 2

#define SERVO_PULSE_MIN 600
#define SERVO_PULSE_CENTER 1500
#define SERVO_PULSE_MAX 2400

#define SERVO_PULSE_PERIOD 16000

static const nrf_drv_timer_t pwm_timer = NRF_DRV_TIMER_INSTANCE(PWM_TIMER);
static const nrf_drv_gpiote_out_config_t gpiote_out_config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(0);

extern volatile uint32_t milliseconds;

static uint8_t step = 0;
static nrf_ppi_channel_t ppi_channels[4];

static uint16_t servo[4] = {1500, 1200, 1200, 2000};
static bool up[4] = {true, false, true, false};

// ****************************************************************************
/*

The nRF51822 does not have direct PWM hardware. Instead PWM can be implemented
using the TIMER, PPI and GPIOTE modules.

However, resources are very limited: the GPIOTE module only has 4 instances.

For our servo pulse output application we can work around that with "manual
labour".

Step 0:
    // This step is only done once when starting up the PWM. We assime that that
    // all GPIO are HIGH.
    Set CC0 to 5 ms and enable NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK
    Force all GPIO to LOW via GPIOTE
    Set all GPIOTE0..3 to set the GPIO to TOGGLE
    Set PPI0..3 to trigger on CC0
    Enable CC0 interrupt
    Clear the timer
    Start the timer
    set Step to 1

Step 1 (at timer count 0):
    // Triggered when the compare interrupt fires
    Set the CC0..3 to the respective servo pulse duration
    Set PPI0..3 to trigger on CC0..3
    set Step to 2

Step 2 (at time of servo pulse CC0):
    // Triggered when the compare interrupt fires because the servo pulse CC0 is
    // done. We reuse CC0 to time the remainder servo pulse frequency.
    Set CC0 to 4 ms
    Disable PPI0 trigged by CC0
    set Step to 3

Step 3 (at time of servo pulse CC0):
    // Triggered when the compare interrupt fires. By now all servo pulses must
    // have been completed, because the longest servo pulse we output is 2.5 ms
    Set CC0 to 5 ms and enable NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK
    Set PPI0..3 to trigger on CC0
    set Step to 1


This sequence can easily be extended to more servo channels by chaning GPIOTE
in Step 3. Of course the repeat frequency halfes, but for up to 12 channels
this is not an issue.


*/
// ****************************************************************************

// ****************************************************************************
// Timer interrupt, called by the nrf timer driver
static void pwm_timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    // printf("S%d %lu\n", step, milliseconds);
    if (step == 1) {
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, servo[0], 0, true);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, servo[1], 0, false);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, servo[2], 0, false);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, servo[3], 0, false);
        step = 2;
    }
    else if (step == 2) {
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_MAX + 1000, 0, true);
        nrf_drv_ppi_channel_disable(ppi_channels[0]);
        step = 3;
    }
    else if (step == 3) {
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_PERIOD, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, SERVO_PULSE_PERIOD, 0, false);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, SERVO_PULSE_PERIOD, 0, false);
        nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, SERVO_PULSE_PERIOD, 0, false);
        nrf_drv_ppi_channel_enable(ppi_channels[0]);
        step = 1;
    }
}

// ****************************************************************************
void SERVO_init(void)
{
    uint32_t ret;

    nrf_drv_timer_config_t timer_config  = {
        .frequency          = NRF_TIMER_FREQ_1MHz,
        .mode               = NRF_TIMER_MODE_TIMER,
        .bit_width          = NRF_TIMER_BIT_WIDTH_16,
        .interrupt_priority = APP_IRQ_PRIORITY_LOW,
        .p_context          = (void *) (uint32_t) PWM_TIMER,
    };


    printf("SERVO_init()\n");

    ret = nrf_drv_ppi_init();
    if ((ret != NRF_SUCCESS) && (ret != MODULE_ALREADY_INITIALIZED)) {
        printf("Failed to initialize PPI: err=%ld", ret);
    }

    ret = nrf_drv_gpiote_init();
    if (ret != NRF_SUCCESS) {
        printf("Failed to initialize GPIOTE: err=%ld", ret);
    }

    ret = nrf_drv_gpiote_out_init(SERVO_1, &gpiote_out_config);
    if (ret != NRF_SUCCESS) {
        printf("Failed to initialize GPIOTE for Servo 1: err=%ld", ret);
    }
    nrf_drv_gpiote_out_task_force(SERVO_1, 1);
    nrf_drv_gpiote_out_task_enable(SERVO_1);

    ret = nrf_drv_gpiote_out_init(SERVO_2, &gpiote_out_config);
    if (ret != NRF_SUCCESS) {
        printf("Failed to initialize GPIOTE for Servo 2: err=%ld", ret);
    }
    nrf_drv_gpiote_out_task_force(SERVO_2, 1);
    nrf_drv_gpiote_out_task_enable(SERVO_2);

    ret = nrf_drv_gpiote_out_init(SERVO_3, &gpiote_out_config);
    if (ret != NRF_SUCCESS) {
        printf("Failed to initialize GPIOTE for Servo 3: err=%ld", ret);
    }
    nrf_drv_gpiote_out_task_force(SERVO_3, 1);
    nrf_drv_gpiote_out_task_enable(SERVO_3);

    ret = nrf_drv_gpiote_out_init(SERVO_4, &gpiote_out_config);
    if (ret != NRF_SUCCESS) {
        printf("Failed to initialize GPIOTE for Servo 4: err=%ld", ret);
    }
    nrf_drv_gpiote_out_task_force(SERVO_4, 1);
    nrf_drv_gpiote_out_task_enable(SERVO_4);

    ret = nrf_drv_ppi_channel_alloc(&ppi_channels[0]);
    if (ret != NRF_SUCCESS) {
        printf("Failed to allocate PPI channel 0: err=%ld", ret);
    }
    ret = nrf_drv_ppi_channel_alloc(&ppi_channels[1]);
    if (ret != NRF_SUCCESS) {
        printf("Failed to allocate PPI channel 1: err=%ld", ret);
    }
    ret = nrf_drv_ppi_channel_alloc(&ppi_channels[2]);
    if (ret != NRF_SUCCESS) {
        printf("Failed to allocate PPI channel 2: err=%ld", ret);
    }
    ret = nrf_drv_ppi_channel_alloc(&ppi_channels[3]);
    if (ret != NRF_SUCCESS) {
        printf("Failed to allocate PPI channel 3: err=%ld", ret);
    }

    nrf_drv_ppi_channel_assign(ppi_channels[0],
        nrf_drv_timer_compare_event_address_get(&pwm_timer, NRF_TIMER_CC_CHANNEL0),
        nrf_drv_gpiote_out_task_addr_get(SERVO_1));
    nrf_drv_ppi_channel_assign(ppi_channels[1],
        nrf_drv_timer_compare_event_address_get(&pwm_timer, NRF_TIMER_CC_CHANNEL1),
        nrf_drv_gpiote_out_task_addr_get(SERVO_2));
    nrf_drv_ppi_channel_assign(ppi_channels[2],
        nrf_drv_timer_compare_event_address_get(&pwm_timer, NRF_TIMER_CC_CHANNEL2),
        nrf_drv_gpiote_out_task_addr_get(SERVO_3));
    nrf_drv_ppi_channel_assign(ppi_channels[3],
        nrf_drv_timer_compare_event_address_get(&pwm_timer, NRF_TIMER_CC_CHANNEL3),
        nrf_drv_gpiote_out_task_addr_get(SERVO_4));


    nrf_drv_ppi_channel_enable(ppi_channels[0]);
    nrf_drv_ppi_channel_enable(ppi_channels[1]);
    nrf_drv_ppi_channel_enable(ppi_channels[2]);
    nrf_drv_ppi_channel_enable(ppi_channels[3]);


    ret = nrf_drv_timer_init(&pwm_timer, &timer_config, pwm_timer_handler);
    if (ret != NRF_SUCCESS) {
        printf("Failed to initalize the timer: err=%ld", ret);
    }

    nrf_drv_timer_clear(&pwm_timer);

    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL0, SERVO_PULSE_PERIOD, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL1, SERVO_PULSE_PERIOD, 0, false);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL2, SERVO_PULSE_PERIOD, 0, false);
    nrf_drv_timer_extended_compare(&pwm_timer, NRF_TIMER_CC_CHANNEL3, SERVO_PULSE_PERIOD, 0, false);

    nrf_drv_timer_enable(&pwm_timer);

    nrf_drv_gpiote_out_task_force(SERVO_1, 0);
    nrf_drv_gpiote_out_task_force(SERVO_2, 0);
    nrf_drv_gpiote_out_task_force(SERVO_3, 0);
    nrf_drv_gpiote_out_task_force(SERVO_4, 0);
    step = 1;
}

void SERVO_process(void)
{
    static uint32_t next = 50;

    if (milliseconds >= next) {
        int i;

        next += 5;

        for (i = 0; i < 4; i++) {
            if (up[i]) {
                if (servo[i] < 2000) {
                    ++servo[i];
                }
                else {
                    up[i] = false;
                    --servo[i];
                }
            }
            else {
                if (servo[i] > 1000) {
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

