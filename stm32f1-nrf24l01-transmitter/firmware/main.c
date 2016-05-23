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


// ****************************************************************************
int main(void)
{
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
    music_play(&song_startup);

    while (1) {
        if (armed) {
            if (milliseconds > 1800) {
                armed = false;
                music_play(&song_deactivate);
            }
        }

        // Put the CPU to sleep until an interrupt triggers. This reduces
        // power consumption drastically.
        // Since the systick runs at 1 millisecond period, the main loop sleeps
        // for at most 1 ms.
        __asm__("wfi\n");
    }

    return 0;
}
