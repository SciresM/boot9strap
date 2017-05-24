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

#define A11_PAYLOAD_LOC 0x1FFFE000 //Keep in mind this needs to be changed in the ld script for arm11 too
#define A11_ENTRYPOINT  0x1FFFFFFC

static void (*const itcmStub)(Firm *firm, bool isNand) = (void (*const)(Firm *, bool))0x01FF8000;
static volatile Arm11Operation *operation = (volatile Arm11Operation *)0x1FF80204;

static void shutdown(void)
{
    flushEntireDCache();
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}

static void invokeArm11Function(Arm11Operation op)
{
    while(*operation != ARM11_READY);
    *operation = op;
    while(*operation != ARM11_READY); 
}

static void loadFirm(bool isNand)
{
    Firm *firmHeader = (Firm *)0x080A0000;
    if(fileRead(firmHeader, "boot.firm", 0x200, 0) != 0x200) return;
    if(!checkFirmHeader(firmHeader)) shutdown();

    Firm *firm;
    u32 maxFirmSize;

    if(!(firmHeader->reserved2[0] & 2))
    {
        //Lockout
        while(!(CFG9_SYSPROT9  & 1)) CFG9_SYSPROT9  |= 1;
        while(!(CFG9_SYSPROT11 & 1)) CFG9_SYSPROT11 |= 1;
        invokeArm11Function(WAIT_BOOTROM11_LOCKED);

        firm = (Firm *)0x20001000;
        maxFirmSize = 0x07FFF000;
    }
    else
    {
        firm = (Firm *)0x08080000;
        maxFirmSize = 0x77000;
    }

    if(fileRead(firm, "boot.firm", 0, maxFirmSize) <= 0x200 || !checkSectionHashes(firm)) shutdown();
    if(firm->reserved2[0] & 1)
    {
        invokeArm11Function(INIT_SCREENS);
        i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A); //Turn on backlight
    }

    memcpy((void *)itcmStub, itcm_stub_bin, itcm_stub_bin_size);

    //Launch firm
    invokeArm11Function(PREPARE_ARM11_FOR_FIRMLAUNCH);
    itcmStub(firm, isNand);
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

            shutdown();
        }

        loadFirm(false);
        unmountSd();
    }

    if(mountCtrNand()) loadFirm(true);

    shutdown();
}
