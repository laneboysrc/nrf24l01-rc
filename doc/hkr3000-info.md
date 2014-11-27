Transmitter:
; The received data is 12 bit, with center (1500us) being 0x465
; The data sent across the air is the timer value that has to be set
; to give the desired time until the timer overflows. The timer tick is
; 750ns.
;
; The algorithm to calculate the timer value is as follows:
;
;   timer_value = (uart_data * 14 / 10) + 0xf200


Wireless configuration:

- 10 byte payload size
- 2 byte CRC
- 5 byte address size
- 20 hop channels
- One hop every 5 ms
- 3 packets every 5 ms: 2 times stick or failsafe data, 1 time bind data


Binding procedure:
The transmitter regularly sends data on the fixed channel 51h (81), with address
12:23:23:45:78.
This data is sent at a lower power. All transmitters do that all the time.

A bind data packet is sent every 5ms.

The transmitter cycles through 4 packets:
ff aa 55 a1 a2 a3 a4 a5 .. ..
cc cc 00 ha hb hc hd he hf hg
cc cc 01 hh hi hj hk hl hm hn
cc cc 02 ho hp hq hr hs ht ..

ff aa 55     Special marker for the first packet
a[1-5]       The 5 address bytes
cc cc        A 16 byte checksum of bytes a1..a5
h[a-t]       20 channels for frequency hopping
..           Not used





Packet format:

f1 f7 4a f8 8e f5 b8 55 67 0f     actual stick packet
40 f8 40 f8 8e f5 b8 aa 5a 0f     actual fs packet


Stick data:
    0     1     2     3     4     5     6    7    8    9
    ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h ???  0x55 0x67 ???
                                             ^^^^
                                             Packet id

Bytes 6, 8 and 9 not touched by the receiver


Failsafe data:
    0     1     2     3     4     5     6    7    8    9
    ST-l  ST-h  TH-l  TH-h  CH3-l CH3-h ???  0xaa 0x5a ???   (failsafe on)
                                                  0x5b       (failsafe off)
                                             ^^^^
                                             Packet id

Bytes 6 and 9 not touched by the receiver


Stick values are sent as direct timer values for a MCS51 timer with a 750ns
clock. To calculate milliseconds of servo pulse duration, use the following
formula:

    servo_pules_in_ms = (0xffff - stickdata) * 3 / 4;



Misc and weird stuff:

- The receiver is designed for 4 channels, but it seems that it was hacked to
  support failsafe, destroying the 4th channel value in payload byte 7.

- If failsafe is off, the receiver returns the steering and throttle channel
  to 1500 us pulse width when the signal is lost.

- The transmitter always generates hop channels in sequencial order. This is
  poor for resiliance against interference, and unnecessary.
  This is especially unfortunate because the transmitter uses the hardware
  random number generator of the nRF chip to obtain a truely random address
  and start of hop.

- The nRF24LE1 contains a persistent storage mechanism. Yet the receivers use
  an external EEPROM.

- The firmware for the HKR3000 and XR3100 are identical except for different
  pin usage.

- The failsafe time is 640 ms. I.e if no stick data packet is received
  within 640 ms failsafe is triggered.

- The 2-wire function (I2C) of the nRF24LE1 is not used but I2C is bit-banged.

- The transmitter only generates the first hop channels between 0 and 49
  (modulo 32h). So the channel range from 0 to 69. Maximum possible would be
  125 according to the data sheet.
  Not sure why the limit is there. It does not seem to be regulatory.
  Possibly an issue with the transmitter PA or receiver LNA?
  To be tested.
