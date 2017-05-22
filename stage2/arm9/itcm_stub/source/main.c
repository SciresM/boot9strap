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

void main(Firm *firm, bool isNand)
{
    const char *argv[2];
    argv[0] = isNand ? "nand:/boot.firm" : "sdmc:/boot.firm";

    if(firm->reserved2[0] & 1)
    {
        struct fb fbs[2] = {
            {
                .top_left  = (u8 *)0x18300000,
                .top_right = (u8 *)0x18300000,
                .bottom    = (u8 *)0x18346500,
            },
            {
                .top_left  = (u8 *)0x18400000,
                .top_right = (u8 *)0x18400000,
                .bottom    = (u8 *)0x18446500,
            },
        };

        argv[1] = (char *)&fbs;
        launchFirm(firm, 2, (char **)argv);
    }
    else
        launchFirm(firm, 1, (char **)argv);
}
