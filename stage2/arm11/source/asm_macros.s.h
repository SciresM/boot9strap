.macro FUNCTION name, section=.text, algn=2
    .section        \section\().\name, "ax", %progbits
    .align          \algn
    .global         \name
    .type           \name, %function
    .func           \name
    .cfi_sections   .debug_frame
    .cfi_startproc
    \name:
.endm
.macro END_FUNCTION
    .cfi_endproc
    .endfunc
.endm
