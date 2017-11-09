/*************************************************************************
	> File Name: bitmap.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 19时58分40秒
 ************************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

#include"stdint.h"
#include"debug.h"
#include"string.h"

#define BITMAP_MASK 1

class Bitmap final
{
    public:
        explicit Bitmap(void* base_,uint32_t bits_,uint32_t limits_ = 4096) :
            base((uint8_t*)base_),
            bits(bits_),
            size(bits_ / 8),
            limits(limits_)
        {
            memset(base,'0',size);
        };

        explicit Bitmap(const Bitmap& lhs) = delete;
        Bitmap& operator=(const Bitmap& lhs) = delete;
        
        uint32_t scan(uint32_t count)
        {
            uint32_t freeBitIndex = 0;
            while(0xff == base[freeBitIndex] && freeBitIndex < size)
                ++freeBitIndex;
            
            if(freeBitIndex == size)
                return -1;
            
            uint8_t byteIndex = 0;
            while((uint8_t)(BITMAP_MASK << byteIndex) & base[freeBitIndex])
                ++byteIndex;
            
            uint32_t bitIndexStart = freeBitIndex * 8 + byteIndex;
            if(count == 1)
                return bitIndexStart;

            uint32_t freeBytes = bits - bitIndexStart;
            if(freeBytes < count)
                return -1;
            
            uint32_t cnt = count;
            uint32_t bitIndexStartTest = bitIndexStart;
            uint32_t boom = 0;  //1
            uint32_t boom_ = 0; //!1
            while(cnt--)
                get(bitIndexStartTest) == 1 ? ++boom : ++boom_;
            
            if(boom == count && boom_ == 0)
                return bitIndexStart;
            return -1;
        }
        

        bool get(uint32_t pos) const
        {ASSERT(pos < bits);return base[pos >> 3] & (0x80 >> (pos & 0x07));};
        
        void set(uint32_t pos)
        {ASSERT(pos < bits);base[pos >> 3] |= (0x80 >> (pos & 0x07));};
        
        void clear(uint32_t pos)
        {ASSERT(pos < bits);base[pos >> 3] &= ~(0x80 >> (pos & 0x07));};

    private:
        uint8_t* base;
        uint32_t bits;
        uint32_t size;  //size*8=bits
        uint32_t limits;
};


#endif
