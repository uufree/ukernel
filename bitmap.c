/*************************************************************************
	> File Name: bitmap.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时47分51秒
 ************************************************************************/

#include"debug.h"
#include"string.h"
#include"bitmap.h"

#include<stdio.h>

void bitmapInit(struct Bitmap* map,void* base_,uint32_t length_,uint32_t limits_)
{
    map->base = (uint8_t*)base_; 
    map->bits = length_ * 8;
    map->length = length_;
    map->limits = limits_;
    
    memset(base_,'\0',map->length);
}

uint8_t bitmapGetPos(struct Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    return (map->base[pos >> 3] & (0x80 >> (pos & 0x07)));
}

void bitmapSetPos(struct Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    map->base[pos >> 3] |= (0x80 >> (pos & 0x07));
}

void bitmapClearPos(struct Bitmap* map,uint32_t pos)
{
    ASSERT(pos < bits);
    map->base[pos >> 3] &= ~(0x80 >> (pos & 0x07)); 
}

int bitmapScan(struct Bitmap* map,uint32_t count)
{//反正能实现需求，注释就懒得写了..
    uint32_t freeBitLine = 0;
    uint8_t freeBitIndex = 0;
    uint32_t bitmapIndexStart = 0;
    uint32_t freeBytes = 0;
    uint32_t count_ = 0;
    uint32_t bitmapIndexStart_ = 0;
    uint32_t newPos = 0;
    
    while(freeBitLine < map->length)
    {
        if(0xff == map->base[freeBitLine])
            continue;

        if(freeBitLine == map->length)
            return -1;
        
        while(freeBitIndex < 8)
        {
            if((uint8_t)(BITMAP_MASK >> freeBitIndex) & map->base[freeBitLine])
            {
                ++freeBitIndex;
                continue;
            }
            bitmapIndexStart = freeBitLine * 8 + freeBitIndex;

            if(count == 1)
                return bitmapIndexStart;

            freeBytes = map->bits - bitmapIndexStart;

            if(count > freeBytes)
                return -1;

            count_ = 0;
            bitmapIndexStart_ = bitmapIndexStart;
        
            while(count_ < count)
            {
                if(bitmapGetPos(map,bitmapIndexStart_) != 0)
                    break;
                ++bitmapIndexStart_;
                ++count_;
            }
             
            if(count == count_)
            {
                for(uint32_t i=0;i<count;++i)
                    bitmapSetPos(map,bitmapIndexStart + i);        

                return bitmapIndexStart;
            }

            newPos = bitmapIndexStart + count_;
            freeBitLine = newPos / 8;
            freeBitIndex = newPos % 8;
        }
    }
    
    return -1;
}
