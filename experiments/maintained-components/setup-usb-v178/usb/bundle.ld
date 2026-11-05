SECTIONS { .text 0 : { *(.text*) *(.rodata*) } /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) } }
