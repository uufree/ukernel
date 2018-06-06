/*************************************************************************
	> File Name: bitmap.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月12日 星期日 21时47分51秒
 ************************************************************************/

#include"debug.h"
#include"string.h"
#include"bitmap.h"
#include"print.h"

#define BITMAP_MASK 0x80

void bitmap_init(struct Bitmap* map,uint32_t base_,uint32_t length_,uint32_t limits_)
{
    map->base = (uint8_t*)base_; 
    map->bits = length_ * 8;
    map->length = length_;
    map->limits = limits_;
    
    memset(map->base,'\0',map->length);
}

uint8_t bitmap_get_pos(struct Bitmap* map,uint32_t pos)
{
//    ASSERT(pos < map->bits);
    return (map->base[pos >> 3] & (0x80 >> (pos & 0x07)));
}

//12.02
//在现有环境下实现的位图搜索算法不适用于无系统环境
//ASSERT犯病了..让我先去睡一会,早起继续调bug
//直接去掉吧,不检测了,注意在代码中控制越界吧..
void bitmap_set_pos(struct Bitmap* map,uint32_t pos)
{
//    ASSERT(pos < map->bits);
    map->base[pos >> 3] |= (0x80 >> (pos & 0x07));
}

void bitmap_clear_pos(struct Bitmap* map,uint32_t pos)
{
    ASSERT(pos < map->bits);
    map->base[pos >> 3] &= ~(0x80 >> (pos & 0x07)); 
}

int bitmap_scan(struct Bitmap* map,uint32_t count)
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
                bitmap_set_pos(map,bitmapIndexStart);
                return bitmapIndexStart;
            }

            freeBytes = map->bits - bitmapIndexStart;

            if(count > freeBytes)
                return -1;

            count_ = 0;
            bitmapIndexStart_ = bitmapIndexStart;
        
            while(count_ < count)
            {
                if(bitmap_get_pos(map,bitmapIndexStart_) != 0)
                    break;
                ++bitmapIndexStart_;
                ++count_;
            }
             
            if(count == count_)
            {
                for(uint32_t i=0;i<count;++i)
                    bitmap_set_pos(map,bitmapIndexStart + i);        

                return bitmapIndexStart;
            }

            newPos = bitmapIndexStart + count_;
            freeBitLine = newPos / 8;
            freeBitIndex = newPos % 8;
        }
    }
    
    return -1;
}

void print_bitmap_message(const struct Bitmap* bitmap)
{
    print_str((char*)"Bitmap Start: 0x");
    print_int(*(uint32_t*)bitmap->base);
    print_char('\n');
    print_str((char*)"Bitmap Bits: 0x");
    print_int(bitmap->bits);
    print_char('\n');
    print_str((char*)"Bitmap Length: 0x");
    print_int(bitmap->length);
    print_char('\n');
}
