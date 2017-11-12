/*************************************************************************
	> File Name: bitmap.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 19时58分40秒
 ************************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

#include"stdint.h"

#define BITMAP_MASK 1

struct Bitmap
{
    uint8_t* base;
    uint32_t bits;
    uint32_t length;  //length * 8 = bits
    uint32_t limits;
};

void bitmapInit(Bitmap* map,void* base_,uint32_t length_,uint32_t limits_ = 4096);
int bitmapScan(Bitmap* map,uint32_t count);
void bitmapClearPos(Bitmap* map,uint32_t pos);
void bitmapSetPos(Bitmap* map,uint32_t pos);
bool bitmapGetPos(Bitmap* map,uint32_t pos);

#endif
