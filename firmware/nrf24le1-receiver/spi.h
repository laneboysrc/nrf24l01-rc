#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

void init_spi(void);
uint8_t spi_transaction(uint8_t count, uint8_t __xdata *buffer);

#endif
