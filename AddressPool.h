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

#define PG_SIZE 4096    
#define BITMAP_BASE 0xc0009a00    
#define K_VIR_MEMORY_BASE 0xc0100000

class VirtualPool final
{
    public:
        VirtualPool(void* bitmapBaseAddr,uint32_t bits,uint32_t addrStart_,uint32_t poolSize_);
        
    private:
        Bitmap bitmap;
        uint32_t addrStart;
        uint32_t poolSize;
};

class PhysicalPool final
{
    public:
        PhysicalPool(void* bitmapBaseAddr,uint32_t bits,uint32_t addrStart_,uint32_t poolSize_);
    
    private:
        Bitmap bitmap;
        uint32_t addrStart;
        uint32_t poolSize;
};

class PhysicalPool *kernelPhyPool,*userPhyPool;
class VirtualPool* kernelVirPool;

extern "C"
{
    void memoryInit(uint32_t allMemory);
};

#endif
