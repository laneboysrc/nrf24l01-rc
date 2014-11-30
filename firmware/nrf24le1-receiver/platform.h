#pragma once

#include <stdint.h>
#include <nrf24le1.h>


#define __SYSTICK_IN_MS 16

#define NUMBER_OF_CHANNELS 3
#define SERVO_PULSE_CENTER 1500 * 4 / 3
#define INITIAL_ENDPOINT_DELTA 250


// ****************************************************************************
// IO pins: (nRF24LE1 module 15x21 mm with 32pin QFN)
//
// Note: the pin numbers in the brackets refer to the module pinout, not
// the nRF24LE1 pinout!
//
// P0.0
// P0.1  (4 ) LED
// P0.2
// P0.3  (8 ) Tx
// P0.4  (9 ) Rx
// P0.5  (10) CH1 / FSCK
// P0.6  (12) BIND
// P0.7  (13) CH2 / FMOSI
// P1.0  (14) CH3 / FMISO
// P1.1  (15) FCSN
// P1.2
// P1.3
// P1.4
// P1.5
// P1.6
//
// 3.3V     (5 )
// GND      (11)
// PROG     (7 )
// RESET    (19)
// ****************************************************************************

#define GPIO_BIND P1_3
#define GPIO_LED P0_1
#define GPIO_CH1 P0_2
#define GPIO_CH2 P1_0
#define GPIO_CH3 P1_1


// ****************************************************************************
// Timer allocation
//
// Timer 0 provides a systick every 16ms, which is the interval in
// which servo pulses are output
//
// Timer 1 generates individual servo pulses. Its interrupt runs at highest
// priority to guarantee accurate servo pulses.
//
// Timer 2 is the hop timer.
//
// ****************************************************************************


void delay_us(uint16_t microseconds);
