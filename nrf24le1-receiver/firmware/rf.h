#ifndef __RF_H__
#define __RF_H__


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define RF_MAX_BUFFER_LENGTH 32


//******************************************************************************
// nRF24L01+ SPI commands
#define R_REGISTER          0x00    // Read a register (see below)
#define W_REGISTER          0x20    // Write a register
#define R_RX_PAYLOAD        0x61    // RX payload
#define W_TX_PAYLOAD        0xa0    // TX payload
#define FLUSH_TX            0xe1    // Flush TX buffer
#define FLUSH_RX            0xe2    // Flush RX buffer
#define REUSE_TX_PL         0xe3    // Reuse TX payload
#define R_RX_PL_WID         0x60    // Payload width for the top payload in the RX FIFO
#define W_ACK_PAYLOAD       0xa8    // Write payload to be transmitted together with ACK
#define W_TX_PAYLOAD_NOACK  0x0b    // Disables AUTOACK on this packet
#define NOP                 0xff    // No Operation; used to read the status register


//******************************************************************************
// nRF24L01+ registers
#define CONFIG          0x00        // Config
    #define PRIM_RX     (1 << 0)
    #define PWR_UP      (1 << 1)
    #define CRC0        (1 << 2)
    #define EN_CRC      (1 << 3)
    #define MAX_RT      (1 << 4)    // Maximum re-transmit occured
    #define TX_DS       (1 << 5)    // Transmitter data sent interrupt
    #define RX_RD       (1 << 6)    // Receiver data ready

#define EN_AA           0x01        // Enable Auto Acknowledgment
#define EN_RXADDR       0x02        // Enabled RX addresses
#define SETUP_AW        0x03        // Setup address width
#define SETUP_RETR      0x04        // Setup Auto. Retrans
#define RF_CH           0x05        // RF channel

#define RF_SETUP        0x06        // RF setup
    #define RF_DR_LOW   (1 << 5)
    #define RF_DR_HIGH  (1 << 3)

#define STATUS          0x07        // Status
#define OBSERVE_TX      0x08        // Observe TX
#define RPD             0x09        // Received Power Detector
#define RX_ADDR_P0      0x0a        // RX address pipe0
#define RX_ADDR_P1      0x0b        // RX address pipe1
#define RX_ADDR_P2      0x0c        // RX address pipe2
#define RX_ADDR_P3      0x0d        // RX address pipe3
#define RX_ADDR_P4      0x0e        // RX address pipe4
#define RX_ADDR_P5      0x0f        // RX address pipe5
#define TX_ADDR         0x10        // TX address
#define RX_PW_P0        0x11        // RX payload width, pipe0
#define RX_PW_P1        0x12        // RX payload width, pipe1
#define RX_PW_P2        0x13        // RX payload width, pipe2
#define RX_PW_P3        0x14        // RX payload width, pipe3
#define RX_PW_P4        0x15        // RX payload width, pipe4
#define RX_PW_P5        0x16        // RX payload width, pipe5
#define FIFO_STATUS     0x17        // FIFO Status Register
#define DYNPD           0x1c        // Enable dynamic payload length
#define FEATURE         0x1d        // Feature register


//******************************************************************************
// Parameters for convenience functions
#define DATA_RATE_250K  0
#define DATA_RATE_1M    1
#define DATA_RATE_2M    2

#define DATA_PIPE_0     (1 << 0)
#define DATA_PIPE_1     (1 << 1)
#define DATA_PIPE_2     (1 << 2)
#define DATA_PIPE_3     (1 << 3)
#define DATA_PIPE_4     (1 << 4)
#define DATA_PIPE_5     (1 << 5)

#define NO_CRC          0
#define CRC_1_BYTE      1
#define CRC_2_BYTES     2

#define NO_AUTO_ACKNOWLEDGE 0


//******************************************************************************
void rf_enable_clock(void);
void rf_disable_clock(void);
void rf_set_ce(void);
void rf_clear_ce(void);

uint8_t rf_get_status(void);

bool rf_is_rx_fifo_emtpy(void);
bool rf_is_tx_fifo_full(void);
void rf_read_fifo(uint8_t *buffer, size_t byte_count);
void rf_flush_rx_fifo(void);
void rf_flush_tx_fifo(void);

void rf_set_irq_source(uint8_t irq_source);
void rf_clear_irq(uint8_t irq_source);

void rf_enable_transmitter(void);
void rf_enable_receiver(void);
void rf_power_down(void);

void rf_set_channel(uint8_t channel);
void rf_set_crc(uint8_t crc_size);
void rf_set_data_rate(uint8_t data_rate);
void rf_set_address_width(uint8_t aw);
uint8_t rf_get_address_width(void);
void rf_set_data_pipes(uint8_t pipes, uint8_t auto_acknowledge_pipes);
void rf_set_payload_size(uint8_t pipes, uint8_t payload_size);
void rf_set_rx_address(uint8_t pipe, uint8_t address_width, const uint8_t address[]);

#endif
