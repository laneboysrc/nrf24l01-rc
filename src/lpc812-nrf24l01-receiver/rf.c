#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <spi.h>
#include <rf.h>

static uint8_t write_buffer[RF_MAX_BUFFER_LENGTH + 1];
static uint8_t read_buffer[RF_MAX_BUFFER_LENGTH + 1];


uint8_t rf_read_register(uint8_t reg)
{
    write_buffer[0] = READ_REG | reg;
    write_buffer[1] = 0;

    spi_transaction(2, write_buffer, read_buffer);

    return read_buffer[1];
}


void rf_write_register(uint8_t reg, uint8_t value)
{
    write_buffer[0] = WRITE_REG | reg;
    write_buffer[1] = value;

    spi_transaction(2, write_buffer, NULL);
}


uint8_t rf_write_command_buffer(uint8_t cmd, uint8_t count, uint8_t *buffer)
{
    int i;

    if (count > RF_MAX_BUFFER_LENGTH) {
        count = RF_MAX_BUFFER_LENGTH;
    }

    write_buffer[0] = cmd;
    for (i = 0; i < count; i++) {
        write_buffer[i + 1] = buffer[i];
    }

    spi_transaction(count + 1, write_buffer, read_buffer);

    return read_buffer[0];
}


uint8_t rf_write_multi_byte_register(uint8_t reg, uint8_t count, uint8_t *buffer)
{
    return rf_write_command_buffer(WRITE_REG | reg, count, buffer);
}


uint8_t rf_read_command_buffer(uint8_t cmd, uint8_t count, uint8_t *buffer)
{
    int i;

    if (count > RF_MAX_BUFFER_LENGTH) {
        count = RF_MAX_BUFFER_LENGTH;
    }

    write_buffer[0] = cmd;
    for (i = 0; i < count; i++) {
        write_buffer[i + 1] = 0;
    }

    spi_transaction(count + 1, write_buffer, read_buffer);

    for (i = 0; i < count; i++) {
        buffer[i] = read_buffer[i + 1];
    }

    return read_buffer[0];
}


uint8_t rf_read_multi_byte_register(uint8_t reg, uint8_t count, uint8_t *buffer)
{
    return rf_read_command_buffer(READ_REG | reg, count, buffer);
}


void rf_enable_clock(bool on)
{
    // rfcken = on
}


void rf_enable_transceiver(bool enable)
{
    // rfce = enable
}


uint8_t rf_read_status(void)
{
    return rf_read_command_buffer(NOP, 0, NULL);
}
