#pragma once

#include "types.h"

typedef struct NdmaChannelRegisters {
    u32 src_addr;
    u32 dst_addr;
    u32 total_words;
    u32 block_words;
    u32 interval_timer_cnt;
    u32 fill_data;
    u32 cnt;
} NdmaChannelRegisters;

typedef struct NdmaRegisters {
    u32 cnt;
    NdmaChannelRegisters channel[8];
} NdmaRegisters;

#define REG_NDMA ((volatile NdmaRegisters *)0x10002000)

// Global cnt
// n is the number of CPU cycles
#define NDMA_ROUND_ROBIN(n)    (BIT(31) | (__builtin_ffs(n) - 1) << 16 | BIT(0))

#define NDMA_INTERVAL_TIMER_DEFAULT (0u)

enum
{
    NDMA_UPDATE_INC     = 0u,
    NDMA_UPDATE_DEC     = 1u,
    NDMA_UPDATE_FIXED   = 2u,
    NDMA_UPDATE_FILL    = 3u, // for filling mode, only
    NDMA_UPDATE_RELOAD  = 4u,
};

#define NDMA_DST_UPDATE_MODE(m) ((m) << 10)
#define NDMA_SRC_UPDATE_MODE(m) ((m) << 13)

#define NDMA_BURST_WORDS(n)    ((__builtin_ffs(n) - 1)<<16)

enum
{
    NDMA_STARTUP_TIMER0       =  0u <<24,
    NDMA_STARTUP_TIMER1       =  1u <<24,
    NDMA_STARTUP_TIMER2       =  2u <<24,
    NDMA_STARTUP_TIMER3       =  3u <<24,
    NDMA_STARTUP_CTRCARD0     =  4u <<24,
    NDMA_STARTUP_CTRCARD1     =  5u <<24,
    NDMA_STARTUP_SDIO1        =  6u <<24,
    NDMA_STARTUP_SDIO3        =  7u <<24, // not used on official retail fw, can be used for SD card
    NDMA_STARTUP_AES_IN       =  8u <<24,
    NDMA_STARTUP_AES_OUT      =  9u <<24,
    NDMA_STARTUP_SHA_IN       = 10u <<24,
    NDMA_STARTUP_SHA_OUT      = 11u <<24,
    NDMA_STARTUP_DEV2DEV      = 15u <<24,

    NDMA_STARTUP_CTRCARD0_TO_AES = NDMA_STARTUP_DEV2DEV | 0,
    NDMA_STARTUP_CTRCARD1_TO_AES = NDMA_STARTUP_DEV2DEV | 1,
    NDMA_STARTUP_AES_TO_CTRCARD0 = NDMA_STARTUP_DEV2DEV | 2,
    NDMA_STARTUP_AES_TO_CTRCARD1 = NDMA_STARTUP_DEV2DEV | 3,

    NDMA_STARTUP_CTRCARD0_TO_SHA = NDMA_STARTUP_DEV2DEV | 4,
    NDMA_STARTUP_CTRCARD1_TO_SHA = NDMA_STARTUP_DEV2DEV | 5,
    NDMA_STARTUP_SHA_TO_CTRCARD0 = NDMA_STARTUP_DEV2DEV | 6,
    NDMA_STARTUP_SHQ_TO_CTRCARD1 = NDMA_STARTUP_DEV2DEV | 7,

    NDMA_STARTUP_SDIO1_TO_AES = NDMA_STARTUP_DEV2DEV | 8,
    NDMA_STARTUP_SDIO3_TO_AES = NDMA_STARTUP_DEV2DEV | 9,
    NDMA_STARTUP_AES_TO_SDIO1 = NDMA_STARTUP_DEV2DEV | 10,
    NDMA_STARTUP_AES_TO_SDIO3 = NDMA_STARTUP_DEV2DEV | 11,

    NDMA_STARTUP_SDIO1_TO_SHA = NDMA_STARTUP_DEV2DEV | 12,
    NDMA_STARTUP_SDIO3_TO_SHA = NDMA_STARTUP_DEV2DEV | 13,
    NDMA_STARTUP_SHA_TO_SDIO1 = NDMA_STARTUP_DEV2DEV | 14,
    NDMA_STARTUP_SHA_TO_SDIO3 = NDMA_STARTUP_DEV2DEV | 15,

    NDMA_STARTUP_AES_TO_SHA   = NDMA_STARTUP_DEV2DEV | 16,
    NDMA_STARTUP_SHA_TO_AES   = NDMA_STARTUP_DEV2DEV | 17,
};

#define NDMA_NORMAL_MODE        (0u)
#define NDMA_IMMEDIATE_MODE     BIT(28)
#define NDMA_REPEATING_MODE     BIT(29)
#define NDMA_IRQ_ENABLE         BIT(30)
#define NDMA_ENABLE             BIT(31)

static inline void ndmaInit(void)
{
    for (u32 i = 0; i < 8; i++)
    {
        REG_NDMA->channel[i].cnt &= ~NDMA_ENABLE;
        REG_NDMA->channel[i].interval_timer_cnt = NDMA_INTERVAL_TIMER_DEFAULT;
    }
    REG_NDMA->cnt = NDMA_ROUND_ROBIN(8);
}

static inline void ndmaCopyAsync(u32 channel, u32 dst, u32 src, size_t size)
{
    volatile NdmaChannelRegisters *const chan = &REG_NDMA->channel[channel];
    chan->src_addr = src;
    chan->dst_addr = dst;
    chan->block_words = size/4;
    chan->interval_timer_cnt = NDMA_INTERVAL_TIMER_DEFAULT;
    chan->cnt = NDMA_ENABLE | NDMA_IRQ_ENABLE | NDMA_IMMEDIATE_MODE | NDMA_BURST_WORDS(1) |
                NDMA_DST_UPDATE_MODE(NDMA_UPDATE_INC) | NDMA_SRC_UPDATE_MODE(NDMA_UPDATE_INC);
}
