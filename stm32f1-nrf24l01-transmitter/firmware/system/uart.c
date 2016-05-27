#include <stdint.h>
#include <errno.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencmsis/core_cm3.h>

#include <ring_buffer.h>
#include <uart.h>


// ****************************************************************************
#define BUFFER_SIZE 1024


static uint8_t tx_buffer[BUFFER_SIZE];
static RING_BUFFER_T tx_ring_buffer;


int _write(int file, char *ptr, int len);


// ****************************************************************************
void init_uart(void)
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