/*
*   main.c
*/

#include <string.h>

#include "types.h"
#include "crypto.h"
#include "i2c.h"
#include "fs.h"
#include "firm.h"
#include "utils.h"
#include "buttons.h"
#include "fatfs/sdmmc/unprotboot9_sdmmc.h"
#include "ndma.h"
#include "cache.h"

#include "chainloader.h"

typedef enum FirmLoadStatus {
    FIRM_LOAD_OK = 0,
    FIRM_LOAD_CANT_READ, // can't mount, file missing or empty
    FIRM_LOAD_CORRUPT,
} FirmLoadStatus;

static volatile Arm11Operation *operation = (volatile Arm11Operation *)0x1FF80204;
extern u8 __itcm_start__[], __itcm_lma__[], __itcm_bss_start__[], __itcm_end__[];

static void invokeArm11Function(Arm11Operation op)
{
    while(*operation != ARM11_READY);
    *operation = op;
    while(*operation != ARM11_READY); 
}

static FirmLoadStatus loadFirm(Firm **outFirm)
{
    static const char *firmName = "boot.firm";
    Firm *firmHeader = (Firm *)0x080A0000;
    u32 rd = fileRead(firmHeader, firmName, 0x200, 0);
    if (rd != 0x200)
        return rd == 0 ? FIRM_LOAD_CANT_READ : FIRM_LOAD_CORRUPT;

    bool isPreLockout = ((firmHeader->reserved2[0] & 2) != 0);
    if ((CFG9_SYSPROT9 & 1) != 0 || (CFG9_SYSPROT11 & 1) != 0)
        isPreLockout = false;
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

    *outFirm = firm;

    u32 calculatedFirmSize = checkFirmHeader(firmHeader, (u32)firm, isPreLockout);

    if(!calculatedFirmSize || fileRead(firm, firmName, 0, maxFirmSize) < calculatedFirmSize || !checkSectionHashes(firm))
        return FIRM_LOAD_CORRUPT;
    else
        return FIRM_LOAD_OK;
}

static void bootFirm(Firm *firm, bool isNand)
{
    bool isScreenInit = (firm->reserved2[0] & 1) != 0;
    if(isScreenInit)
    {
        invokeArm11Function(INIT_SCREENS);
        I2C_writeReg(I2C_DEV_MCU, 0x22, 0x2A); //Turn on backlight
    }

    memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
    memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);

    //Launch firm
    invokeArm11Function(PREPARE_ARM11_FOR_FIRMLAUNCH);
    __dsb();

    flushEntireDCache();
    chainload(firm, isNand);
    __builtin_unreachable();
}

// Set the info LED and wait if the FIRM file is corrupt, or if the NTRBOOT combo is pressed
static void displayStatus(FirmLoadStatus sdStatus, FirmLoadStatus nandStatus)
{
    static const u8 statusColors[][3] = {
        {   0, 255,   0 }, // Green (SD load OK)
        { 255, 255,   0 }, // Yellow (SD FIRM not found/empty & NAND load OK)
        { 255, 100,   0 }, // Orange (SD FIRM corrupt & NAND load OK)
        { 255, 255, 255 }, // White (SD FIRM missing & NAND FIRM not found/empty)
        { 255,   0, 255 }, // Magenta (SD FIRM missing & NAND FIRM corrupt)
        { 255,   0,   0 }, // Red (SD FIRM corrupt & NAND FIRM corrupt)
    };

    const u8 *rgb;

    if (sdStatus == FIRM_LOAD_OK)
        rgb = statusColors[0];
    else if (nandStatus == FIRM_LOAD_OK)
        rgb = statusColors[(u32)sdStatus];
    else if (sdStatus == FIRM_LOAD_CANT_READ)
        rgb = statusColors[2 + (u32)nandStatus];
    else
        rgb = statusColors[5];

    // If the NTRBOOT combo is held, if the SD FIRM is corrupt or if we can't boot, display the status
    bool ntrbootComboPressed = HID_PAD == NTRBOOT_BUTTONS;
    if (ntrbootComboPressed || sdStatus == FIRM_LOAD_CORRUPT || (sdStatus != FIRM_LOAD_OK && nandStatus != FIRM_LOAD_OK))
    {
        const vu8 *bootMediaStatus = (const vu8 *)0x1FFFE00C;
        const vu32 *bootPartitionsStatus = (const vu32 *)0x1FFFE010;

        // Shell closed, no error booting NTRCARD, NAND paritions not even considered
        bool isNtrBoot = bootMediaStatus[3] == 2 && !bootMediaStatus[1] && !bootPartitionsStatus[0] && !bootPartitionsStatus[1];

        // Blink if NTRBOOT, otherwise not
        mcuSetInfoLedPattern(rgb[0], rgb[1], rgb[2], isNtrBoot ? 500 : 0, false);
        while(HID_PAD & NTRBOOT_BUTTONS);
        // 0 -> 500ms: color
        // 500 -> 1000: black
        // 1000 -> 1500: color
        // 1500 -> 2000: black
        // ie. it will blink wait_time/(2 * period)
        wait(2000);
        mcuSetInfoLedPattern(0, 0, 0, 0, false);
        // The wait is needed here, in case we power off
        // (at the very least 2ms, so the MCU notices the pattern change)
        wait(50);
    }
}

void arm9Main(void)
{
    FirmLoadStatus sdStatus, nandStatus;
    Firm *firm = NULL;

    setupKeyslots();
    ndmaInit();
    unprotboot9_sdmmc_initialize();

    sdStatus = mountSd() ? loadFirm(&firm) : FIRM_LOAD_CANT_READ;
    if (sdStatus != FIRM_LOAD_OK)
        nandStatus = mountCtrNand() ? loadFirm(&firm) : FIRM_LOAD_CANT_READ;
    else
        nandStatus = FIRM_LOAD_CANT_READ;

    unmountSd();
    unmountCtrNand();
    displayStatus(sdStatus, nandStatus);

    // Can we boot?
    if (sdStatus == FIRM_LOAD_OK || nandStatus == FIRM_LOAD_OK)
        bootFirm(firm, sdStatus != FIRM_LOAD_OK);
    else
        mcuPowerOff();

    __builtin_unreachable();
}
