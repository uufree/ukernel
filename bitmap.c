/*************************************************************************
	> File Name: bitmap.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时47分51秒
 ************************************************************************/

#include"debug.h"
#include"string.h"
#include"bitmap.h"

void bitmapInit(Bitmap* map,void* base_,uint32_t length_,uint32_t limits_)
{
    map->base = (uint8_t*)base_; 
    map->bits = length_ * 8;
    map->length = length_;
    map->limits = limits_;

    memset(base_,'\0',length_);
}

int bitmapScan(Bitmap* map,uint32_t count)
{
    uint32_t freeBitIndex = 0;
    while(0xff == map->base[freeBitIndex] && freeBitIndex < map->length)
        ++freeBitIndex;

    if(freeBitIndex == map->length)
        return -1;

    uint8_t byteIndex = 0;
    while((uint8_t)(BITMAP_MASK << byteIndex) & map->base[freeBitIndex])
        ++byteIndex;

    uint32_t bitmapIndexStart = freeBitIndex * 8 + byteIndex;
    if(count == 1)
        return bitmapIndexStart;

    uint32_t freeBytes = map->bits - bitmapIndexStart;
    if(count < freeBytes)
        return -1;

    uint32_t count_ = count;
    uint32_t bitmapIndexStart_ = bitmapIndexStart;
    uint32_t boom = 0;
    uint32_t boom_ = 0;
    while(count_--)
    {
        bitmapGetPos(map,bitmapIndexStart_) == 1 ? ++boom : ++boom_;
        ++bitmapIndexStart_;
    }
    
    if(boom == count && boom_ == 0)
        return bitmapIndexStart;
    return -1;
}

bool bitmapGetPos(Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    return (map->base[pos >> 3] & (0x80 >> (pos & 0x07)));
}

void bitmapSetPos(Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    map->base[pos >> 3] |= (0x80 >> (pos & 0x07));
}

void bitmapClearPos(Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    map->base[pos >> 3] &= ~(0x80 >> (pos & 0x07)); 
}


