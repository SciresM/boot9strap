/*
*   This file is part of Luma3DS
*   Copyright (C) 2022 TuxSH
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

// chainloader_entry.s
void disableMpuAndJumpToEntrypoints(int argc, char **argv, void *arm11Entry, void *arm9Entry);
void copyFirmSection(void *dst, const void *src, size_t size);

static void launchFirm(const Firm *firm, int argc, char **argv)
{
    //Copy FIRM sections to respective memory locations
    //I have benchmarked this and NDMA is actually a little bit slower.
    for(u32 sectionNum = 0; sectionNum < 4; sectionNum++)
    {
        const FirmSection *const section = &firm->section[sectionNum];
        copyFirmSection(section->address, (u8 *)firm + section->offset, section->size);
    }

    disableMpuAndJumpToEntrypoints(argc, argv, firm->arm9Entry, firm->arm11Entry);

    __builtin_unreachable();
}

void chainloaderMain(const Firm *firm, bool isNand)
{
    u32 argc;
    char *argv[2] = {0};
    struct fb fbs[2] =
    {
        {
            .top_left  = 0x18300000,
            .top_right = 0x18300000,
            .bottom    = 0x18346500,
        },
        {
            .top_left  = 0x18400000,
            .top_right = 0x18400000,
            .bottom    = 0x18446500,
        },
    };

    argv[0] = isNand ? "nand:/boot.firm" : "sdmc:/boot.firm";

    if(firm->reserved2[0] & 1)
    {
        argc = 2;
        argv[1] = (char *)&fbs;
    }
    else argc = 1;

    launchFirm(firm, argc, argv);
}
