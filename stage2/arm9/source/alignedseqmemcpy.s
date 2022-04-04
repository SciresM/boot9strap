#include "asm_macros.s.h"
.arm
.cpu arm946e-s

// Align on cache line boundaries & make sure the loops don't cross them.
FUNCTION alignedseqmemcpy, .text, 5
    // src=r1 and dst=r0 are expected to be 4-byte-aligned
    push    {r4-r10, lr}

    lsrs    r12, r2, #5
    sub     r2, r2, r12, lsl #5
    beq     2f

1:
    ldmia   r1!, {r3-r10}
    stmia   r0!, {r3-r10}
    subs    r12, #1
    bne     1b

2:
    lsrs    r12, r2, #2
    sub     r2, r2, r12, lsl #2
    beq     4f

3:
    ldr     r3, [r1], #4
    str     r3, [r0], #4
    subs    r12, #1
    bne     3b

4:
    tst     r2, #2
    ldrneh  r3, [r1], #2
    strneh  r3, [r0], #2

    tst     r2, #1
    ldrneb  r3, [r1], #1
    strneb  r3, [r0], #1

    pop     {r4-r10, pc}
END_FUNCTION
