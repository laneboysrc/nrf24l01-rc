#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "ring_buffer.h"

#define BUFFER_SIZE 1024

volatile uint32_t milliseconds;

static RING_BUFFER_T tx_ring_buffer;
static uint8_t tx_buffer[BUFFER_SIZE];


// ****************************************************************************
static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

}


// ****************************************************************************
static void init_systick(void)
{
    // 72 MHz / 8 => 9000000 counts per second
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    // 9000000/9000 = 1000 overflows per second - every 1ms one interrupt
    // SysTick interrupt every N clock pulses: set reload to N-1
    systick_set_reload(8999);

    systick_interrupt_enable();
    systick_counter_enable();
}


// ****************************************************************************
static void init_gpio(void)
{
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    // Enable clocks for GPIO port A (for GPIO_USART1_TX)
    rcc_periph_clock_enable(RCC_GPIOA);
}


// ****************************************************************************
static void init_uart(void)
{
    rcc_periph_clock_enable(RCC_USART1);

    // FIXME: Not sure if this is needed since we don't remap...
    rcc_periph_clock_enable(RCC_AFIO);

    ring_buffer_init(&tx_ring_buffer, tx_buffer, BUFFER_SIZE);

    // Setup GPIO pins GPIO_USART1_RE_TX on PA9 and GPIO_USART1_RE_RX on PA10
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                    GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                    GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);

    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART1, USART_MODE_TX_RX);

    // Enable USART1 receive interrupt
    USART_CR1(USART1) |= USART_CR1_RXNEIE;

    nvic_enable_irq(NVIC_USART1_IRQ);
    usart_enable(USART1);
}


// ****************************************************************************
void sys_tick_handler(void)
{
    ++milliseconds;
}


// ****************************************************************************
void usart1_isr(void)
{
    // Check if we were called because of RXNE
    if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {

        uint16_t ch = usart_recv(USART1);

        (void) ch;
    }

    // Check if we were called because of TXE
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

        uint8_t data;

        if (ring_buffer_read_uint8(&tx_ring_buffer, &data) == 0) {
            // Disable the TXE interrupt, it's no longer needed
            USART_CR1(USART1) &= ~USART_CR1_TXEIE;
        } else {
            // Put data into the transmit register
            usart_send(USART1, data);
        }
    }
}


// ****************************************************************************
int main(void)
{
    int i;

    init_clock();
    init_systick();
    init_gpio();
    init_uart();

    printf("Hello world!\n;");

    // Blink the LED connected to PC13
    while (1) {
        gpio_toggle(GPIOC, GPIO13);
        for (i = 0; i < 800000; i++) {
            __asm__("nop");
        }
    }

    return 0;
}
