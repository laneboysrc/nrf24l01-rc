#include <stdio.h>
#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include <systick.h>
#include <uart.h>
#include <spi.h>
#include <nrf24l01p.h>
#include <sound.h>
#include <music.h>
#include <protocol_hk310.h>






// // Frequencies for each notes in Hz
// // Source: http://www.phy.mtu.edu/~suits/notefreqs.html
// #define F_C3 130.81
// #define F_D3 146.83
// #define F_E3 164.81
// #define F_F3 174.61
// #define F_G3 196.00
// #define F_A3 220.00
// #define F_B3 246.94
// #define F_C4 261.63
// #define F_D4 293.66
// #define F_E4 329.63
// #define F_F4 349.23
// #define F_G4 392.00
// #define F_A4 440.00
// #define F_B4 493.88
// #define F_C5 523.25
// #define SONG_END 0


// static const uint16_t song_startup[] = {
//     F_C4, F_C4, F_F4, F_F4, F_A4, F_A4, F_C5, F_C5, F_C5, F_C5, F_A4, F_A4, F_C5, F_C5, F_C5, F_C5, F_C5, F_C5, F_C5, SONG_END
// };

// static const uint16_t song_activate[] = {
//     F_C4, F_D4, F_E4, F_F4, F_G4, F_C4, F_D4, F_E4, F_F4, F_G4, F_A4, F_B4, F_C5, F_C5, F_C5, F_C5, SONG_END
// };

// static const uint16_t song_deactivate[] = {
//     F_C5, F_B4, F_A4, F_G4, F_F4, F_C5, F_B4, F_A4, F_G4, F_F4, F_E4, F_D4, F_C4, F_C4, F_C4, F_C4, SONG_END
// };

// uint16_t const *song_pointer = NULL;
// static uint8_t current_note = 0;

// ****************************************************************************
static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_24mhz();
}



// ****************************************************************************
static void init_gpio(void)
{
    // Enable clocks for GPIO port A (for GPIO_USART1_TX) and C (LED)
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    rcc_periph_clock_enable(RCC_AFIO);

    // Configure LED output port
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    gpio_set(GPIOC, GPIO13);
}


// // ****************************************************************************
// static void sound_frequency(uint16_t frequency)
// {
//     timer_set_period(TIM2, 500000/(uint32_t)frequency);
// }


// // ****************************************************************************
// static void sound_off(void)
// {
//     timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_INACTIVE);
// }


// // ****************************************************************************
// static void sound_on(void)
// {
//     timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_TOGGLE);
// }


// // ****************************************************************************
// static void play_song(uint16_t const *song)
// {
//     current_note = 0;
//     song_pointer = song;
// }











// ****************************************************************************




// ****************************************************************************
// static void init_timer2(void)
// {
//     rcc_periph_clock_enable(RCC_TIM2);

//     // Remap TIM2_CH1 to GPIOA15
//     gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_TIM2_REMAP_PARTIAL_REMAP1);

//     gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM2_PR1_CH1_ETR);

//     timer_reset(TIM2);
//     timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
//     timer_continuous_mode(TIM2);
//     timer_enable_preload(TIM2);
//     timer_set_prescaler(TIM2, 12);      // Timer runs at 1 MHz
//     timer_disable_oc_preload(TIM2, TIM_OC1);

//     timer_enable_oc_output(TIM2, TIM_OC1);

//     timer_disable_oc_output(TIM2, TIM_OC2);
//     timer_disable_oc_output(TIM2, TIM_OC3);
//     timer_disable_oc_output(TIM2, TIM_OC4);
//     timer_disable_oc_clear(TIM2, TIM_OC1);
//     timer_set_oc_slow_mode(TIM2, TIM_OC1);
//     timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_INACTIVE);
//     timer_set_oc_value(TIM2, TIM_OC1, 1);

//     sound_frequency(F_C4);
//     sound_on();

//     timer_enable_counter(TIM2);
// }



// ****************************************************************************
int main(void)
{
    int count = 0;
    bool armed = true;

    init_clock();
    init_systick();
    init_gpio();
    init_uart();
    init_spi();
    init_nrf24();
    init_sound();

    init_protocol_hk310();


    printf("Hello world!\n");
    // music_play(&song_startup);
    sound_play(440, 300, NULL);

    while (1) {
        if (armed) {
            if (milliseconds > 4000) {
                armed = false;
                music_play(&song_deactivate);
            }
        }

        // Blink the LED connected to PC13
        gpio_toggle(GPIOC, GPIO13);

        for (int i = 0; i < 5000000; i++) {
            __asm__("nop");
        }

        // __asm__("wfi\n");

        printf("%08lu %d\n", milliseconds, count);
        ++count;
    }

    return 0;
}
