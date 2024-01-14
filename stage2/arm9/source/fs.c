/*
*   fs.c
*/

#include <string.h>
#include "fs.h"
#include "fatfs/ff.h"

static FATFS fs;
static bool sdMounted = false, nandMounted = false;

bool mountSd(void)
{
    if (!sdMounted)
        sdMounted = f_mount(&fs, "0:", 1) == FR_OK;
    return sdMounted;
}

void unmountSd(void)
{
    if (sdMounted)
        sdMounted = f_mount(NULL, "0:", 1) != FR_OK;
}

bool mountCtrNand(void)
{
    if (!nandMounted)
        nandMounted = f_mount(&fs, "1:", 1) == FR_OK && f_chdrive("1:") == FR_OK;
    return nandMounted;
}

void unmountCtrNand(void)
{
    if (nandMounted)
        nandMounted = f_mount(NULL, "1:", 1) != FR_OK;
}

u32 fileRead(void *dest, const char *path, u32 size, u32 maxSize)
{
    FIL file;
    FRESULT result = FR_OK;
    u32 ret = 0;

    if(f_open(&file, path, FA_READ) != FR_OK) return ret;

    if(!size) size = f_size(&file);
    if(!maxSize || size <= maxSize)
        result = f_read(&file, dest, size, (unsigned int *)&ret);
    result |= f_close(&file);

    return result == FR_OK ? ret : 0;
}

bool fileWrite(const void *buffer, const char *path, u32 size)
{
    FIL file;
    FRESULT result = FR_OK;

    switch(f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS))
    {
        case FR_OK:
        {
            unsigned int written;
            result = f_write(&file, buffer, size, &written);
            if(result == FR_OK) result = f_truncate(&file);
            result |= f_close(&file);

            return result == FR_OK && (u32)written == size;
        }
        case FR_NO_PATH:
            for(u32 i = 1; path[i] != 0; i++)
                if(path[i] == '/')
                {
                    char folder[i + 1];
                    memcpy(folder, path, i);
                    folder[i] = 0;
                    result = f_mkdir(folder);
                }

            return result == FR_OK && fileWrite(buffer, path, size);
        default:
            return false;
    }
}

bool fileExists(const char* path)
{
    FIL file;
    if(f_open(&file, path, FA_READ) != FR_OK) return false;   
    f_close(&file);
    return true;
}

u32 fileReadLine(char *dest, const char *path, u32 lineNum, u32 maxSize)
{
    FIL fil;
    FRESULT fr;
    UINT br;
    u32 totalRead = 0;
    u32 currentLine = 0;
    bool lineContentStarted = false;
    bool endOfFile = false;

    fr = f_open(&fil, path, FA_READ);
    if (fr != FR_OK)
        return 0;

    while (!endOfFile)
    {
        char c;
        fr = f_read(&fil, &c, 1, &br);
        
        if (fr != FR_OK || br == 0)
        {
            endOfFile = true;
            if (currentLine != lineNum || !lineContentStarted) 
                break;
        }

        if (c == '\n' || c == '\r')
        {
            if (currentLine == lineNum)
            {
                if (!lineContentStarted) 
                    totalRead = 1;
                break;
            }
            currentLine++;
            lineContentStarted = false;
        }
        else if (currentLine == lineNum && totalRead < maxSize - 1)
        {
            dest[totalRead++] = c;
            lineContentStarted = true;
        }
    }

    dest[totalRead] = '\0';
    f_close(&fil);

    return totalRead;
}
