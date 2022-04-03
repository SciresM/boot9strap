#pragma once

#include "../../types.h"

//These return 0 for success, non-zero for error.

//Using this requires that the DTCM at 0xfff00000 is accessible. You can either disable MPU, or setup a MPU region for it.
//Additionally, reading the NAND requires ITCM to be accessible at 0x07ff8000.

typedef enum {
	unprotboot9_sdmmc_deviceid_sd = 0x200,
	unprotboot9_sdmmc_deviceid_nand = 0x201
} unprotboot9_sdmmc_deviceid;

typedef s32 (*unprotboot9_sdmmc_dma_prepare_cb)(u32 fifoAddr);
typedef void (*unprotboot9_sdmmc_dma_abort_cb)(void);
typedef s32 (*unprotboot9_sdmmc_sector_cb)(u32 fifoAddr);

s32 unprotboot9_sdmmc_initialize(void);//This must be used first.
s32 unprotboot9_sdmmc_initdevice(unprotboot9_sdmmc_deviceid deviceid);//This must be used before doing anything with the device.
s32 unprotboot9_sdmmc_selectdevice(unprotboot9_sdmmc_deviceid deviceid);//Only use this if the device was already initialized, and if the device isn't already selected(the latter is checked for by the boot9 code anyway).

//Sets up a raw sector read from the currently selected and previously initialized device (in a way you can use DMA).
//If the callback is set, the read is interrupted every sector and you are supposed to empty the FIFO in less than 650ms
s32 unprotboot9_sdmmc_readrawsectors_setup(u32 sector, u32 numsectors, unprotboot9_sdmmc_sector_cb cb, unprotboot9_sdmmc_dma_prepare_cb dmaprep_cb, unprotboot9_sdmmc_dma_abort_cb dmaabt_cb);

//Read raw sectors using the CPU
s32 unprotboot9_sdmmc_readrawsectors(u32 sector, u32 numsectors, void *buf);
