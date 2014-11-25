#include <stdlib.h>
#include <stdint.h>

#include <platform.h>
#include <spi.h>
#include <uart0.h>

#define LPC_SPI LPC_SPI0

#define SPI_CFG_ENABLE (1 << 0)
#define SPI_CFG_MASTER (1 << 2)
#define SPI_CFG_LSBF (1 << 3)

#define SPI_STAT_RXRDY (1 << 0)
#define SPI_STAT_TXRDY (1 << 1)
#define SPI_STAT_ENDTRANSFER (1 << 7)
#define SPI_STAT_MSTIDLE (1 << 8)

#define SPI_TXDATCTL_SSEL_N(s) ((s) << 16)
#define SPI_TXDATCTL_EOT (1 << 20)
#define SPI_TXDATCTL_EOF (1 << 21)
#define SPI_TXDATCTL_RXIGNORE (1 << 22)
#define SPI_TXDATCTL_LEN(l) ((l - 1) << 24)


void init_spi(void)
{
    // FIXME: check datasheet page 53..55 regarding timings
    // LPC_SPI->DLY  = (0x2 << 0) | (0x2 << 4) | (0x5 << 12);

    // nRF24L01+ datasheet page 50: maximum data rate of 10Mbps
    // To have some margin, we use 2 MHz SPI clock.
    LPC_SPI->DIV = (__SYSTEM_CLOCK / 2000000) - 1;

    LPC_SPI->CFG = SPI_CFG_ENABLE | SPI_CFG_MASTER;

    LPC_SPI->TXCTRL =  SPI_TXDATCTL_EOF | SPI_TXDATCTL_LEN(8);
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
    while (~LPC_SPI->STAT & SPI_STAT_MSTIDLE);

    while (count--) {
        // Wait for TXRDY
        while (~LPC_SPI->STAT & SPI_STAT_TXRDY);

        if (out != NULL) {
            LPC_SPI->TXDAT = *(out++);
        }
        else {
            LPC_SPI->TXDAT = 0;
        }

        // Wait for RXRDY
        while (~LPC_SPI->STAT & SPI_STAT_RXRDY);

        *in = LPC_SPI->RXDAT;

        // Increment in only when we are dealing with an input buffer
        if (read_buffer != NULL) {
            ++in;
        }
    }

    // Force END OF TRANSFER
    LPC_SPI->STAT = SPI_STAT_ENDTRANSFER;

    // Wait for MSTIDLE
    while (~LPC_SPI->STAT & SPI_STAT_MSTIDLE);

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
