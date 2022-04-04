/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "../crypto.h"
#include "sdmmc/unprotboot9_sdmmc.h"
#include "../ndma.h"
#include "../cache.h"

/* Definitions of physical drive number for each media */
#define SDCARD  0
#define CTRNAND 1


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	static bool sdInitialized = false, nandInitialized = false;
	int res = 0;

	switch (pdrv)
	{
		case SDCARD:
			if (!sdInitialized)
				res = unprotboot9_sdmmc_initdevice(unprotboot9_sdmmc_deviceid_sd);
			sdInitialized = res == 0;
			break;
		case CTRNAND:
			if (!nandInitialized)
				res = ctrNandInit();
			nandInitialized = res == 0;
			break;
	}

	return res == 0 ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static s32 bromSdmmcReadWithDmaPrepareCb(u32 fifoAddr)
{
	volatile NdmaChannelRegisters *const chan0 = &REG_NDMA->channel[0];
	chan0->src_addr = fifoAddr;
	chan0->block_words = 512 / 4;
	chan0->cnt = NDMA_ENABLE | NDMA_NORMAL_MODE | NDMA_STARTUP_SDIO1 | NDMA_BURST_WORDS(512/4) |
			     NDMA_DST_UPDATE_MODE(NDMA_UPDATE_INC) | NDMA_SRC_UPDATE_MODE(NDMA_UPDATE_FIXED);
    return 0;
}

static void bromSdmmcReadWithDmaAbortCb(void)
{
	volatile NdmaChannelRegisters *const chan0 = &REG_NDMA->channel[0];
	chan0->cnt &= ~NDMA_ENABLE;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	int res = 0;
	switch (pdrv)
	{
		case SDCARD:
			res = unprotboot9_sdmmc_selectdevice(unprotboot9_sdmmc_deviceid_sd);
			if (res == 0)
			{
				volatile NdmaChannelRegisters *const chan0 = &REG_NDMA->channel[0];
				flushDCacheRange(buff, 512 * count);
				chan0->cnt &= ~NDMA_ENABLE;
				chan0->total_words = 512 * count / 4;
				chan0->dst_addr = (u32)buff;
				res = unprotboot9_sdmmc_readrawsectors_setup((u32)sector, count, NULL, bromSdmmcReadWithDmaPrepareCb, bromSdmmcReadWithDmaAbortCb);
				while (chan0->cnt & NDMA_ENABLE);
				flushDCacheRange(buff, 512 * count);
			}
			break;
		case CTRNAND:
			res = ctrNandRead((u32)sector, count, buff);
			break;
		default:
			res = -1;
			break;
	}
	return res == 0 ? RES_OK : RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;
	(void)buff;
	(void)sector;
	(void)count;
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)pdrv;
	(void)cmd;
	(void)buff;
    return cmd == CTRL_SYNC ? RES_OK : RES_PARERR;
}
