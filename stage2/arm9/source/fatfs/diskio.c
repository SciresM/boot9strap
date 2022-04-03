/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "sdmmc/sdmmc.h"
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
	__attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
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
	DWORD sector,	/* Sector address in LBA */
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
				res = unprotboot9_sdmmc_readrawsectors_setup(sector, count, NULL, bromSdmmcReadWithDmaPrepareCb, bromSdmmcReadWithDmaAbortCb);
				while (chan0->cnt & NDMA_ENABLE);
				flushDCacheRange(buff, 512 * count);
			}
			break;
		case CTRNAND:
			res = ctrNandRead(sector, count, buff);
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

#if _USE_WRITE
DRESULT disk_write (
	__attribute__((unused))
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	__attribute__((unused))
	const BYTE *buff,       	/* Data to be written */
	__attribute__((unused))
	DWORD sector,		/* Sector address in LBA */
	__attribute__((unused))
	UINT count			/* Number of sectors to write */
)
{
    return RES_OK;//(pdrv == SDCARD && (*(vu16 *)(SDMMC_BASE + REG_SDSTATUS0) & TMIO_STAT0_WRPROTECT) != 0 && !sdmmc_sdcard_writesectors(sector, count, buff)) ? RES_OK : RES_PARERR;
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	__attribute__((unused))
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	__attribute__((unused))
	BYTE cmd,		/* Control code */
	__attribute__((unused))
	void *buff		/* Buffer to send/receive control data */
)
{
    return cmd == CTRL_SYNC ? RES_OK : RES_PARERR;
}
#endif
