#pragma once

#include <stdint.h>

void SPI_init(void);
uint8_t SPI_transaction(unsigned int count, uint8_t *buffer);

