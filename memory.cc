/*************************************************************************
	> File Name: memory.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时16分08秒
 ************************************************************************/

#include"memory.h"
#include"print.h"

namespace memory
{

    void MemoryMessage::init()
    {
        uint32_t allMemory = (*(uint32_t*)(0xb00));

        printStr((char*)"MemoryMessage Init Start!\n");
        usedPageTableSize = 256 * PG_SIZE;
        usedMemory = usedPageTableSize + 0x100000;
        freeMemory = allMemory - usedMemory;
        freePages = freeMemory / PG_SIZE;

        kernelFreePages = freePages / 4;
        userFreePages = freePages - kernelFreePages;
    
        kernelBitmapLenght = kernelFreePages / 8;
        userBitmapLenght = userFreePages / 8;

        kernelPhyStart = usedMemory;
        userPhyStart = usedMemory + kernelFreePages * PG_SIZE;
    
        userBitmapBaseAddr = BITMAP_BASE + userBitmapLenght;
        kernelBitmapBaseAddr = BITMAP_BASE + kernelBitmapLenght + userBitmapLenght;

        printStr((char*)"MemoryMessage Init Done!\n");
    }

    void MemoryMessage::printMemoryMessage()
    {
        /**********All Memory**********/
        printStr((char*)"UsedPageTableSize: ");
        printInt(usedPageTableSize);
        printChar('\n');
        printStr((char*)"UsedMemory: 0x");
        printInt(usedMemory);
        printChar('\n');
        printStr((char*)"FreeMemory: 0x");
        printInt(freeMemory);
        printChar('\n');
        printStr((char*)"FreePages 0x");
        printInt(freePages);
        printChar('\n');
        
        /********Kernel Memory*************/
        printStr((char*)"KernelFreePages: 0x");
        printInt(kernelFreePages);
        printChar('\n');
        printStr((char*)"KernelPhyStart: 0x");
        printInt(kernelPhyStart);
        printChar('\n');
        printStr((char*)"KernelBitmapBaseAddr: 0x");
        printInt(kernelBitmapBaseAddr);
        printChar('\n');
        printStr((char*)"KernelBitmapLenght 0x");
        printInt(kernelBitmapLenght);
        printChar('\n');
    
        /*********User Memory***********/
        printStr((char*)"UserFreePages: 0x");
        printInt(userFreePages);
        printChar('\n');
        printStr((char*)"UserPhyStart: 0x");
        printInt(userPhyStart);
        printChar('\n');
        printStr((char*)"UserBitmapBaseAddr: 0x");
        printInt(userBitmapBaseAddr);
        printChar('\n');
        printStr((char*)"UserBitmapLenght: 0x");
        printInt(userBitmapLenght);
        printChar('\n');
    }
    
    KernelMemory::KernelMemory(const MemoryMessage* mm) :
        kernelVPool(&mm->kernelBitmapBaseAddr,mm->kernelFreePages,K_VIR_MEMORY_BASE,mm->kernelFreePages * PG_SIZE),
        kernelPPool((void*)BITMAP_BASE,mm->kernelFreePages,mm->kernelPhyStart,mm->kernelFreePages * PG_SIZE)     
    {};

    UserMemory::UserMemory(const MemoryMessage* mm) :
        userPPool(&mm->userBitmapBaseAddr,mm->userFreePages,mm->userPhyStart,mm->userFreePages * PG_SIZE)
    {};
}


