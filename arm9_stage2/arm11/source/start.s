.section .text.start
.align 4
.global _start
_start:
    cpsid aif
    b main
