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

#include "firm.h"
#include "memory.h"
#include "crypto.h"

static __attribute__((noinline)) bool overlaps(u32 as, u32 ae, u32 bs, u32 be)
{
    if(as <= bs && bs <= ae)
        return true;
    if(bs <= as && as <= be)
        return true;
    return false;
}

static __attribute__((noinline)) bool inRange(u32 as, u32 ae, u32 bs, u32 be)
{
   if(as >= bs && ae <= be)
        return true;
   return false;
}

u32 checkFirmHeader(Firm *firmHeader, u32 firmBufferAddr, bool isPreLockout)
{
    if(memcmp(firmHeader->magic, "FIRM", 4) != 0 || firmHeader->arm9Entry == NULL) //Allow for the ARM11 entrypoint to be zero in which case nothing is done on the ARM11 side
        return 0;

    bool arm9EpFound = false,
         arm11EpFound = false;

    u32 size = 0x200;
    for(u32 i = 0; i < 4; i++)		
        size += firmHeader->section[i].size;

    for(u32 i = 0; i < 4; i++)
    {
        FirmSection *section = &firmHeader->section[i];

        //Allow empty sections
        if(section->size == 0)
            continue;

        if((section->offset < 0x200) ||
           (section->address + section->size < section->address) || //Overflow check
           ((u32)section->address & 3) || (section->offset & 0x1FF) || (section->size & 0x1FF) || //Alignment check
           (overlaps((u32)section->address, (u32)section->address + section->size, firmBufferAddr, firmBufferAddr + size)) ||
           ((!inRange((u32)section->address, (u32)section->address + section->size, 0x08000000, 0x08000000 + 0x00100000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x18000000, 0x18000000 + 0x00600000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x1FF00000, 0x1FFFFC00)) &&
            (!(!isPreLockout && inRange((u32)section->address, (u32)section->address + section->size, 0x20000000, 0x20000000 + 0x8000000)))))
            return 0;

        if(firmHeader->arm9Entry >= section->address && firmHeader->arm9Entry < (section->address + section->size))
            arm9EpFound = true;

        if(firmHeader->arm11Entry >= section->address && firmHeader->arm11Entry < (section->address + section->size))
            arm11EpFound = true;
    }

    return (arm9EpFound && (firmHeader->arm11Entry == NULL || arm11EpFound)) ? size : 0;
}

bool checkSectionHashes(Firm *firm)
{
    for(u32 i = 0; i < 4; i++)
    {
        FirmSection *section = &firm->section[i];

        if(section->size == 0)
            continue;

        __attribute__((aligned(4))) u8 hash[0x20];

        sha(hash, (u8 *)firm + section->offset, section->size, SHA_256_MODE);

        if(memcmp(hash, section->hash, 0x20) != 0)
            return false;
    }

    return true;
}
