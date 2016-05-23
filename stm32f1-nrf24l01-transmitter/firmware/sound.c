/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include <systick.h>
#include <sound.h>

static void(* callback)(void);

#define SOUND_TIMER_FREQUENCY 12000000


// ****************************************************************************
void init_sound(void)
{
    rcc_periph_clock_enable(RCC_TIM2);

    // Remap TIM2_CH1 to GPIOA15
    gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_TIM2_REMAP_PARTIAL_REMAP1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM2_PR1_CH1_ETR);

    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM2, 2);   // 24 Mhz / 2  => SOUND_TIMER_FREQUENCY
    timer_generate_event(TIM2, TIM_EGR_UG);
    timer_enable_preload(TIM2);

    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
    timer_enable_oc_preload(TIM2, TIM_OC1);
    // timer_set_oc_polarity_low(TIM2, TIM2_OC);

    // Write some default values
    timer_set_period(TIM2, 0xffff);
    timer_set_oc_value(TIM2, TIM_OC1, 0x8000);
}


// ****************************************************************************
static void end_of_sound_callback(void)
{
    sound_stop();
    if (callback != NULL) {
        (*callback)();
    }
}


// ****************************************************************************
static void set_timer(unsigned int frequency, uint8_t volume)
{
    uint32_t period;
    uint32_t duty_cycle;

    // Frequency 0 is the code for pauses within the songs
    if (frequency == 0) {
        timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_INACTIVE);
        return;
    }


    if (volume == 0) {
        // Keep the timer running but set the output to "inactive" state
        timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_INACTIVE);
    }
    else {
        timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
    }


    // Clamp volume to 0..100
    if (volume > 100) {
        volume = 100;
    }

    // The Timer2 runs at 12 MHz, so we need to set the ARR to that figure
    // divided by the frequency we are looking to generate.
    period = SOUND_TIMER_FREQUENCY / frequency;

    // Simple non-linear function to mimic a  perceived liner volume level
    duty_cycle = (period >> 1) * (uint32_t)volume / 100 * volume / 100 * volume / 100;

    timer_set_period(TIM2, period);
    timer_set_oc_value(TIM2, TIM_OC1, duty_cycle);
}


// ****************************************************************************
void sound_play(unsigned int frequency, uint8_t volume, uint32_t duration_ms, void(* cb)(void))
{
    callback = cb;
    set_timer(frequency, volume);
    systick_set_callback(end_of_sound_callback, duration_ms);
    timer_enable_counter(TIM2);
}


// ****************************************************************************
void sound_stop(void)
{
    systick_clear_callback(end_of_sound_callback);
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_INACTIVE);
    timer_disable_counter(TIM2);
    // timer_disable_oc_output(TIM2, TIM_OC1);
}

