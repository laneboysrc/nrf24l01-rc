#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define NRF24_MAX_PAYLOAD_SIZE 32

//******************************************************************************
// nRF24L01+ SPI commands
#define NRF24_R_REGISTER          0x00    // Read a register (see below)
#define NRF24_W_REGISTER          0x20    // Write a register
#define NRF24_R_RX_PAYLOAD        0x61    // RX payload
#define NRF24_W_TX_PAYLOAD        0xa0    // TX payload
#define NRF24_FLUSH_TX            0xe1    // Flush TX buffer
#define NRF24_FLUSH_RX            0xe2    // Flush RX buffer
#define NRF24_REUSE_TX_PL         0xe3    // Reuse TX payload
#define NRF24_R_RX_PL_WID         0x60    // Payload width for the top payload in the RX FIFO
#define NRF24_W_ACK_PAYLOAD       0xa8    // Write payload to be transmitted together with ACK
#define NRF24_W_TX_PAYLOAD_NOACK  0x0b    // Disables AUTOACK on this packet
#define NRF24_NOP                 0xff    // No Operation; used to read the status register
#define NRF24_ACTIVATE            0x50    // BK241 specific command


//******************************************************************************
// nRF24L01+ registers
#define NRF24_CONFIG          0x00        // Config
    #define NRF24_PRIM_RX     (1 << 0)
    #define NRF24_PWR_UP      (1 << 1)
    #define NRF24_CRCO        (1 << 2)
    #define NRF24_EN_CRC      (1 << 3)
    #define NRF24_MAX_RT      (1 << 4)    // Maximum re-transmit occured
    #define NRF24_TX_DS       (1 << 5)    // Transmitter data sent interrupt
    #define NRF24_RX_RD       (1 << 6)    // Receiver data ready

#define NRF24_EN_AA           0x01        // Enable Auto Acknowledgment
#define NRF24_EN_RXADDR       0x02        // Enabled RX addresses
#define NRF24_SETUP_AW        0x03        // Setup address width
    #define NRF24_ADDRESS_WIDTH_3_BYTES 0x01
    #define NRF24_ADDRESS_WIDTH_4_BYTES 0x02
    #define NRF24_ADDRESS_WIDTH_5_BYTES 0x03
#define NRF24_SETUP_RETR      0x04        // Setup Auto. Retrans
#define NRF24_RF_CH           0x05        // RF channel

#define NRF24_RF_SETUP        0x06        // RF setup
    #define NRF24_RF_DR_LOW   (1 << 5)
    #define NRF24_RF_DR_HIGH  (1 << 3)

#define NRF24_STATUS          0x07        // Status
#define NRF24_OBSERVE_TX      0x08        // Observe TX
#define NRF24_RPD             0x09        // Received Power Detector
#define NRF24_RX_ADDR_P0      0x0a        // RX address pipe0
#define NRF24_RX_ADDR_P1      0x0b        // RX address pipe1
#define NRF24_RX_ADDR_P2      0x0c        // RX address pipe2
#define NRF24_RX_ADDR_P3      0x0d        // RX address pipe3
#define NRF24_RX_ADDR_P4      0x0e        // RX address pipe4
#define NRF24_RX_ADDR_P5      0x0f        // RX address pipe5
#define NRF24_TX_ADDR         0x10        // TX address
#define NRF24_RX_PW_P0        0x11        // RX payload width, pipe0
#define NRF24_RX_PW_P1        0x12        // RX payload width, pipe1
#define NRF24_RX_PW_P2        0x13        // RX payload width, pipe2
#define NRF24_RX_PW_P3        0x14        // RX payload width, pipe3
#define NRF24_RX_PW_P4        0x15        // RX payload width, pipe4
#define NRF24_RX_PW_P5        0x16        // RX payload width, pipe5
#define NRF24_FIFO_STATUS     0x17        // FIFO Status Register
#define NRF24_DYNPD           0x1c        // Enable dynamic payload length
#define NRF24_FEATURE         0x1d        // Feature register

#define NRF24_POWER_n18dBm    0
#define NRF24_POWER_n12dBm    1
#define NRF24_POWER_n6dBm     2
#define NRF24_POWER_0dBm     3

//******************************************************************************
void NRF24_init(void);

uint8_t NRF24_read_register(uint8_t reg);
void NRF24_write_register(uint8_t reg, uint8_t value);
uint8_t NRF24_write_multi_byte_register(uint8_t reg, const uint8_t *buffer, uint8_t count);

void NRF24_activate(uint8_t code);


uint8_t NRF24_get_status(void);

void NRF24_flush_rx_fifo(void);
void NRF24_flush_tx_fifo(void);

void NRF24_write_payload(const uint8_t payload[], uint8_t payload_size);
void NRF24_read_payload(uint8_t *payload, uint8_t payload_size);

void NRF24_set_bitrate(uint8_t bitrate);
void NRF24_set_power(uint8_t power);
