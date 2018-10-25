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


// ****************************************************************************
void init_spi(void)
{
    // nRF24L01+ datasheet page 50: maximum data rate of 10Mbps
    // To have some margin, we use 2 MHz SPI clock.
    LPC_SPI->DIV = (__SYSTEM_CLOCK / 2000000) - 1;

    LPC_SPI->CFG = SPI_CFG_ENABLE | SPI_CFG_MASTER;

    LPC_SPI->TXCTRL =  SPI_TXDATCTL_EOF | SPI_TXDATCTL_LEN(8);
}


// ****************************************************************************
uint8_t spi_transaction(unsigned int count, uint8_t *buffer)
{
    uint8_t *ptr = buffer;

    // Wait for MSTIDLE
    while (~LPC_SPI->STAT & SPI_STAT_MSTIDLE);

    while (count--) {
        // Wait for TXRDY
        while (~LPC_SPI->STAT & SPI_STAT_TXRDY);

        LPC_SPI->TXDAT = *ptr;

        // Wait for RXRDY
        while (~LPC_SPI->STAT & SPI_STAT_RXRDY);

        *ptr = LPC_SPI->RXDAT;
        ++ptr;
    }

    // Force END OF TRANSFER
    LPC_SPI->STAT = SPI_STAT_ENDTRANSFER;

    // Wait for MSTIDLE
    while (~LPC_SPI->STAT & SPI_STAT_MSTIDLE);

    return *buffer;
}

