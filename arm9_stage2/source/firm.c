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
#include "cache.h"
#include "crypto.h"

extern u32 __start__, __end__, __stack_top__, __stack_bottom__;

static __attribute((noinline)) bool overlaps(u32 as, u32 ae, u32 bs, u32 be)
{
    if (as <= bs && bs <= ae)
        return true;
    else if (bs <= as && as <= be)
        return true;
    return false;
}

bool checkFirmHeader(Firm *firm)
{
    //Very basic checks

    if(memcmp(firm->magic, "FIRM", 4) != 0)
        return false;

    if(firm->arm9Entry == NULL) //Allow for the arm11 entrypoint to be zero in which case nothing is done on the arm11 side
        return false;

    u32 size = 0x200;
    for(u32 i = 0; i < 4; i++)
        size += firm->section[i].size;

    bool arm9EpFound = false,
         arm11EpFound = false;

    for(u32 i = 0; i < 4; i++)
    {
        FirmSection *section = &firm->section[i];

        //Allow empty sections
        if(section->size == 0)
            continue;

        if(section->offset < 0x200)
            return false;

        if(section->address + section->size < section->address) //overflow check
            return false;

        if(((u32)section->address & 3) || (section->offset & 0x1FF) || (section->size & 0x1FF)) //alignment check
            return false;

        if(overlaps((u32)section->address, (u32)section->address + section->size, (u32)&__start__, (u32)&__end__))
            return false;
        else if(overlaps((u32)section->address, (u32)section->address + section->size, (u32)&__stack_bottom__, (u32)&__stack_top__))
            return false;
        else if(overlaps((u32)section->address, (u32)section->address + section->size, (u32)firm + section->offset, (u32)firm + size))
            return false;
        else if((firm->reserved2[0] & 2) && overlaps((u32)section->address, (u32)section->address + section->size, 0x20000000, 0x30000000))
            return false;

        if(firm->arm9Entry >= section->address && firm->arm9Entry < (section->address + section->size))
            arm9EpFound = true;

        if(firm->arm11Entry >= section->address && firm->arm11Entry < (section->address + section->size))
            arm11EpFound = true;
    }

    return arm9EpFound && (firm->arm11Entry == NULL || arm11EpFound);
}

bool checkSectionHashes(Firm *firm)
{
    for(u32 i = 0; i < 4; i++)
    {
        __attribute__((aligned(4))) u8 hash[0x20];
        FirmSection *section = &firm->section[i];
        
        if(section->size == 0)
            continue;
        
        sha(hash, (u8 *)firm + section->offset, section->size, SHA_256_MODE);
        if(memcmp(hash, section->hash, 0x20) != 0)
            return false;
    }

    return true;
}

void launchFirm(Firm *firm, int argc, char **argv)
{
    //Copy FIRM sections to respective memory locations
    for(u32 sectionNum = 0; sectionNum < 4 && firm->section[sectionNum].size != 0; sectionNum++)
        memcpy(firm->section[sectionNum].address, (u8 *)firm + firm->section[sectionNum].offset, firm->section[sectionNum].size);

    //Set ARM11 entrypoint
    *(vu32 *)0x1FFFFFFC = (u32)firm->arm11Entry;

    //Ensure that all memory transfers have completed and that the caches have been flushed
    flushEntireDCache();
    flushEntireICache();

    //Jump to ARM9 entrypoint. Also give it additional arguments it can dismiss
    ((void (*)(int, char**, u32))firm->arm9Entry)(argc, argv, 0x0000BEEF);

    __builtin_unreachable();
}
