.section .text.start
.align 4
.global _start
.type   _start, %function
_start:
b start

.global arm11SectionStarted
arm11SectionStarted:
    .word 0

start:
    cpsid aif
    ldr sp, =__stack_top__
    b main

.global prepareForFirmlaunch
.type   prepareForFirmlaunch, %function
prepareForFirmlaunch:
    mov r0, #0x20000000

    mov r1, #0
    str r1, [r0, #-4]       @ entrypoint
    str r1, [r0, #-0x10]    @ operation

    _wait_for_core0_entrypoint_loop:
        ldr r1, [r0, #-4]   @ check if core0's entrypoint is 0
        cmp r1, #0
        beq _wait_for_core0_entrypoint_loop

    bx r1                   @ jump to core0's entrypoint
prepareForFirmlaunchEnd:

.global prepareForFirmlaunchSize
prepareForFirmlaunchSize: .word prepareForFirmlaunchEnd - prepareForFirmlaunch
