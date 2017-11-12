/*************************************************************************
	> File Name: AddressPool.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时19分43秒
 ************************************************************************/

#include"AddressPool.h"

namespace memory
{
    VirtualPool::VirtualPool(void* bitmapBaseAddr_,uint32_t bits_,uint32_t addrStart_,uint32_t poolSize_) :
        bitmap(bitmapBaseAddr_,bits_),
        addrStart(addrStart_),
        poolSize(poolSize_)
    {};

    PhysicalPool::PhysicalPool(void* bitmapBaseAddr_,uint32_t bits_,uint32_t addrStart_,uint32_t poolSize_) :
        bitmap(bitmapBaseAddr_,bits_),
        addrStart(addrStart_),
        poolSize(poolSize_)
    {}
}
