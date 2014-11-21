#include <stdlib.h>
#include <stdint.h>

#include <platform.h>
#include <spi.h>

void init_spi(void)
{

    // FIXME: check datasheet page 53..55 regarding timings

    // nRF24L01+ datasheet page 50: maximum data rate of 10Mbps
    // To have some margin, we use 5 MHz SPI clock.
    LPC_SPI0->DIV = (__SYSTEM_CLOCK / 5000000) - 1;

    LPC_SPI0->CFG = (1 << 0) |          // Enable SPI0
                    (1 << 2) |          // Master mode
                    (0 << 3) |          // LSB First mode disabled
                    (0 << 4) |          // CPHA = 0
                    (0 << 5) |          // CPOL = 0
                    (0 << 8);           // SPOL = 0

    LPC_SPI0->TXCTRL = (1 << 21) |      // set EOF
                       ((8 - 1) << 24); // 8 bit frames

}


uint8_t spi_transaction(
    unsigned int count, uint8_t *write_buffer, uint8_t *read_buffer)
{
    uint8_t *out = write_buffer;
    uint8_t *in = read_buffer;

    uint8_t result;

    if (in == NULL) {
        in = &result;
    }

    // Wait for MSTIDLE
    while (!(LPC_SPI0->STAT & (1 << 8)));

    while (count--) {
        // Wait for TXRDY
        while (!(LPC_SPI0->STAT & (1 << 1)));

        if (out != NULL) {
            LPC_SPI0->TXDAT = *(out++);
        }
        else {
            LPC_SPI0->TXDAT = 0;
        }

        // Wait for RXRDY
        while (!(LPC_SPI0->STAT & (1 << 0)));

        *in = LPC_SPI0->RXDAT;

        // Increment in only when we are dealing with an input buffer
        if (read_buffer != NULL) {
            ++in;
        }
    }

    // Force END OF TRANSFER
    LPC_SPI0->STAT = (1 << 7);

    return read_buffer != NULL ? *read_buffer : result;
}

uint8_t spi_read(void)
{
    return spi_transaction(1, NULL, NULL);
}

void spi_write(uint8_t byte)
{
    spi_transaction(1, &byte, NULL);
}

uint8_t spi_read_write(uint8_t byte)
{
    return spi_transaction(1, &byte, NULL);
}