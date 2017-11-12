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
    
    uint32_t VirtualPool::getVaddr(uint32_t count)
    {
        int idx = bitmap.scan(count);
        if(idx == -1)
            return 0;

        uint32_t ipx = 0;
        while(ipx < count)
        {
            bitmap.set(idx + ipx);
            ++ipx;
        }
         
        return addrStart + idx * PG_SIZE;
    }

    PhysicalPool::PhysicalPool(void* bitmapBaseAddr_,uint32_t bits_,uint32_t addrStart_,uint32_t poolSize_) :
        bitmap(bitmapBaseAddr_,bits_),
        addrStart(addrStart_),
        poolSize(poolSize_)
    {};

    uint32_t PhysicalPool::getPaddr()
    {
        int idx = bitmap.scan(1);
        if(idx == -1)
            return 0;
        bitmap.set(idx);

        return addrStart + idx * PG_SIZE;
    }
}
