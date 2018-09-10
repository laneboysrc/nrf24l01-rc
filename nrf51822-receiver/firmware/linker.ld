/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x0,        LENGTH = 128k
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 16k
}

SECTIONS
{
    .fs_data_out ALIGN(4):
    {
        PROVIDE( __start_fs_data = .);
        KEEP(*(fs_data))
        PROVIDE( __stop_fs_data = .);
    } = 0
}

INCLUDE "nrf5x_common.ld"
