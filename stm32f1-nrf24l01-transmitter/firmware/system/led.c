#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>

#include <config.h>
#include <led.h>
#include <systick.h>


static bool led_on;
static bool led_flashing;

static uint32_t flash_period_start_ms;


#define LED_PWM_MODULO 10
#define LED_ON_VALUE 3
#define FLASH_PERIOD_MS 200


// ****************************************************************************
void LED_on(void)
{
    led_on = true;
    led_flashing = false;
}


// ****************************************************************************
void LED_off(void)
{
    led_on = false;
    led_flashing = false;
}


// ****************************************************************************
void LED_flashing(void)
{
    if (led_flashing) {
        return;
    }

    led_on = true;
    led_flashing = true;
    flash_period_start_ms = milliseconds;
}


// ****************************************************************************
void LED_systick_callback(void)
{
    uint8_t led_brightness;

    if (led_flashing) {
        if ((milliseconds - flash_period_start_ms) >= FLASH_PERIOD_MS) {
            flash_period_start_ms = milliseconds;
            led_on = !led_on;
        }
    }

    led_brightness = config.tx.led_pwm_percent / LED_PWM_MODULO;
    if (led_on  &&  (milliseconds % LED_PWM_MODULO) < led_brightness) {
        gpio_clear(GPIOC, GPIO13);
    }
    else {
        gpio_set(GPIOC, GPIO13);
    }
}


// ****************************************************************************
void LED_init(void)
{
    // Configure LED output port
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_set(GPIOC, GPIO13);

    LED_off();
}