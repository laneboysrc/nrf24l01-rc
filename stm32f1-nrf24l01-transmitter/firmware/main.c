#include <stdint.h>
#include <stdio.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencmsis/core_cm3.h>

#include <inputs.h>
#include <mixer.h>
#include <music.h>
#include <nrf24l01p.h>
#include <protocol_hk310.h>
#include <sound.h>
#include <spi.h>
#include <systick.h>
#include <uart.h>


// ****************************************************************************
static void init_clock(void)
{
    // Enable the Clock Security System
    rcc_css_enable();

    rcc_clock_setup_in_hse_8mhz_out_24mhz();
}


// ****************************************************************************
static void init_gpio(void)
{
    // Enable clocks for GPIO port A (for GPIO_USART1_TX) and C (LED)
    // IMPORTANT: you can not 'or' them into one call due to bit-mangling
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_AFIO);

    // Configure LED output port
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_set(GPIOC, GPIO13);
}


// ****************************************************************************
int main(void)
{
    init_clock();
    init_systick();
    init_gpio();
    init_uart();
    init_spi();
    init_sound();
    init_nrf24();

    init_inputs();
    init_mixer();
    init_protocol_hk310();

    printf("\n\n\n**********\nTransmitter initialized\n");
    // music_play(&song_startup);
    sound_play(523, 100, NULL);

    // FIXME: use watchdog

    while (1) {
        // bool armed = true;
        // if (armed) {
        //     if (milliseconds > 1800) {
        //         armed = false;
        //         music_play(&song_deactivate);
        //     }
        // }

        // Put the CPU to sleep until an interrupt triggers. This reduces
        // power consumption drastically.
        // Since the systick runs at 1 millisecond period, the main loop sleeps
        // for at most 1 ms.
        __WFI();
    }

    return 0;
}
