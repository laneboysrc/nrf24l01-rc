#pragma once

#include <stdint.h>

void init_spi(void);
uint8_t spi_transaction(unsigned int count, uint8_t *buffer);

