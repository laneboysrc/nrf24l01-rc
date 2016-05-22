#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <ring_buffer.h>
#include <spi.h>
#include <nrf24l01p.h>

#define BUFFER_SIZE 1024

volatile uint32_t milliseconds;

static RING_BUFFER_T tx_ring_buffer;
static uint8_t tx_buffer[BUFFER_SIZE];

int _write(int file, char *ptr, int len);


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
    // Enable clocks for GPIO port A (for GPIO_USART1_TX) and C (LED)
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    // Configure LED output port
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
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

        // If there is still data in the transmit buffer send the next byte,
        // otherwise disable the TXE interrupt as it is no longer needed.
        if (ring_buffer_read_uint8(&tx_ring_buffer, &data)) {
            usart_send(USART1, data);
        }
        else {
            USART_CR1(USART1) &= ~USART_CR1_TXEIE;
        }
    }
}


// ****************************************************************************
int _write(int file, char *ptr, int len)
{
    RING_BUFFER_SIZE_T written;

    if (file == 1) {
        written = ring_buffer_write(&tx_ring_buffer, (uint8_t *)ptr, len);

        // Enable the TXE interrupt
        USART_CR1(USART1) |= USART_CR1_TXEIE;

        return written;
    }

    errno = EIO;
    return -1;
}


// ****************************************************************************
static void init_timer2(void)
{
    rcc_periph_clock_enable(RCC_TIM2);

    /* Set timer start value. */
    TIM_CNT(TIM2) = 1;

    /* Set timer prescaler. 72MHz/1440 => 50000 counts per second. */
    TIM_PSC(TIM2) = 1440;

    /* End timer value. If this is reached an interrupt is generated. */
    TIM_ARR(TIM2) = 50000;

    /* Update interrupt enable. */
    TIM_DIER(TIM2) |= TIM_DIER_UIE;

    /* Start timer. */
    TIM_CR1(TIM2) |= TIM_CR1_CEN;

    // Without this the timer interrupt routine will never be called
    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 1);
}


// ****************************************************************************
void tim2_isr(void)
{
    gpio_toggle(GPIOC, GPIO13);
    TIM_SR(TIM2) &= ~TIM_SR_UIF;    // Clear interrrupt flag
}


// ****************************************************************************
int main(void)
{
    int count = 0;

    init_clock();
    init_systick();
    init_gpio();
    init_uart();
    init_spi();
    init_nrf24();
    init_timer2();

    printf("Hello world!\n");


    // Blink the LED connected to PC13
    while (1) {
        // gpio_toggle(GPIOC, GPIO13);

        for (int i = 0; i < 5000000; i++) {
            __asm__("nop");
        }

        printf("%08lu %d\n", milliseconds, count);
        ++count;
    }

    return 0;
}
