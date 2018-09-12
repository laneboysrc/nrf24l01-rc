/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef __MICRO_ESB_H
#define __MICRO_ESB_H

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"

#define DEBUGPIN1   12
#define DEBUGPIN2   13
#define DEBUGPIN3   14
#define DEBUGPIN4   15

#ifdef  UESB_DEBUG
#define DEBUG_PIN_SET(a)    (NRF_GPIO->OUTSET = (1 << (a)))
#define DEBUG_PIN_CLR(a)    (NRF_GPIO->OUTCLR = (1 << (a)))
#else
#define DEBUG_PIN_SET(a)
#define DEBUG_PIN_CLR(a)
#endif

// Hard coded parameters - change if necessary
#define     UESB_CORE_MAX_PAYLOAD_LENGTH    32
#define     UESB_CORE_TX_FIFO_SIZE          8
#define     UESB_CORE_RX_FIFO_SIZE          8

#define     UESB_SYS_TIMER                  NRF_TIMER2
#define     UESB_SYS_TIMER_IRQ_Handler      TIMER2_IRQHandler

#define     UESB_PPI_TIMER_START            4
#define     UESB_PPI_TIMER_STOP             5
#define     UESB_PPI_RX_TIMEOUT             6
#define     UESB_PPI_TX_START               7

// Interrupt flags
#define     UESB_INT_TX_SUCCESS_MSK         0x01
#define     UESB_INT_TX_FAILED_MSK          0x02
#define     UESB_INT_RX_DR_MSK              0x04

#define     UESB_PID_RESET_VALUE            0xFF

// Configuration parameter definitions
typedef enum {
    UESB_PROTOCOL_SB,       // Legacy ShockBurst mode - No ACK or retransmit functionality
    UESB_PROTOCOL_ESB,      // Enhanced ShockBurst with fixed payload length
    UESB_PROTOCOL_ESB_DPL   // Enhanced ShockBurst with dynamic payload length
} uesb_protocol_t;

typedef enum {
    UESB_MODE_PTX,          // Primary transmitter
    UESB_MODE_PRX           // Primary receiver
} uesb_mode_t;

typedef enum {
    UESB_BITRATE_2MBPS = RADIO_MODE_MODE_Nrf_2Mbit,
    UESB_BITRATE_1MBPS = RADIO_MODE_MODE_Nrf_1Mbit,
    UESB_BITRATE_250KBPS = RADIO_MODE_MODE_Nrf_250Kbit
} uesb_bitrate_t;

typedef enum {
    UESB_CRC_16BIT = RADIO_CRCCNF_LEN_Two,
    UESB_CRC_8BIT  = RADIO_CRCCNF_LEN_One,
    UESB_CRC_OFF   = RADIO_CRCCNF_LEN_Disabled
} uesb_crc_t;

typedef enum {
    UESB_TX_POWER_4DBM     = RADIO_TXPOWER_TXPOWER_Pos4dBm,
    UESB_TX_POWER_0DBM     = RADIO_TXPOWER_TXPOWER_0dBm,
    UESB_TX_POWER_NEG4DBM  = RADIO_TXPOWER_TXPOWER_Neg4dBm,
    UESB_TX_POWER_NEG8DBM  = RADIO_TXPOWER_TXPOWER_Neg8dBm,
    UESB_TX_POWER_NEG12DBM = RADIO_TXPOWER_TXPOWER_Neg12dBm,
    UESB_TX_POWER_NEG16DBM = RADIO_TXPOWER_TXPOWER_Neg16dBm,
    UESB_TX_POWER_NEG20DBM = RADIO_TXPOWER_TXPOWER_Neg20dBm,
    UESB_TX_POWER_NEG30DBM = RADIO_TXPOWER_TXPOWER_Neg30dBm
} uesb_tx_power_t;

typedef enum {
    UESB_TXMODE_AUTO,        // Automatic TX mode - When the TX fifo is non-empty and the radio is idle packets will be sent automatically.
    UESB_TXMODE_MANUAL,      // Manual TX mode - Packets will not be sent until uesb_start_tx() is called. Can be used to ensure consistent packet timing.
    UESB_TXMODE_MANUAL_START // Manual start TX mode - Packets will not be sent until uesb_start_tx() is called, but transmission will continue automatically until the TX FIFO is empty.
} uesb_tx_mode_t;

// Internal state definition
typedef enum {
    UESB_STATE_UNINITIALIZED,
    UESB_STATE_IDLE,
    UESB_STATE_PTX_TX,
    UESB_STATE_PTX_TX_ACK,
    UESB_STATE_PTX_RX_ACK,
    UESB_STATE_PRX,
    UESB_STATE_PRX_SEND_ACK,
    UESB_STATE_PRX_SEND_ACK_PAYLOAD
} uesb_mainstate_t;

typedef void (*uesb_event_handler_t)(void);

