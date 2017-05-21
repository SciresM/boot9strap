/*
*   main.c
*/

#include "types.h"
#include "memory.h"
#include "crypto.h"
#include "i2c.h"
#include "cache.h"
#include "fs.h"
#include "firm.h"
#include "buttons.h"
#include "../build/bundled.h"

#define MAX_FIRM_SIZE         0x77000
#define MAX_FIRM_SIZE_FCRAM   0x07FFF000
#define A11_PAYLOAD_LOC 0x1FFFE000 //Keep in mind this needs to be changed in the ld script for arm11 too
#define A11_ENTRYPOINT  0x1FFFFFFC

static vu8 *arm11Flags = (vu8 *)0x1FFFFFF0;

static void doArm11Stuff(void) // b11 lockout sync and screen init
{
    memcpy((void *)A11_PAYLOAD_LOC, arm11_bin, arm11_bin_size);
    *(vu32 *)A11_ENTRYPOINT = A11_PAYLOAD_LOC;
    while(*(vu32 *)A11_ENTRYPOINT != 0);
    if(*arm11Flags & 1) i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A); //Turn on backlight
}

static void loadFirm(bool isNand)
{
    const char *argv[1];
    argv[0] = isNand ? "nand:/boot.firm" : "sdmc:/boot.firm";

    Firm *firmHeader = (Firm *)0x080A0000;
    if(fileRead(firmHeader, "boot.firm", 0x200) == 0 || !checkFirmHeader(firmHeader))
        return;
    
    *arm11Flags = firmHeader->reserved2[0];
    if(*arm11Flags & 2)
    {
        Firm *firm = (Firm *)0x08080000;
        doArm11Stuff();
        if(fileRead(firm, "boot.firm", MAX_FIRM_SIZE) == 0 || !checkSectionHashes(firm))
            return;
        launchFirm(firm, 1, (char **)argv);
    }
    else
    {
        Firm *firm = (Firm *)0x20001000;
        while(!(CFG9_SYSPROT9 & 1))
            CFG9_SYSPROT9 |= 1;

        while(!(CFG9_SYSPROT11 & 1))
            CFG9_SYSPROT11 |= 1;

        doArm11Stuff();

        if(fileRead(firm, "boot.firm", MAX_FIRM_SIZE_FCRAM) == 0 || !checkSectionHashes(firm))
            return;
        
        launchFirm(firm, 1, (char **)argv);
    }
}

void main(void)
{
    setupKeyslots();

    if(mountSd())
    {
        //I believe this is the canonical secret key combination
        if((HID_PAD & SECRET_BUTTONS) == SECRET_BUTTONS)
        {
            fileWrite((void *)0x08080000, "boot9strap/boot9.bin", 0x10000);
            fileWrite((void *)0x08090000, "boot9strap/boot11.bin", 0x10000);
            fileWrite((void *)0x10012000, "boot9strap/otp.bin", 0x100);
        }

        loadFirm(false);
        unmountSd();
    }

    if(mountCtrNand()) loadFirm(true);

    //Shutdown
    flushEntireDCache();
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}
