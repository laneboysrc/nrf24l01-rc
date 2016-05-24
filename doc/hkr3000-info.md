# HKR3000 and XR3100 receiver technical information

Both receivers use the [Nordic NRF24LE1](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24LE1) RF chip. It is a composite device containing the RF logic of the popular NRF24L01+ and an 8051 MCU.

Unfortunately both receivers use the OTP version, not the Flash version, so they can not be re-programmed.

The HKR3000 has an LNA (Low Noise Amplifier), the XR3100 connects the antenna directly the NRF24LE1. In theory this should give the HKR3000 better range, but in practice I haven't noticed any difference between them.


## Transmitter

The HK310 and 3XS transmitter also contains an NRF24LE1, which receives channel 1, 2 and 3 data (we call this **stick data**) from another microcontroller via UART. The stick data over the UART is 12 bit, with center (1500 us) being 0x465.
The NRF24LE1 in the transmitter converts this stick data into a 16-bit timer value that can be directly loaded into one of the timers on the NRF24LE1's 8051 MCU. These 16-bits are sent over the air. The timer tick is assumed to be 750 ns (16 MHz / 12).

The algorithm to calculate the timer value is as follows:

        timer_value = (uart_data * 14 / 10) + 0xf200


## NRF24 RF configuration

- 10 byte payload size
- 5 byte address size
- 2 byte CRC
- 20 hop channels
- One hop every 5 ms
- Transmitter sends three packets every 5 ms: two stick (or failsafe) packets, one bind packet
- Uni-directional data transfer (transmitter only transmits, receiver only receives; no acknowledgement or telemetry)


## Stick and failsafe packet format

Example payload of actual captured packets (10 bytes payload):
```
f1 f7 4a f8 8e f5 b8 55 67 0f     stick packet
40 f8 40 f8 8e f5 b8 aa 5a 0f     failsafe packet
```

Stick data:
```
0     1     2     3     4     5     6    7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h ???  0x55 0x67 ???
                                         ^^^^
                                         Packet id
```
Bytes 6, 8 and 9 are ignored by the receiver


Failsafe data:
```
0     1     2     3     4     5     6    7    8    9
ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h ???  0xaa 0x5a ???   (failsafe on)
                                              0x5b       (failsafe off)
                                         ^^^^
                                         Packet id
```
Bytes 6 and 9 are ignored by the receiver


Stick values are sent as direct timer values for an 8051 timer with a 750 ns clock. To calculate milliseconds of servo pulse duration, use the following formula:

    servo_pules_in_us = (0xffff - stickdata) * 3 / 4;


## Binding procedure:

The transmitter regularly sends data on the fixed channel **0x51** (81 decimal), with address **12:23:23:45:78** (hex). This data is sent at a lower power (PA in the transmitter turned off, NRF24 power set to a low power level). All transmitters do that all the time. Due to the low transmit power, the transmitter has to be brought physically very close (within a few cm) before binding can succeed.

The transmitter cycles through four type of bind packets:
```
ff aa 55 a1 a2 a3 a4 a5 .. ..
cc cc 00 ha hb hc hd he hf hg
cc cc 01 hh hi hj hk hl hm hn
cc cc 02 ho hp hq hr hs ht ..
```

``ff aa 55``     Special marker for the first bind packet

``a[1-5]``       The 5 address bytes

``cc cc``        The 16 bit sum of bytes a1..a5

``h[a-t]``       20 channels for frequency hopping

``..``           Not used


## Miscellaneous

- The firmware for the HKR3000 and XR3100 firmware are identical except for different I/O pin usage.

- The receiver was originally designed for 4 channels, but it seems that it was *hacked* to support failsafe, destroying the 4th channel value in payload byte 7. This was a weird decision as byte 9 could have been used, fully maintaining 4 channels.

- If failsafe is off in the transmitter, the receiver still applies a default failsafe value of 1500 us to steering and throttle channels when the signal is lost.

- The failsafe time is 640 ms. I.e failsafe is triggered when no stick data packet is received within 640 ms.

- Because the OTP version of the NRF24LE1 is used, a small external EEPROM had to be added to store 25 bytes of bind data (5 address bytes, 20 hop channel bytes). Using the Flash version of the NRF24LE1 can save this external component because the Flash version contains a high endurance NVM memory.

- The 2-wire function (I2C) of the nRF24LE1 is not used but I2C is bit-banged.

- The transmitter always generates hop channels in sequencial order. This is poor for resiliance against interference. This is especially unfortunate because the transmitter uses the hardware random number generator of the NRF24 chip to obtain a truely random address and start of hop.

- The transmitter only generates the first hop channels between 0 and 49 (modulo 0x32). So the channel range is from 0 to 69. Maximum possible would be 125 according to the data sheet. It is unclear why this limit has been implemented. It may be due to regulatory issues, or possibly because of limitiations in the transmitter PA or receiver LNA.
