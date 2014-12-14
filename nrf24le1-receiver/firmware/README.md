# Tools required to build the NRF24LE1 firmware

**Note: a pre-compiled firmware is available for download [here](receiver.bin).**

- **SDCC** [http://sdcc.sourceforge.net/](http://sdcc.sourceforge.net/)

- **GNU Make** [https://www.gnu.org/software/make/](https://www.gnu.org/software/make/)
  Windows executable is available at [http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm)

- **srecord package** [http://srecord.sourceforge.net/](http://srecord.sourceforge.net/)

- Windows users: **rm.exe** from [coreutils](http://gnuwin32.sourceforge.net/downlinks/coreutils-bin-zip.php).


# Building the firmware

Running ``make`` in a console window in this directory builds the firmware. The resulting *receiver.bin* and *receiver.hex* are located in the build directory.

Running ``make program`` flashes the firmware, assuming you are using the [nrf_prog](../../nrf_prog_v1_0/) programmer.

It may be advisable to check the ``makefile`` whether the settings are desired for your application.

You can build firmware images for the HKR3000 or XR3100 by running ``make hkr3000`` and ``make xr3100``. Note that those receivers include the OTP version, so you can only flash the firmware if you change to the NRF24LE1**E** (Flash) version.
