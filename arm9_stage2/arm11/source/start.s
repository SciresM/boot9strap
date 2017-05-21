.section .text.start
.align 4
.global _start
_start:
    cpsid aif
    ldr sp, =0x1FFFE000
    b main
