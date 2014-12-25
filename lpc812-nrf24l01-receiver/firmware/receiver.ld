MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x4000            /* 16K */
  RAM (rwx) : ORIGIN = 0x10000000, LENGTH = 0x1000      /* 4K */
}

SECTIONS
{
    .vectors :
    {
        _text = .;
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH


    .text :
    {
        KEEP(*(.after_vectors))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH


    .data : AT (_etext)
    {
        _data = .;
        *(.survivors)       /* 80 bytes that survive reset in LPC81x */
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM


    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM


    /* Use the top-most page in flash to store the persistent data */
    .persistent_data (0x4000 - 64) :
    {
        KEEP(*(.persistent_data))
    } > FLASH


    /* The stack starts 32 bytes below to of RAM as those 32 bytes are needed
     * for the IAR ROM function that we use to write the Flash with persistent
     * data.
     */
    _stacktop = ORIGIN(RAM) + LENGTH(RAM) - 32;
}
