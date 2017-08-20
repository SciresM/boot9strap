/*
*   main.c
*/

#include "types.h"
#include "memory.h"
#include "crypto.h"
#include "i2c.h"
#include "fs.h"
#include "firm.h"
#include "utils.h"
#include "buttons.h"
#include "../build/bundled.h"

static void (*const itcmStub)(Firm *firm, bool isNand) = (void (*const)(Firm *, bool))0x01FF8000;
static volatile Arm11Operation *operation = (volatile Arm11Operation *)0x1FF80204;

static void invokeArm11Function(Arm11Operation op)
{
    while(*operation != ARM11_READY);
    *operation = op;
    while(*operation != ARM11_READY); 
}

static void loadFirm(bool isNand)
{
    static const char *firmName = "boot.firm";
    Firm *firmHeader = (Firm *)0x080A0000;
    if(fileRead(firmHeader, firmName, 0x200, 0) != 0x200) return;

    bool isPreLockout = ((firmHeader->reserved2[0] & 2) != 0),
         isScreenInit = ((firmHeader->reserved2[0] & 1) != 0);

    Firm *firm;
    u32 maxFirmSize;

    if(!isPreLockout)
    {
        //Lockout
        while(!(CFG9_SYSPROT9  & 1)) CFG9_SYSPROT9  |= 1;
        while(!(CFG9_SYSPROT11 & 1)) CFG9_SYSPROT11 |= 1;
        invokeArm11Function(WAIT_BOOTROM11_LOCKED);

        firm = (Firm *)0x20001000;
        maxFirmSize = 0x07FFF000; //around 127MB (although we don't enable ext FCRAM on N3DS, beware!)
    }
    else
    {
        //Uncached area, shouldn't affect performance too much, though
        firm = (Firm *)0x18000000;
        maxFirmSize = 0x300000; //3MB
    }

    u32 calculatedFirmSize = checkFirmHeader(firmHeader, (u32)firm, isPreLockout);

    if(!calculatedFirmSize) mcuPowerOff();

    if(fileRead(firm, firmName, 0, maxFirmSize) < calculatedFirmSize || !checkSectionHashes(firm)) mcuPowerOff();

    if(isScreenInit)
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
        /* I believe this is the canonical secret key combination. */
        if(HID_PAD == NTRBOOT_BUTTONS)
        {
            fileWrite((void *)0x08080000, "boot9strap/boot9.bin", 0x10000);
            fileWrite((void *)0x08090000, "boot9strap/boot11.bin", 0x10000);
            fileWrite((void *)0x10012000, "boot9strap/otp.bin", 0x100);

            /* Wait until buttons are not held, for compatibility. */
            while(HID_PAD & NTRBOOT_BUTTONS);
            wait(2000ULL);
        }

        loadFirm(false);
        unmountSd();
    }

    if(mountCtrNand())
    {
        /* Wait until buttons are not held, for compatibility. */
        if(HID_PAD == NTRBOOT_BUTTONS)
        {
            while(HID_PAD & NTRBOOT_BUTTONS);
            wait(2000ULL);
        }
        loadFirm(true);
    }

    mcuPowerOff();
}
