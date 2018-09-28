#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <LPC8xx.h>


#define __SYSTICK_IN_MS 10

#define NUMBER_OF_CHANNELS 3
#define SERVO_PULSE_CENTER 1500
#define INITIAL_ENDPOINT_DELTA 200


// ****************************************************************************
// IO pins:
//
//                              4-ch/TSSOP16          8ch/TSSOP20
//
// PIO0_0   (TDO, ISP-Rx)       16  CH3/Rx            19  CH2/Rx
// PIO0_1   (TDI)               9   NRF_SCK           12  CH6
// PIO0_2   (TMS, SWDIO)        6   NRF_MOSI          7   NRF_CSN
// PIO0_3   (TCK, SWCLK)        5   NRF_CSN           6   NRF_SCK
// PIO0_4   (TRST, ISP-Tx)      4   CH4/CPPM/Tx       5   CH5/CPPM/Tx
// PIO0_5   (RESET)             3   LED               4   NRF_MOSI
// PIO0_6                       15  CH2               16  CH3
// PIO0_7                       14  CH1               17  CH4
// PIO0_8   (XTALIN)            11  XTALIN            14  XTALIN
// PIO0_9   (XTALOUT)           10  XTALOUT           13  XTALOUT
// PIO0_10  (Open drain)        8   NRF_MISO          9   NRF_MISO
// PIO0_11  (Open drain)        7   NRF_IRQ           8   NRF_IRQ
// PIO0_12  (ISP-entry)         2   ISP/BIND          3   ISP/BIND
// PIO0_13                      1   NRF_CE            2   CH8
// PIO0_14                                            20  CH1
// PIO0_15                                            11  LED
// PIO0_16                                            10  NRF_CE
// PIO0_17                                            11  CH7
//
// 3.3V                         12                    15
// GND                          13                    16
// ****************************************************************************

// Common GPIOs
#define GPIO_BIT_BIND 12
#define GPIO_BIT_RX 0
#define GPIO_BIT_TX 4

#define GPIO_4CH_BIT_CH1 7
#define GPIO_4CH_BIT_CH2 6
#define GPIO_4CH_BIT_CH3 0
#define GPIO_4CH_BIT_CH4 4
#define GPIO_4CH_BIT_LED 5
#define GPIO_4CH_BIT_NRF_SCK 1
#define GPIO_4CH_BIT_NRF_MOSI 2
#define GPIO_4CH_BIT_NRF_CSN 3
#define GPIO_4CH_BIT_NRF_MISO 11
#define GPIO_4CH_BIT_NRF_IRQ 10
#define GPIO_4CH_BIT_NRF_CE 13

#define GPIO_8CH_BIT_CH1 14
#define GPIO_8CH_BIT_CH2 0
#define GPIO_8CH_BIT_CH3 6
#define GPIO_8CH_BIT_CH4 7
#define GPIO_8CH_BIT_CH5 4
#define GPIO_8CH_BIT_CH6 1
#define GPIO_8CH_BIT_CH7 17
#define GPIO_8CH_BIT_CH8 13
#define GPIO_8CH_BIT_LED 15
#define GPIO_8CH_BIT_NRF_SCK 3
#define GPIO_8CH_BIT_NRF_MOSI 5
#define GPIO_8CH_BIT_NRF_CSN 2
#define GPIO_8CH_BIT_NRF_MISO 11
#define GPIO_8CH_BIT_NRF_IRQ 10
#define GPIO_8CH_BIT_NRF_CE 16

extern uint32_t gpio_mask_led;
extern uint32_t gpio_mask_nrf_ce;


void invoke_ISP(void);
void delay_us(uint32_t microseconds);

extern bool is8channel;