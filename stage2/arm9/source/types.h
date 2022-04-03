/*
*   types.h
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

//Common data types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

#define CFG9_SYSPROT9   (*(vu8 *)0x10000000)
#define CFG9_SYSPROT11  (*(vu8 *)0x10000001)

#define BIT(n) (1u<<(n))
#define NORETURN __attribute__((noreturn))

typedef enum
{
    INIT_SCREENS = 0,
    WAIT_BOOTROM11_LOCKED,
    PREPARE_ARM11_FOR_FIRMLAUNCH,
    ARM11_READY
} Arm11Operation;

/*static inline void __wfi(void)
{
    u32 val = 0;
    __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" :: "r"(val) : "memory");
}*/

void __wfi(void); // in cache.s
