#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RF_MAX_BUFFER_LENGTH 32

//******************************************************************************
// nRF24L01+ SPI commands
#define READ_REG        0x00  // Read a register (see below)
#define WRITE_REG       0x20  // Write a register
#define RD_RX_PLOAD     0x61  // RX payload
#define WR_TX_PLOAD     0xa0  // TX payload
#define FLUSH_TX        0xe1  // Flush TX buffer
#define FLUSH_RX        0xe2  // Flush RX buffer
#define REUSE_TX_PL     0xe3  // Reuse TX payload
#define NOP             0xff  // No Operation; used to read the status register

//******************************************************************************
// nRF24L01+ registers
#define CONFIG          0x00  // Config
#define EN_AA           0x01  // Enable Auto Acknowledgment
#define EN_RXADDR       0x02  // Enabled RX addresses
#define SETUP_AW        0x03  // Setup address width
#define SETUP_RETR      0x04  // Setup Auto. Retrans
#define RF_CH           0x05  // RF channel
#define RF_SETUP        0x06  // RF setup
#define STATUS          0x07  // Status
#define OBSERVE_TX      0x08  // Observe TX
#define CD              0x09  // Carrier Detect
#define RX_ADDR_P0      0x0a  // RX address pipe0
#define RX_ADDR_P1      0x0b  // RX address pipe1
#define RX_ADDR_P2      0x0c  // RX address pipe2
#define RX_ADDR_P3      0x0d  // RX address pipe3
#define RX_ADDR_P4      0x0e  // RX address pipe4
#define RX_ADDR_P5      0x0f  // RX address pipe5
#define TX_ADDR         0x10  // TX address
#define RX_PW_P0        0x11  // RX payload width, pipe0
#define RX_PW_P1        0x12  // RX payload width, pipe1
#define RX_PW_P2        0x13  // RX payload width, pipe2
#define RX_PW_P3        0x14  // RX payload width, pipe3
#define RX_PW_P4        0x15  // RX payload width, pipe4
#define RX_PW_P5        0x16  // RX payload width, pipe5
#define FIFO_STATUS     0x17  // FIFO Status Register


uint8_t rf_read_register(uint8_t reg);
void rf_write_register(uint8_t reg, uint8_t value);
uint8_t rf_write_command_buffer(uint8_t cmd, uint8_t count , uint8_t *buffer);
uint8_t rf_read_command_buffer(uint8_t cmd, uint8_t count, uint8_t *buffer);
uint8_t rf_write_multi_byte_register(uint8_t reg, uint8_t count , uint8_t *buffer);
uint8_t rf_read_multi_byte_register(uint8_t reg, uint8_t count , uint8_t *buffer);
void rf_enable_clock(bool on);
void rf_enable_transceiver(bool enable);
uint8_t rf_read_status(void);
