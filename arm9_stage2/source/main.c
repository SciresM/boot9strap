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

#define MAX_FIRM_SIZE   0x04000000
#define A11_PAYLOAD_LOC 0x1FFFE000 //Keep in mind this needs to be changed in the ld script for arm11 too
#define A11_ENTRYPOINT  0x1FFFFFFC

static Firm *firm = (Firm *)0x24000000;

static void initScreens(void)
{
    memcpy((void *)A11_PAYLOAD_LOC, arm11_bin, arm11_bin_size);
    *(vu32 *)A11_ENTRYPOINT = A11_PAYLOAD_LOC;
    while(*(vu32 *)A11_ENTRYPOINT != 0);
    i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A); //Turn on backlight
}

static void loadFirmOnce(void)
{
    if(fileRead(firm, "bootonce.firm", MAX_FIRM_SIZE) != 0)
    {
        if(checkFirm(firm))
        {
            const char *argv[1];
            argv[0] = "sdmc:/bootonce.firm";
            fileDelete("bootonce.firm");
            
            if(firm->reserved2[0] & 1) initScreens();
            launchFirm(firm, 1, (char **)argv);
        }
    }
}

static void loadFirm(bool isNand)
{
    if(fileRead(firm, "boot.firm", MAX_FIRM_SIZE) != 0)
    {
        if(checkFirm(firm))
        {
            const char *argv[1];
            argv[0] = isNand ? "nand:/boot.firm" : "sdmc:/boot.firm";

            if(firm->reserved2[0] & 1) initScreens();
            launchFirm(firm, 1, (char **)argv);
        }
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

        loadFirmOnce();
        loadFirm(false);
        unmountSd();
    }

    if(mountCtrNand()) loadFirm(true);

    //Shutdown
    flushEntireDCache();
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}
