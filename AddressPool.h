/*************************************************************************
	> File Name: AddressPool.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时19分31秒
 ************************************************************************/

#ifndef _ADDRESSPOOL_H
#define _ADDRESSPOOL_H

#include"stdint.h"
#include"bitmap.h"
#include"print.h"

namespace memory
{
    class VirtualPool final
    {
        public:
            VirtualPool(void* bitmapBaseAddr_,uint32_t bits_,uint32_t addrStart_,uint32_t poolSize_);
        
        private:
            data::Bitmap bitmap;
            uint32_t addrStart;
            uint32_t poolSize;
    };

    class PhysicalPool final
    {
        public:
            PhysicalPool(void* bitmapBaseAddr,uint32_t bits,uint32_t addrStart_,uint32_t poolSize_);
    
        private:
            data::Bitmap bitmap;
            uint32_t addrStart;
            uint32_t poolSize;
    };
}

#endif
