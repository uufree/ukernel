/*************************************************************************
	> File Name: bitmap.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 19时58分40秒
 ************************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

#include"stdint.h"

#define BITMAP_MASK 0x80

struct Bitmap
{
    uint8_t* base;
    uint32_t bits;
    uint32_t length;  //length * 8 = bits
    uint32_t limits;
};

void bitmapInit(struct Bitmap* map,char* base_,uint32_t length_,uint32_t limits_);
uint8_t bitmapGetPos(struct Bitmap* map,uint32_t pos);
void bitmapSetPos(struct Bitmap* map,uint32_t pos);
void bitmapClearPos(struct Bitmap* map,uint32_t pos);
int bitmapScan(struct Bitmap* map,uint32_t count);

#endif
