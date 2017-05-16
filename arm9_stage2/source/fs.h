/*
*   fs.h
*/

#pragma once

#include "types.h"

bool mountSd(void);
void unmountSd(void);
bool mountCtrNand(void);
u32 fileRead(void *dest, const char *path, u32 maxSize);