.arm.little

.open "raw.bin","code9.bin",0x08000200

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
; Useful constant addresses.

b9_memcpy equ 0xFFFF03F0
b9_store_addr equ 0x08010000
b11_store_addr equ 0x08020000
b11_axi_addr equ 0x1FFC0000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; b9_hook_1: Wait for boot11 to finish a task, then overwrite a function pointer
;            it will call just before lockout. Then, setup the MPU, and return.
b9_hook_1:
    pwn_b11:
        stmfd sp!, {r0-r6, lr}
        ldr r0, =0x1FFE802C ; r0 = b11_funcptr_address
        ldr r1, =0x1FFB2E00 ; r1 = our_boot11_hook
        str r1, [r0]        ; overwrite value
        wait_loop:          ; This is actually a ToCToU race condition.
            ldr r2, [r0]        ; Derefence      
            cmp r2, r1          ; Has stored value changed?
            beq wait_loop       ; If not, go back.
        str r1, [r0]        ; Overwrite final funcptr.

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

.Close
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.open "raw.bin","code11.bin",0x1FFB2E00
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


.Close