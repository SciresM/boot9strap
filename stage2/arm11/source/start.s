.section .text.start
.align 4
.global _start
.type   _start, %function
_start:
b start

.global operation
operation:
    .word 0

start:
    cpsid aif

    @ Set the control register to reset default: everything disabled
    ldr r0, =0x54078
    mcr p15, 0, r0, c1, c0, 0

    @ Set the auxiliary control register to reset default.
    @ Enables instruction folding, static branch prediction,
    @ dynamic branch prediction, and return stack.
    mov r0, #0xF
    mcr p15, 0, r0, c1, c0, 1

    @ Invalidate all caches, flush the prefetch buffer and DSB
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 4
    mcr p15, 0, r0, c7, c7, 0
    mcr p15, 0, r0, c7, c10, 4

    @ Clear BSS
    ldr r0, =__bss_start
    mov r1, #0
    ldr r2, =__bss_end
    sub r2, r0
    bl memset32

    ldr sp, =__stack_top__
    b main

.global prepareForFirmlaunch
.type   prepareForFirmlaunch, %function
prepareForFirmlaunch:
    str r0, [r1]            @ tell ARM9 we're done
    mov r0, #0x20000000

    _wait_for_core0_entrypoint_loop:
        ldr r1, [r0, #-4]   @ check if core0's entrypoint is 0
        cmp r1, #0
        beq _wait_for_core0_entrypoint_loop

    bx r1                   @ jump to core0's entrypoint
prepareForFirmlaunchEnd:

.global prepareForFirmlaunchSize
prepareForFirmlaunchSize: .word prepareForFirmlaunchEnd - prepareForFirmlaunch
