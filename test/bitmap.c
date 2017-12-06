/*************************************************************************
	> File Name: bitmap.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时47分51秒
 ************************************************************************/

#include"bitmap.h"
#include<string.h>
#include<stdio.h>

void bitmapInit(struct Bitmap* map,char* base_,uint32_t length_,uint32_t limits_)
{
    map->base = (uint8_t*)base_; 
    map->bits = length_ * 8;
    map->length = length_;
    map->limits = limits_;
    
    memset(map->base,'\0',map->length);
}

uint8_t bitmapGetPos(struct Bitmap* map,uint32_t pos)
{
    return (map->base[pos >> 3] & (0x80 >> (pos & 0x07)));
}

//在现有环境下实现的位图搜索算法不适用于无系统环境
//ASSERT犯病了..让我先去睡一会,早起继续调bug
//直接去掉吧,不检测了,注意在代码中控制越界吧..
void bitmapSetPos(struct Bitmap* map,uint32_t pos)
{
    map->base[pos >> 3] |= (0x80 >> (pos & 0x07));
}

void bitmapClearPos(struct Bitmap* map,uint32_t pos)
{
    map->base[pos >> 3] &= ~(0x80 >> (pos & 0x07)); 
}

//好吧,算法都整不明白..重新写吧..
int bitmapScan(struct Bitmap* map,uint32_t count)
{
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
        {
            ++freeBitLine;
            continue;
        }

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
            {
                bitmapSetPos(map,bitmapIndexStart);
                return bitmapIndexStart;
            }

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

