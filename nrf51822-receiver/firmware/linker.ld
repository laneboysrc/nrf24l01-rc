/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

page_size = 1024;

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

SECTIONS
{
    /* Use the top-most page in flash to store the persistent data */
    .persistent_data (ORIGIN(FLASH) + LENGTH(FLASH) - page_size) :
    {
        KEEP(*(.persistent_data))
    } > FLASH
}