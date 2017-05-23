#pragma once

#include <stdint.h>
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

#define SCREEN_TOP_WIDTH     400
#define SCREEN_BOTTOM_WIDTH  320
#define SCREEN_HEIGHT        240
#define SCREEN_TOP_FBSIZE    (3 * SCREEN_TOP_WIDTH * SCREEN_HEIGHT)
#define SCREEN_BOTTOM_FBSIZE (3 * SCREEN_BOTTOM_WIDTH * SCREEN_HEIGHT)

typedef enum
{
    NO_ARM11_OPERATION = 0,
    INIT_SCREENS,
    WAIT_BOOTROM11_LOCKED,
    PREPARE_ARM11_FOR_FIRMLAUNCH,
} Arm11Operation;
