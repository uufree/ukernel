/*************************************************************************
	> File Name: AddressPool.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时19分43秒
 ************************************************************************/

#include"AddressPool.h"
#include"print.h"

extern void memoryInit(uint32_t allMemory)
{
    printStr((char*)"Memory Init Start!\n");
    uint32_t usedPageTableSize = 256 * PG_SIZE;
    uint32_t usedMemory = usedPageTableSize + 0x100000;
    uint32_t freeMemory = allMemory - usedMemory;
    uint32_t freePages = freeMemory / PG_SIZE;

    uint32_t kernelFreePages = freePages / 4;
    uint32_t userFreePages = freePages - kernelFreePages;

    uint32_t kernelPhyStart = usedMemory;
    uint32_t userphyStart = usedMemory + kernelFreePages * PG_SIZE;


}

