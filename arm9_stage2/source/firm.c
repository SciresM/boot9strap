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
#include "strings.h"
#include "cache.h"


static bool checkFirm(Firm *firm)
{
    //Very basic checks
    //No blacklist / whitelist
    if(memcmp(firm->magic, "FIRM", 4) != 0)
        return false;

    if(firm->arm9Entry == NULL || firm->arm11Entry == NULL)
        return false;

    bool arm9EpFound = false;
    for(u32 i = 0; i < 4; i++)
    {
        if(firm->section[i].address + firm->section[i].size < firm->section[i].address) //Overflow
            return false;

        if(((u32)firm->section[i].address & 3) || (firm->section[i].offset & 0x1FF) || (firm->section[i].size & 0x1FF))
            return false;

        if(firm->arm9Entry >= firm->section[i].address && firm->arm9Entry < (firm->section[i].address + firm->section[i].size))
            arm9EpFound = true;
    }

    return arm9EpFound; // allow for the arm11 entrypoint to be zero in which case nothing is done on the arm11 side
}

void launchFirm(Firm *firm, int argc, char **argv)
{
    if(!checkFirm(firm))
        return;

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
