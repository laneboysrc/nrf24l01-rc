#pragma once

#include <stdint.h>

typedef uint16_t RING_SIZE_T;

typedef struct  {
    uint8_t *data;
    RING_SIZE_T size;
    uint32_t begin;
    uint32_t end;
} RING_BUFFER_T;

void ring_buffer_init(RING_BUFFER_T *ring, uint8_t *buf, RING_SIZE_T size);
RING_SIZE_T ring_buffer_write(RING_BUFFER_T *ring, uint8_t *data, RING_SIZE_T size);
RING_SIZE_T ring_buffer_write_uint8(RING_BUFFER_T *ring, uint8_t data);
RING_SIZE_T ring_buffer_read(RING_BUFFER_T *ring, uint8_t *data, RING_SIZE_T size);
RING_SIZE_T ring_buffer_read_uint8(RING_BUFFER_T *ring, uint8_t *data);
