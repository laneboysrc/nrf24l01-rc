# Tools required to build the LPC812 firmware

**Note: a pre-compiled firmware is available for download [here](receiver.hex).**

- **GCC for ARM** [https://launchpad.net/gcc-arm-embedded/](https://launchpad.net/gcc-arm-embedded/)

- **GNU Make** [https://www.gnu.org/software/make/](https://www.gnu.org/software/make/);
  Windows executable is available at [http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm)

- Optional: LCP81x-ISP [https://github.com/laneboysrc/LPC81x-ISP-tool](https://github.com/laneboysrc/LPC81x-ISP-tool) Alternatively you can use any tool that can flash the LPC812 via ISP.

- Windows users: **rm.exe** from [coreutils](http://gnuwin32.sourceforge.net/downlinks/coreutils-bin-zip.php).


# Building the firmware

Running ``make`` in a console window in this directory builds the firmware. The resulting *receiver.bin* and *receiver.hex* are located in the build directory.

Running ``make program`` flashes the firmware, assuming you are using the *LCP81x-ISP* tool.

It may be advisable to check the ``makefile`` whether the settings are desired for your application.
