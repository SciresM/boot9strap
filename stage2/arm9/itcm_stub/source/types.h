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
#define IRQ_IE          (*(vu32 *)0x10001000)
#define IRQ_IF          (*(vu32 *)0x10001004)

#define BIT(n) (1u<<(n))

struct fb {
     u8 *top_left;
     u8 *top_right;
     u8 *bottom;
};

/*static inline void __wfi(void)
{
    u32 val = 0;
    __asm__ __volatile__("mcr p15, 0, %0, c7, c0, 4" :: "r"(val) : "memory");
}*/

void __wfi(void); // in cache.s
