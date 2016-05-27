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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include <sound.h>
#include <systick.h>


// ****************************************************************************
#define SOUND_TIMER_PRESCALER 4

static void(* callback)(void);
static uint32_t volume_factor;
static uint8_t sound_timer_frequency;


// ****************************************************************************
void init_sound(void)
{
    rcc_periph_clock_enable(RCC_TIM2);

    // Remap TIM2_CH1 to GPIOA15.
    // Note that the default function of the pin A15 is JTAG, so we need to
    // turn that off to it as normal GPIO!
    gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_TIM2_REMAP_PARTIAL_REMAP1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM2_PR1_CH1_ETR);

    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    // Timer runs at 24 / 4 = 6 MHz
    timer_enable_preload(TIM2);
    timer_set_prescaler(TIM2, SOUND_TIMER_PRESCALER - 1);
    sound_timer_frequency = rcc_apb1_frequency / SOUND_TIMER_PRESCALER;

    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
    timer_enable_oc_preload(TIM2, TIM_OC1);
    timer_disable_oc_output(TIM2, TIM_OC1);

    // Polarity needs to be low to make our volume control algorithm work
    timer_set_oc_polarity_low(TIM2, TIM_OC1);

    sound_set_volume(100);
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
static void set_timer(unsigned int frequency)
{
    uint32_t period;
    uint32_t duty_cycle;

    // Frequency 0 is the code for pause within the songs
    if (frequency == 0) {
        timer_disable_oc_output(TIM2, TIM_OC1);
        return;
    }


    if (volume_factor == 0) {
        // Keep the timer running but turn the output off
        timer_disable_oc_output(TIM2, TIM_OC1);
    }
    else {
        timer_enable_oc_output(TIM2, TIM_OC1);
    }


    // The Timer2 runs at 6 MHz, so we need to set the ARR to that figure
    // divided by the frequency we are looking to generate.
    period = sound_timer_frequency / frequency;

    // Simple non-linear function to mimic a perceived linear volume level
    duty_cycle = (period / 2) * volume_factor / 100 * volume_factor / 100 * volume_factor / 100;

    timer_set_period(TIM2, period);
    timer_set_oc_value(TIM2, TIM_OC1, duty_cycle);
}


// ****************************************************************************
void sound_set_volume(uint8_t volume)
{
    // Clamp volume to 0..100
    if (volume > 100) {
        volume = 100;
    }

    volume_factor = (uint32_t)volume;
}


// ****************************************************************************
void sound_play(unsigned int frequency, uint32_t duration_ms, void(* cb)(void))
{
    callback = cb;
    set_timer(frequency);
    timer_enable_counter(TIM2);
    systick_set_callback(end_of_sound_callback, duration_ms);
}


// ****************************************************************************
void sound_stop(void)
{
    systick_clear_callback(end_of_sound_callback);
    timer_disable_oc_output(TIM2, TIM_OC1);
    timer_disable_counter(TIM2);
}

