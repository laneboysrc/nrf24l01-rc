# HobbyKing HK310 and Turnigy 3XS compatible receivers

This project includes **electronics** and **firmware** to build **RC receivers** that are replacements for the HobbyKing [HKR3000](http://hobbyking.com/hobbyking/store/__12187__HK_3000_3ch_2_4ghz_FHSS_RX_suit_the_HK310_Transmitter.html) and [XR3100](http://hobbyking.com/hobbyking/store/__22910__HK3100_3ch_2_4ghz_FHSS_Micro_RX_suits_HK310_Turnigy_3XS_TX_.html) receivers. They can be used with the [HK300](http://hobbyking.com/hobbyking/store/__12177__HK_300_3_Channel_2_4ghz_FHSS_Ground_Radio.html), [HK310](http://hobbyking.com/hobbyking/store/__12181__HK_310_3_Channel_2_4ghz_FHSS_Ground_Radio.html) and [Turnigy 3XS](http://hobbyking.com/hobbyking/store/__24905__Turnigy_3XS_FHSS_2_4GHz_3_Channel_Radio_System.html) RC car radio systems.

The receivers make use of off-the-shelf inexpensive RF modules.

[IMAGE: all receivers]

Two variants are provided:

- [lpc812-nrf24l01](lpc812-nrf24l01-receiver/): NRF24L01+ module plus a [NXP LPC812](http://www.nxp.com/products/microcontrollers/cortex_m0_m0/series/LPC800.html) MCU
- [nrf24le1](nrf24le1-receiver/): Using a NRF24LE1 module (integrated 8051 MCU)


## Advantages of the NRF24L01+ - LPC812 version

- Easily obtainable and very low cost NRF module (less than USD 1 including shipping in qty 10 on eBay, Aliexpress)
- Very precise servo pulses due to LPC812 timer hardware
- ARM Cortex-M0 32-bit CPU is easy to program for with the GCC ARM compiler
- MCU has a ROM bootloader and can be flashed using any USB-to-serial dongle
- Servo pulses can be updated down to every 5 ms (200 Hz). In theory this may provide better response but in practice the transmitter would need to be modified to fully take
advantage of this fact.
- UART output using the LANE Boys RC [preprocessor protocol](http://laneboysrc.blogspot.com/2012/12/pre-processor-for-diy-rc-light.html), e.g. to hook up the [DIY RC light controller](https://github.com/laneboysrc/rc-light-controller) with a single servo cable.

Downsides:

- Requires a PCB and intermediate SMD soldering skills
- No CPPM output (but it has a UART output that can carry CPPM-like data and more)


## Advantages of the NRF24LE1 version

- Low part count: NRF24LE1 module, voltage regulator, a few capacitors; done!
- Simple to build even on a breadboard
- CPPM output
- UART output using the LANE Boys RC [preprocessor protocol](http://laneboysrc.blogspot.com/2012/12/pre-processor-for-diy-rc-light.html), e.g. to hook up the [DIY RC light controller](https://github.com/laneboysrc/rc-light-controller) with a single servo cable. Baudrate is limited to 38400 though.

Downsides:

- NRF24LE1 module is more expensive than NRF24L01 and harder to find
- Requires special programming hardware. Multiple open-source versions can be found on the Internet, but requires more work to get them going.
- Servo pulses are software timed, hence slightly jittery despite using interrupts. Note that the firmware included in this project is improved over the original HKR3000 and XR3100 firmware, which regulaerly extends servo pulses by 16 us due to poor interrupt logic implementation.
- For the PCB provided in this project you need to get the NRF24LE1 module version that has an SMD crystal (size 21 x 15 mm)

The NRF24LE1 firmware included in this project can be compiled to be compatible with the HKR3000 and XR3100 hardware. However, since those receivers use the OTP version (NRF24LE1G) of the NRF24LE1 one would have to replace the chip with the NR24LE1E versions to be able to use it.
