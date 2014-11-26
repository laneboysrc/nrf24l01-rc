#pragma once

#include <LPC8xx.h>


#define __SYSTICK_IN_MS 10

#define NUMBER_OF_CHANNELS 3
#define SERVO_PULSE_CENTER 1500 * 4 / 3
#define INITIAL_ENDPOINT_DELTA 250


// ****************************************************************************
// IO pins: (LPC812 in TSSOP16 package)
//
// PIO0_0   (16, TDO, ISP-Rx)   CH3/Rx
// PIO0_1   (9,  TDI)           NRF_SCK
// PIO0_2   (6,  TMS, SWDIO)    NRF_MOSI
// PIO0_3   (5,  TCK, SWCLK)    NRF_CSN
// PIO0_4   (4,  TRST, ISP-Tx)  CH4/CPPM/Tx
// PIO0_5   (3,  RESET)         LED
// PIO0_6   (15)                CH2
// PIO0_7   (14)                CH1
// PIO0_8   (11, XTALIN)        XTALIN
// PIO0_9   (10, XTALOUT)       XTALOUT
// PIO0_10  (8,  Open drain)    NRF_MISO
// PIO0_11  (7,  Open drain)    NRF_IRQ
// PIO0_12  (2,  ISP-entry)     ISP/BIND
// PIO0_13  (1)                 NRF_CE
//
// 3.3V     (12)
// GND      (13)
// ****************************************************************************

#define GPIO_BIT_CH1 7
#define GPIO_BIT_CH2 6
#define GPIO_BIT_CH3 8      // FIXME 0 in actual finished firmware
#define GPIO_BIT_UART_TX 4
#define GPIO_BIT_BIND 12
#define GPIO_BIT_LED 9      // FIXME: 5 in actual finished firmware!
#define GPIO_BIT_NRF_SCK 1
#define GPIO_BIT_NRF_MOSI 2
#define GPIO_BIT_NRF_CSN 3
#define GPIO_BIT_NRF_MISO 10
#define GPIO_BIT_NRF_IRQ 11
#define GPIO_BIT_NRF_CE 13

#define GPIO_NRF_CE LPC_GPIO_PORT->W0[GPIO_BIT_NRF_CE]
#define GPIO_BIND LPC_GPIO_PORT->W0[GPIO_BIT_BIND]
#define GPIO_LED LPC_GPIO_PORT->W0[GPIO_BIT_LED]

void invoke_ISP(void);
