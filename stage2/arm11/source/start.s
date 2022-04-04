#include "asm_macros.s.h"

.arm
.cpu mpcore

FUNCTION _start, .crt0
    b       start

.global operation
operation:
    .word   0

start:
    cpsid   aif

    // Set the control register to reset default: everything disabled
    ldr     r0, =0x54078
    mcr     p15, 0, r0, c1, c0, 0

    // Set the auxiliary control register to reset default.
    // Enables instruction folding, static branch prediction,
    // dynamic branch prediction, and return stack.
    mov     r0, #0xF
    mcr     p15, 0, r0, c1, c0, 1

    // Invalidate all caches, flush the prefetch buffer and DSB
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4
    mcr     p15, 0, r0, c7, c7, 0
    mcr     p15, 0, r0, c7, c10, 4

    // Clear BSS
    ldr     r0, =__bss_start__
    mov     r1, #0
    ldr     r2, =__bss_end__
    sub     r2, r0
    bl      memset

    // Copy the firmlaunch code
    ldr     r1, =prepareForFirmlaunch
    ldr     r0, =0x1FFFFC00
    ldm     r1, {r4-r11}
    stm     r0, {r4-r11}

    // DSB just in case
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4

    ldr     sp, =__stack_top__
    b       arm11Main
END_FUNCTION

FUNCTION prepareForFirmlaunch, .crt0
    str     r0, [r1]            // tell ARM9 we're done
    mov     r0, #0x20000000

    // DSB just in case
    mov     r4, #0
    mcr     p15, 0, r4, c7, c10, 4

    1:
        ldr     r1, [r0, #-4]   // check if core0's entrypoint is 0
        cmp     r1, #0
        beq     1b

    bx r1                       // jump to core0's entrypoint
.if (. - prepareForFirmlaunch) != 32
    .error "prepareForFirmlaunch has incorrect size"
.endif
END_FUNCTION
