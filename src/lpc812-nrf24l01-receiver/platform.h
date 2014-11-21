#pragma once

#include <LPC8xx.h>


#define __SYSTICK_IN_MS 10


// ****************************************************************************
// IO pins: (LPC812 in TSSOP16 package)
//
// PIO0_0   (16, TDO, ISP-Rx)   Rx
// PIO0_1   (9,  TDI)           MISO
// PIO0_2   (6,  TMS, SWDIO)    SSEL
// PIO0_3   (5,  TCK, SWCLK)    SCK
// PIO0_4   (4,  TRST, ISP-Tx)  Tx
// PIO0_5   (3,  RESET)         RESET
// PIO0_6   (15)                RF interrupt
// PIO0_7   (14)                MOSI
// PIO0_8   (11, XTALIN)
// PIO0_9   (10, XTALOUT)
// PIO0_10  (8,  Open drain)
// PIO0_11  (7,  Open drain)
// PIO0_12  (2,  ISP-entry)     ISP
// PIO0_13  (1)                 RFCE
//
// GND      (13)
// 3.3V     (12)
// ****************************************************************************
#define GPIO_RFCE LPC_GPIO_PORT->W0[13]
