/* Linker script for AliExpress STM32F103 board (STM32F103C8T6, 64K flash, 20K RAM). */

MEMORY
{
	rom (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}


/* Put the transmitter configuration in the upper-most 8 KBytes of flash */
SECTIONS
{
    .transmitter_configuration 0x0800e000 :
    {
        *(.rodata.config_flash)
    }
}


INCLUDE libopencm3_stm32f1.ld


