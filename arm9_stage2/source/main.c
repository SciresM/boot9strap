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

#define MAX_FIRM_SIZE 0x04000000

static Firm *firm = (Firm *)0x24000000;

static void loadFirm(bool isNand)
{
    //No-screeninit payload
    if(fileRead(firm, "boot.firm", MAX_FIRM_SIZE) != 0)
    {
        const char *argv[1];
        if(isNand) argv[0] = "nand:/boot.firm";
        else argv[0] = "sdmc:/boot.firm";

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