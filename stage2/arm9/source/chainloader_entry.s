#include "asm_macros.s.h"
.arm
.cpu arm946e-s

FUNCTION chainload, .chainloader
    ldr     sp, =__itcm_stack_top__
    b       chainloaderMain
END_FUNCTION

FUNCTION copyFirmSection, .chainloader
    push    {r4-r12, lr}
    cmp     r2, #0
    beq     2f
1:
    // Copy blocks of 32 bytes
    ldm     r1!, {r4-r11}
    stm     r0!, {r4-r11}
    subs    r2, #32
    bne     1b
2:
    pop     {r4-r12, pc}
END_FUNCTION

FUNCTION disableMpuAndJumpToEntrypoints, .chainloader
    mov     r4, r0
    mov     r5, r1
    mov     r6, r2
    mov     r7, r3

    // Old versions of b9s used ITCM at 0x01FF8000 so we need to translate
    // the address contained in argv
    ldm     r5, {r0, r1}
    sub     r0, r0, #0x06000000
    cmp     r1, #0
    subne   r1, r1, #0x06000000
    stm     r5, {r0, r1}
    sub     r5, r5, #0x06000000

    // Clean and invalidate DCache
    ldr     r12, =0xFFFF0830
    blx     r12

    // Disable caches / MPU
    mrc     p15, 0, r0, c1, c0, 0  // read control register
    bic     r0, #(1<<12)           // - instruction cache disable
    bic     r0, #(1<<2)            // - data cache disable
    bic     r0, #(1<<0)            // - MPU disable
    mcr     p15, 0, r0, c1, c0, 0  // write control register

    // Set the Arm11 entrypoint
    mov     r0, #0x20000000
    str     r7, [r0, #-4]

    // Invalidate caches and dsb
    mov     r8, #0
    mcr     p15, 0, r8, c7, c5, 0
    mcr     p15, 0, r8, c7, c6, 0
    mcr     p15, 0, r8, c7, c10, 4

    // Jump to the Arm9 entrypoint
    mov     r0, r4
    mov     r1, r5
    ldr     r2, =(VERSION_MINOR << 16 | 0xBEEF)

    bx      r6
END_FUNCTION
