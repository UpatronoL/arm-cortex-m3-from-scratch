MEMORY {
    FLASH   (rx)    : ORIGIN = 0x0, LENGTH = 256K
    SRAM    (rw)    : ORIGIN = 0x20000000, LENGTH = 64K
}

_stack_top = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS {
    .vector_table 0x00000000 : {
        *(.vector_table)
    } >FLASH
    .text : {
        *(.text);
        _etext = .;
    } >FLASH
    .data : AT( ADDR(.text) + SIZEOF(.text) ){
        _data = .; /* pointer to the beggining of the data section*/
        *(.data); /* all the relevant data from all the sections*/
        _edata = .; /* pointer to the end of the data section*/

    } >SRAM
    .bss : {
        _bstart = .;
        *(.bss) *(COMMON);
        _bend = .;
    } >SRAM
}
