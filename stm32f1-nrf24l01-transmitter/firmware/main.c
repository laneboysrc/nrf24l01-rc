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


static uint16_t frequency_sequence[18] = {
    1000,
    500,
    1000,
    500,
    1000,
    500,
    2000,
    500,
    2000,
    500,
    2000,
    500,
    1000,
    500,
    1000,
    500,
    1000,
    5000,
};


static int frequency_sel = 0;



// ****************************************************************************
static void init_clock(void)
{
    rcc_clock_setup_in_hse_8mhz_out_24mhz();
}


// ****************************************************************************
static void init_systick(void)
{
    // 24 MHz / 8 => 3000000 counts per second
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    // 3000000/3000 = 1000 overflows per second - every 1ms one interrupt
    // SysTick interrupt every N clock pulses: set reload to N-1
    systick_set_reload(2999);

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

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO0);

    timer_reset(TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_continuous_mode(TIM2);
    timer_disable_preload(TIM2);
    timer_set_prescaler(TIM2, 12000);
    timer_set_period(TIM2, 65535);
    timer_disable_oc_preload(TIM2, TIM_OC1);

    timer_enable_oc_output(TIM2, TIM_OC1);

    timer_disable_oc_output(TIM2, TIM_OC2);
    timer_disable_oc_output(TIM2, TIM_OC3);
    timer_disable_oc_output(TIM2, TIM_OC4);
    timer_disable_oc_clear(TIM2, TIM_OC1);
    timer_set_oc_slow_mode(TIM2, TIM_OC1);
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_TOGGLE);
    timer_set_oc_value(TIM2, TIM_OC1, 1000);
    timer_enable_counter(TIM2);

    timer_enable_irq(TIM2, TIM_DIER_CC1IE);
    nvic_enable_irq(NVIC_TIM2_IRQ);
}


// ****************************************************************************
void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
        uint16_t compare_time;
        uint16_t new_time;
        uint16_t frequency;

        timer_clear_flag(TIM2, TIM_SR_CC1IF);

        compare_time = timer_get_counter(TIM2);
        frequency = frequency_sequence[frequency_sel];
        new_time = compare_time + frequency;
        timer_set_oc_value(TIM2, TIM_OC1, new_time);

        ++frequency_sel;
        if (frequency_sel == 18) {
            frequency_sel = 0;
        }

        /* Toggle LED to indicate compare event. */
        gpio_toggle(GPIOC, GPIO13);
    }
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


    while (1) {
        // Blink the LED connected to PC13
        // gpio_toggle(GPIOC, GPIO13);
        // gpio_toggle(GPIOA, GPIO0);

        for (int i = 0; i < 5000000; i++) {
            __asm__("nop");
        }

        printf("%08lu %d\n", milliseconds, count);
        ++count;
    }

    return 0;
}