// Main UESB configuration struct, contains all radio parameters
typedef struct
{
    uesb_protocol_t         protocol;
    uesb_mode_t             mode;
    uesb_event_handler_t    event_handler;

    // General RF parameters
    uesb_bitrate_t          bitrate;
    uesb_crc_t              crc;
    uint8_t                 rf_channel;
    uint8_t                 payload_length;
    uint8_t                 rf_addr_length;

    uesb_tx_power_t         tx_output_power;
    uint8_t                 tx_address[5];
    uint8_t                 rx_address_p0[5];
    uint8_t                 rx_address_p1[5];
    uint8_t                 rx_address_p2;
    uint8_t                 rx_address_p3;
    uint8_t                 rx_address_p4;
    uint8_t                 rx_address_p5;
    uint8_t                 rx_address_p6;
    uint8_t                 rx_address_p7;
    uint8_t                 rx_pipes_enabled;

    // ESB specific features
    uint8_t                 dynamic_payload_length_enabled;
    uint8_t                 dynamic_ack_enabled;
    uint16_t                retransmit_delay;
    uint16_t                retransmit_count;

    // Control settings
    uesb_tx_mode_t          tx_mode;

    uint8_t                 radio_irq_priority;
}uesb_config_t;

// Default radio parameters, roughly equal to nRF24L default parameters (except CRC which is set to 16-bit, and protocol set to DPL)
#define UESB_DEFAULT_CONFIG {.mode                  = UESB_MODE_PTX,                    \
                             .protocol              = UESB_PROTOCOL_ESB_DPL,            \
                             .event_handler         = 0,                                \
                             .rf_channel            = 2,                                \
                             .payload_length        = UESB_CORE_MAX_PAYLOAD_LENGTH,     \
                             .rf_addr_length        = 5,                                \
                             .bitrate               = UESB_BITRATE_2MBPS,               \
                             .crc                   = UESB_CRC_16BIT,                   \
                             .tx_output_power       = UESB_TX_POWER_0DBM,               \
                             .rx_address_p0         = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7},   \
                             .rx_address_p1         = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2},   \
                             .rx_address_p2         = 0xC3,                             \
                             .rx_address_p3         = 0xC4,                             \
                             .rx_address_p4         = 0xC5,                             \
                             .rx_address_p5         = 0xC6,                             \
                             .rx_address_p6         = 0xC7,                             \
                             .rx_address_p7         = 0xC8,                             \
                             .rx_pipes_enabled      = 0x3F,                             \
                             .dynamic_payload_length_enabled = 1,                       \
                             .dynamic_ack_enabled   = 0,                                \
                             .retransmit_delay      = 250,                              \
                             .retransmit_count      = 3,                                \
                             .tx_mode               = UESB_TXMODE_AUTO,                 \
                             .radio_irq_priority    = 1}

enum uesb_event_type_t  {UESB_EVENT_TX_SUCCESS, UESB_EVENT_TX_FAILED, UESB_EVENT_RX_RECEIVED};

typedef enum {UESB_ADDRESS_PIPE0, UESB_ADDRESS_PIPE1, UESB_ADDRESS_PIPE2, UESB_ADDRESS_PIPE3, UESB_ADDRESS_PIPE4, UESB_ADDRESS_PIPE5, UESB_ADDRESS_PIPE6, UESB_ADDRESS_PIPE7} uesb_address_type_t;

typedef struct
{
    enum uesb_event_type_t  type;
}uesb_event_t;

typedef struct
{
    uint8_t length;
    uint8_t pipe;
    int8_t  rssi;
    uint8_t noack;
    uint8_t data[UESB_CORE_MAX_PAYLOAD_LENGTH];
}uesb_payload_t;

typedef struct
{
    uesb_payload_t *payload_ptr[UESB_CORE_TX_FIFO_SIZE];
    uint32_t        entry_point;
    uint32_t        exit_point;
    uint32_t        count;
}uesb_payload_tx_fifo_t;

typedef struct
{
    uesb_payload_t *payload_ptr[UESB_CORE_RX_FIFO_SIZE];
    uint32_t        entry_point;
    uint32_t        exit_point;
    uint32_t        count;
}uesb_payload_rx_fifo_t;

uint32_t uesb_init(uesb_config_t *parameters);

uint32_t uesb_disable(void);

bool     uesb_is_idle(void);

uint32_t uesb_write_tx_payload(uesb_payload_t *payload);

uint32_t uesb_write_tx_payload_noack(uesb_payload_t *payload);

uint32_t uesb_write_ack_payload(uesb_payload_t *payload);

uint32_t uesb_read_rx_payload(uesb_payload_t *payload);

uint32_t uesb_start_tx(void);

uint32_t uesb_start_rx(void);

uint32_t uesb_stop_rx(void);

uint32_t uesb_get_tx_attempts(uint32_t *attempts);

uint32_t uesb_flush_tx(void);

uint32_t uesb_flush_rx(void);

uint32_t uesb_get_clear_interrupts(uint32_t *interrupts);

uint32_t uesb_set_address(uesb_address_type_t address, const uint8_t *data_ptr);

uint32_t uesb_set_rf_channel(uint32_t channel);

uint32_t uesb_set_tx_power(uesb_tx_power_t tx_output_power);

#endif
