ENTRY(_start)

BASE_ADDRESS        =   0x80200000;

SECTIONS {
    . = BASE_ADDRESS;

    PROVIDE(kernelStart = .);

    . = ALIGN(4096);

    PROVIDE(textStart = .);
    .text : {
        *(.boot)
        *(.text .text.*)
        . = ALIGN(4096);
        _trampoline = .;
        *(.trampoline)
        . = ALIGN(4096);
        ASSERT(. - _trampoline == 8192, "error: trampoline not have two page");
        _signalTrampoline = .;
        *(.signalTrampoline)
        . = ALIGN(4096);
        ASSERT(. - _signalTrampoline == 4096, "error: signal trampoline not one page");
        PROVIDE(textEnd = .);
    }

    . = ALIGN(4096);
    rodataStart = .;
    .rodata : {
        *(.rodata .rodata.*)
        rodataEnd = .;
    }

    . = ALIGN(4096);
    dataStart = .;
    .data : {
        *(.data .data.*)
        . = ALIGN(4096);
        _stack = .;
        *(.stack)
        dataEnd = .;
    }

    . = ALIGN(4096);
    PROVIDE(bssStart = .);
    .bss : {
        *(.bss .bss.* .sbss .sbss.*)
        . = ALIGN(4096);
        _pgdir = .;
        *(.pgdir)
        ASSERT(. - _pgdir == 4096, "error: pgdir not one page");
        . = ALIGN(4096);
        PROVIDE(bssEnd = .);
    }

    . = ALIGN(4096);
    PROVIDE(kernelEnd = .);
}