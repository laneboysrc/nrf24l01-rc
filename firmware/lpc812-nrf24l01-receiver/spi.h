#pragma once

#include <stdint.h>

void init_spi(void);
uint8_t spi_transaction(
    unsigned int count, uint8_t *write_buffer, uint8_t *read_buffer);


uint8_t spi_read(void);
void spi_write(uint8_t byte);
uint8_t spi_read_write(uint8_t byte);

