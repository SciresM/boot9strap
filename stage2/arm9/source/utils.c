/*
*   This file is part of Luma3DS
*   Copyright (C) 2016 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b of GPLv3 applies to this file: Requiring preservation of specified
*   reasonable legal notices or author attributions in that material or in the Appropriate Legal
*   Notices displayed by works containing it.
*/

/*
*   waitInput function based on code by d0k3 https://github.com/d0k3/Decrypt9WIP/blob/master/source/hid.c
*/

#include "utils.h"
#include "i2c.h"
#include "cache.h"
#include "memory.h"

typedef struct McuInfoLedPattern {
    u8 delay;
    u8 smoothing;
    u8 loopDelay;
    u8 reserved;
    u8 red[32];
    u8 green[32];
    u8 blue[32];
} McuInfoLedPattern;
_Static_assert(sizeof(McuInfoLedPattern) == 100, "McuInfoLedPattern: wrong size");

static inline void startChrono(void)
{
    static bool isChronoStarted = false;

    if(isChronoStarted) return;

    REG_TIMER_CNT(0) = 0; //67MHz
    for(u32 i = 1; i < 4; i++) REG_TIMER_CNT(i) = 4; //Count-up

    for(u32 i = 0; i < 4; i++) REG_TIMER_VAL(i) = 0;

    REG_TIMER_CNT(0) = 0x80; //67MHz; enabled
    for(u32 i = 1; i < 4; i++) REG_TIMER_CNT(i) = 0x84; //Count-up; enabled

    isChronoStarted = true;
}

static u64 chrono(void)
{
    u64 res = 0;
    for(u32 i = 0; i < 4; i++) res |= REG_TIMER_VAL(i) << (16 * i);

    res /= (TICKS_PER_SEC / 1000);

    return res;
}

void NORETURN mcuPowerOff(void)
{
    //Ensure that all memory transfers have completed and that the data cache has been flushed
    flushEntireDCache();

    I2C_writeReg(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}

void wait(u64 amount)
{
    startChrono();

    u64 initialValue = chrono();

    while(chrono() - initialValue < amount);
}

// Max 500ms
static inline u8 mcuPeriodMsToTick(u32 periodMs)
{
    // 512MHz
    u32 res = 512u * periodMs / 1000u;
    res = res < 2 ? 1 : res - 1; // res not allowed to be zero
    res = res > 255 ? 255 : res; // res can't exceed 255
    return (u8)res;
}

void mcuSetInfoLedPattern(u8 r, u8 g, u8 b, u32 periodMs, bool smooth)
{
    McuInfoLedPattern pattern;

    if (periodMs == 0)
    {
        pattern.delay = 0xFF; // as high as we can; needs to be non-zero. Delay between frames (array elems)
        pattern.smoothing = 1;
        pattern.loopDelay = 0xFE; // as high as we can
        for (u32 i = 0; i < 32; i++)
        {
            pattern.red[i] = r;
            pattern.green[i] = g;
            pattern.blue[i] = b;
        }
    }
    else
    {
        periodMs = periodMs < 63 ? 63 : periodMs;
        pattern.delay = mcuPeriodMsToTick(periodMs);
        pattern.smoothing = smooth ? mcuPeriodMsToTick(periodMs) : 1;
        pattern.loopDelay = 0; // restart immediately
        for (u32 i = 0; i < 16; i++)
        {
            pattern.red[2*i] = r;
            pattern.red[2*i + 1] = 0;
            pattern.green[2*i] = g;
            pattern.green[2*i + 1] = 0;
            pattern.blue[2*i] = b;
            pattern.blue[2*i + 1] = 0;
        }
    }

    I2C_writeRegBuf(I2C_DEV_MCU, 0x2D, (const u8 *)&pattern, sizeof(McuInfoLedPattern));
}
