#include <stdlib.h>
#include <stdint.h>

#include <platform.h>
#include <spi.h>



// ****************************************************************************
void init_spi(void)
{
}


// ****************************************************************************
uint8_t spi_transaction(uint8_t count, uint8_t __xdata *buffer)
{
    uint8_t __xdata *ptr = buffer;

    RFCON_rfcsn = 0;

    while (count--) {
        SPIRDAT = *ptr;

        // Wait for SPIF
        while (!(SPIRSTAT & 0x02));

        *ptr = SPIRDAT;
        ++ptr;
    }

    RFCON_rfcsn = 1;

    return *buffer;
}

