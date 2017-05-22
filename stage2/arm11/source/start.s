.section .text.start
.align 4
.global _start
.type   _start, %function
_start:
    cpsid aif
    ldr sp, =__stack_top__
    b main

.global prepareForFirmlaunch
.type   prepareForFirmlaunch, %function
prepareForFirmlaunch:
    ldr r0, =0x1FFFFFFC
    mov r1, #0
    str r1, [r0]        @ zero out core0's entrypoint

    _wait_for_core0_entrypoint_loop:
        ldr r1, [r0]    @ check if core0's entrypoint is 0
        cmp r1, #0
        beq _wait_for_core0_entrypoint_loop

    bx r1               @ jump to core0 entrypoint
prepareForFirmlaunchEnd:

.global prepareForFirmlaunchSize
prepareForFirmlaunchSize: .word prepareForFirmlaunchEnd - prepareForFirmlaunch
