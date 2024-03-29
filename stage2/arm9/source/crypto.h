/*
*   This file is part of Luma3DS
*   Copyright (C) 2016 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b of GPLv3 applies to this file: Requiring preservation of specified
*   reasonable legal notices or author attributions in that material or in the Appropriate Legal
*   Notices displayed by works containing it.
*/

/*
*   Crypto libs from http://github.com/b1l1s/ctr
*   kernel9Loader code originally adapted from https://github.com/Reisyukaku/ReiNand/blob/228c378255ba693133dec6f3368e14d386f2cde7/source/crypto.c#L233
*/

#pragma once

#include "types.h"

/**************************AES****************************/
#define REG_AESCNT          ((vu32 *)0x10009000)
#define REG_AESMACBLKCNT    ((vu16 *)0x10009004)
#define REG_AESBLKCNT       ((vu16 *)0x10009006)
#define REG_AESWRFIFO       ((vu32 *)0x10009008)
#define REG_AESRDFIFO       ((vu32 *)0x1000900C)
#define REG_AESKEYSEL       ((vu8 *)0x10009010)
#define REG_AESKEYCNT       ((vu8 *)0x10009011)
#define REG_AESCTR          ((vu32 *)0x10009020)

#define REG_AESKEYFIFO      ((vu32 *)0x10009100)
#define REG_AESKEYXFIFO     ((vu32 *)0x10009104)
#define REG_AESKEYYFIFO     ((vu32 *)0x10009108)

#define REGs_AESTWLKEYS     (*((vu32 (*)[4][3][4])0x10009040))

#define AES_CCM_DECRYPT_MODE    (0u << 27)
#define AES_CCM_ENCRYPT_MODE    (1u << 27)
#define AES_CTR_MODE            (2u << 27)
#define AES_CTR_MODE            (2u << 27)
#define AES_CBC_DECRYPT_MODE    (4u << 27)
#define AES_CBC_ENCRYPT_MODE    (5u << 27)
#define AES_ECB_DECRYPT_MODE    (6u << 27)
#define AES_ECB_ENCRYPT_MODE    (7u << 27)
#define AES_ALL_MODES           (7u << 27)

#define AES_CNT_START           BIT(31)
#define AES_CNT_IRQ_ENABLE      BIT(30)
#define AES_CNT_INPUT_ORDER     BIT(25)
#define AES_CNT_OUTPUT_ORDER    BIT(24)
#define AES_CNT_INPUT_ENDIAN    BIT(23)
#define AES_CNT_OUTPUT_ENDIAN   BIT(22)
#define AES_CNT_RDFIFO_SIZE(n)  (((n)/16 - 1) << 14)
#define AES_CNT_WRFIFO_SIZE(n)  ((4 - (n)/16) << 12)
#define AES_CNT_FLUSH_READ      BIT(11)
#define AES_CNT_FLUSH_WRITE     BIT(10)

#define AES_INPUT_BE            (AES_CNT_INPUT_ENDIAN)
#define AES_INPUT_LE            0
#define AES_INPUT_NORMAL        (AES_CNT_INPUT_ORDER)
#define AES_INPUT_REVERSED      0
#define AES_INPUT_TWLNORMAL     0
#define AES_INPUT_TWLREVERSED   (AES_CNT_INPUT_ORDER)

#define AES_BLOCK_SIZE      0x10

#define AES_KEYCNT_WRITE    (1 << 0x7)
#define AES_KEYNORMAL       0
#define AES_KEYX            1
#define AES_KEYY            2

/**************************SHA****************************/
#define REG_SHA_CNT         ((vu32 *)0x1000A000)
#define REG_SHA_BLKCNT      ((vu32 *)0x1000A004)
#define REG_SHA_HASH        ((vu32 *)0x1000A040)
#define REG_SHA_INFIFO      ((vu32 *)0x1000A080)

#define SHA_CNT_STATE           (BIT(1) | BIT(0))
#define SHA_CNT_IN_DMA_ENABLE   BIT(2)
#define SHA_CNT_OUTPUT_ENDIAN   BIT(3)
#define SHA_CNT_MODE            (3u << 4)

#define SHA_HASH_READY      0x00000000
#define SHA_NORMAL_ROUND    0x00000001
#define SHA_FINAL_ROUND     0x00000002

#define SHA_OUTPUT_BE       SHA_CNT_OUTPUT_ENDIAN
#define SHA_OUTPUT_LE       0

#define SHA_256_MODE        0
#define SHA_224_MODE        0x00000010
#define SHA_1_MODE          0x00000020

#define SHA_256_HASH_SIZE   (256 / 8)
#define SHA_224_HASH_SIZE   (224 / 8)
#define SHA_1_HASH_SIZE     (160 / 8)

#define CFG_SYSPROT9        (*(vu8  *)0x10000000)
#define CFG_BOOTENV         (*(vu32 *)0x10010000)
#define CFG_UNITINFO        (*(vu8  *)0x10010010)
#define CFG_TWLUNITINFO     (*(vu8  *)0x10010014)
#define OTP_DEVCONSOLEID    (*(vu64 *)0x10012000)
#define OTP_TWLCONSOLEID    (*(vu64 *)0x10012100)
#define CFG11_SOCINFO       (*(vu32 *)0x10140FFC)

#define ISN3DS    (CFG11_SOCINFO & 2)
#define ISDEVUNIT (CFG_UNITINFO != 0)

void sha(void *res, const void *src, u32 size, u32 mode);
void sha_dma(void *res, const void *src, u32 size, u32 mode);

int ctrNandInit(void);
int ctrNandRead(u32 sector, u32 sectorCount, u8 *outbuf);
void setupKeyslots(void);
