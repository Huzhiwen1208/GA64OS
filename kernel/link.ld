SECTIONS
{
    . = 0x40100000;  /* Base address */

    .text : {
        KEEP(*(.start.text))
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    . = ALIGN(8);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    

    _end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.gnu*)
        *(.eh_frame*)
    }
}

__bss_size = (__bss_end -__bss_start)>>3;