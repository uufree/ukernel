/*************************************************************************
	> File Name: bitmap.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 19时58分40秒
 ************************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

#include"stdint.h"

class Bitmap final
{
    public:
        explicit Bitmap(uint32_t memorySum,uint32_t limits_ = 4096);
        explicit Bitmap(const Bitmap& lhs);
        Bitmap& operator=(const Bitmap& lhs);
        ~Bitmap();
        
        uint32_t scan(uint32_t count);
        void get(uint32_t pos);
        void set(uint32_t pos,bool value);

    private:
        uint8_t* array;
        uint32_t size;  //size*8=bits
        uint32_t memorySum;
        uint32_t limits;
};


#endif
