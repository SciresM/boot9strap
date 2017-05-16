/*
*   memcpy adapted from https://github.com/mid-kid/CakesForeveryWan/blob/557a8e8605ab3ee173af6497486e8f22c261d0e2/source/memfuncs.c
*/

#pragma once

#include "types.h"

void memcpy(void *dest, const void *src, u32 size);
int memcmp(const void *buf1, const void *buf2, u32 size);
void memset32(void *dest, u32 filler, u32 size);
