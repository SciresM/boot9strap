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

#define MAX_FIRM_SIZE 0x40000000

static Firm *firm = (Firm *)0x24000000;

static void loadFirm(bool isNand)
{
    //No-screeninit payload
    if(fileRead(firm, "boot.firm", MAX_FIRM_SIZE) != 0)
    {
        const char *argv[1];
        if(isNand)
        {
            argv[0] = "1:/boot.firm";
            restoreShaHashBackup();
        }
        else argv[0] = "0:/boot.firm";
        
        launchFirm(firm, 1, (char **)argv);
    }
}

void main(void)
{
    if(mountSd())
    {
        loadFirm(false);
        unmountSd();
    }

    if(mountCtrNand()) loadFirm(true);

    //Shutdown
    flushEntireDCache();
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(true);
}