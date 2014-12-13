# HKR3000 and XR3100 receiver disassembly

This folder contains **control files** for the D52 disassembler to disassemble firmware images extracted from the HobbyKing HKR3000 and XR3100 receiver.

Note that the firmware images are not provided, you have to read it out from your own receivers using any of the widely availblae NRF24LE1 programmers.


Required tools:

- D52 ([http://www.bipom.com/dis51.php](http://www.bipom.com/dis51.php))

- GNU Make ([https://www.gnu.org/software/make/](https://www.gnu.org/software/make/);
  Windows executable is available at [http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm))

- If you are using Windows you also need **cp.exe** and **rm.exe** from [coreutils](http://gnuwin32.sourceforge.net/downlinks/coreutils-bin-zip.php).


After you've installed the required tools, run **make**, which will create assembler files with the extension **.d52**.


For test purpose you can run **make bin**, which will re-assemble the **.d52** files. You can then compare the original firmware image with the re-assembled one. They should perfectly match.

For re-assembly you need additional tools:

-  MCU 8051 IDE ([http://sourceforge.net/projects/mcu8051ide/](http://sourceforge.net/projects/mcu8051ide/))

- objcopy ([https://www.gnu.org/software/binutils/](https://www.gnu.org/software/binutils/))
  If you are using Windows you can get them via [MinGW](http://www.mingw.org/)
