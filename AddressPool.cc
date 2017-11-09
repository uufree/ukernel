/*************************************************************************
	> File Name: AddressPool.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时19分43秒
 ************************************************************************/

#include"AddressPool.h"
#include"print.h"

extern void memoryInit(uint32_t allMemory);

void memoryInit(uint32_t allMemory)
{
    printStr((char*)"Memory Init Start!\n");
    uint32_t usedPageTableSize = 256 * PG_SIZE;
    uint32_t usedMemory = usedPageTableSize + 0x100000;
    uint32_t freeMemory = allMemory - usedMemory;
    uint32_t freePages = freeMemory / PG_SIZE;

    uint32_t kernelFreePages = freePages / 4;
    uint32_t userFreePages = freePages - kernelFreePages;
    
    uint32_t kernelBitmapLenght = kernelFreePages / 8;
    uint32_t userBitmapLenght = userFreePages / 8;

    uint32_t kernelPhyStart = usedMemory;
    uint32_t userPhyStart = usedMemory + kernelFreePages * PG_SIZE;
    
    uint32_t userBitmapBaseAddr = BITMAP_BASE + userBitmapLenght;
    uint32_t kernelVirBitmapBaseAddr = BITMAP_BASE + kernelBitmapLenght + userBitmapLenght;

    static class PhysicalPool kernelPhyPool_((void*)BITMAP_BASE,kernelFreePages,kernelPhyStart,kernelFreePages * PG_SIZE);
    static class PhysicalPool userPhyPool_((void*)&userBitmapBaseAddr,userFreePages,userPhyStart,userFreePages * PG_SIZE);    
    static class VirtualPool kernelVirPool_((void*)&kernelVirBitmapBaseAddr,kernelFreePages,K_VIR_MEMORY_BASE,kernelFreePages * PG_SIZE);

    kernelPhyPool = &kernelPhyPool_;
    userPhyPool = &userPhyPool_;
    kernelVirPool = &kernelVirPool_;
    
/*****************INPUT MEMORY MESSAGE***************************/
    printStr((char*)"KernelPhyPool->bitmapStart: 0x");
    printInt(BITMAP_BASE);
    printChar('\n');
    printStr((char*)"KernelPhyPool->bitmapSize: ");
    printInt(kernelBitmapLenght);
    printChar('\n');
    printStr((char*)"KernelPhyPool->addrStart: 0x");
    printInt(kernelPhyStart);
    printChar('\n');
    printStr((char*)"KernelPhyStart->poolSize: ");
    printInt(kernelFreePages * PG_SIZE);
    printChar('\n');
    
    printStr((char*)"UserPhyPool->bitmapStart: 0x");
    printInt(userBitmapBaseAddr);
    printChar('\n');
    printStr((char*)"UserPhyPool->bitmapSize: ");
    printInt(userBitmapLenght);
    printChar('\n');
    printStr((char*)"UserPhyPool->addrStart: 0x");
    printInt(userPhyStart);
    printChar('\n');
    printStr((char*)"UserPhyStart->poolSize: ");
    printInt(userFreePages * PG_SIZE);
    printChar('\n');
    
    printStr((char*)"KernelVirPool->bitmapStart: 0x");
    printInt(kernelVirBitmapBaseAddr);
    printChar('\n');
    printStr((char*)"KernelVirPool->bitmapSize: ");
    printInt(kernelBitmapLenght);
    printChar('\n');
    printStr((char*)"KernelVirPool->addrStart: 0x");
    printInt(K_VIR_MEMORY_BASE);
    printChar('\n');
    printStr((char*)"UserPhyStart->poolSize: ");
    printInt(kernelFreePages * PG_SIZE);
    printChar('\n');
    
    printStr((char*)"Memory Init Done!\n");
}

