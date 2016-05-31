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
#include <watchdog.h>


// ****************************************************************************
void nmi_handler(void) {
    // The NMI is triggered by the Clock Security System. We clear the CSS
    // interrupt and switch back to the internal RC oscillator

    rcc_css_int_clear();
    rcc_clock_setup_in_hsi_out_24mhz();
    SOUND_play(220, 1000, NULL);
}


// ****************************************************************************
static void clock_init(void)
{
    // Enable the Clock Security System
    rcc_css_enable();

    // NOTE: the transmitter will not boot when the crystal is not working as
    // there is no timeout waiting for the HSE in rcc_clock_setup_in_hse_8mhz_out_24mhz().
    rcc_clock_setup_in_hse_8mhz_out_24mhz();
}


// ****************************************************************************
static void gpio_init(void)
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
    clock_init();
    gpio_init();
    SYSTICK_init();
    UART_init();
    SPI_init();
    SOUND_init();
    NRF24_init();

    WATCHDOG_start();

    INPUTS_init();
    MIXER_init();
    PROTOCOL_HK310_init();

    printf("\n\n\n**********\nTransmitter initialized\n");
    // MUSIC_play(&song_startup);
    SOUND_play(523, 100,    NULL);

    while (1) {
        WATCHDOG_reset();

        // Put the CPU to sleep until an interrupt triggers. This reduces
        // power consumption drastically.
        // Since the systick runs at 1 millisecond period, the main loop sleeps
        // for at most 1 ms.
        __WFI();
    }

    return 0;
}
