#include <stdlib.h>
#include <stdint.h>

#include <platform.h>
#include <spi.h>



void init_spi(void)
{
}


uint8_t spi_transaction(
   uint8_t count, uint8_t *write_buffer, uint8_t *read_buffer)
{
    uint8_t *out = write_buffer;
    uint8_t *in = read_buffer;

    uint8_t result = 0;


    RFCON_rfcsn = 0;

    if (in == NULL) {
        in = &result;
    }


    while (count--) {
        if (out != NULL) {
            SPIRDAT = *(out++);
        }
        else {
            SPIRDAT = 0;
        }

        // Wait for SPIF
        while (!(SPIRSTAT & 0x02));

        *in = SPIRDAT;

        // Increment in only when we are dealing with an input buffer
        if (read_buffer != NULL) {
            ++in;
        }
    }

    RFCON_rfcsn = 1;

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
