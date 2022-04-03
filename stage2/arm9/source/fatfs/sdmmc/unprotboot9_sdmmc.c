#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "unprotboot9_sdmmc.h"

s32 unprotboot9_sdmmc_initialize()
{
	void (*funcptr_cleardtcm)() = (void*)0xffff01b0;
	void (*funcptr_boot9init)() = (void*)0xffff1ff9;
	s32 (*funcptr_mmcinit)() = (void*)0xffff56c9;

	*((u16*)0x10000020) |= 0x200;//If not set, the hardware will not detect any inserted card on the sdbus.
	*((u16*)0x10000020) &= ~0x1;//If set while bitmask 0x200 is set, a sdbus command timeout error will occur during sdbus init.

	funcptr_cleardtcm();
	*((u32*)(0xfff0009c+0x1c)) = 1;//Initialize the sdmmc busid.

	funcptr_boot9init();//General boot9 init function.

	return funcptr_mmcinit();
}

s32 unprotboot9_sdmmc_initdevice(unprotboot9_sdmmc_deviceid deviceid)
{
	s32 (*funcptr)(u32) = (void*)0xffff5775;
	return funcptr(deviceid);
}

s32 unprotboot9_sdmmc_selectdevice(unprotboot9_sdmmc_deviceid deviceid)
{
	s32 (*funcptr)(u32) = (void*)0xffff5c11;
	return funcptr(deviceid);
}

s32 unprotboot9_sdmmc_readrawsectors_setup(u32 sector, u32 numsectors, unprotboot9_sdmmc_sector_cb cb, unprotboot9_sdmmc_dma_prepare_cb dmaprep_cb, unprotboot9_sdmmc_dma_abort_cb dmaabt_cb)
{
	u32 original_deviceid;
	s32 ret;
	s32 (*funcptr)(u32, u32) = (void*)0xffff2e39;

	original_deviceid = *(u32*)0xfff000bc;

	//The boot9 sector reading code calls the deviceselect code with deviceid=nand. Overwrite the current_deviceid value in memory with the nand value, so that the deviceselect function does nothing there.
	*(u32*)0xfff000bc = (u32)unprotboot9_sdmmc_deviceid_nand;

	//Prepare the arguments
	*(u32*)0xfff000a8 = (u32)dmaprep_cb;
	*(u32*)0xfff000ac = (u32)cb;
	*(u32*)0xfff000b0 = (u32)dmaabt_cb;
	//*0xfff000bc is 0 for Port1, 1 for Port3. BootROM always uses Port1.

	ret = funcptr(sector, numsectors);

	*(u32*)0xfff000bc = original_deviceid;
	return ret;
}

s32 unprotboot9_sdmmc_readrawsectors(u32 sector, u32 numsectors, void *buf)
{
	//This mimics the function @ 0xffff55f8
	*(u32*)0xfff00198 = (u32)buf;
	return unprotboot9_sdmmc_readrawsectors_setup(sector, numsectors, (unprotboot9_sdmmc_sector_cb)0xffff3bd5, NULL, NULL);
}
