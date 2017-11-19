/*************************************************************************
	> File Name: memory.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时16分08秒
 ************************************************************************/

#include"memory.h"
#include"print.h"
#include"debug.h"
#include"string.h"

void initMemoryMessage(struct MemoryMessage* message)
{
    uint32_t allMemory = (*(uint32_t*)(0xb00));

    printStr((char*)"MemoryMessage Init Start!\n");
    message->usedPageTableSize = 256 * PG_SIZE;
    message->usedMemory = message->usedPageTableSize + 0x100000;
    message->freeMemory = allMemory - message->usedMemory;
    message->freePages = message->freeMemory / PG_SIZE;

    message->kernelFreePages = message->freePages / 4;
    message->kernelPhyStart = message->usedMemory;
    message->kernelVirStart = K_VIR_MEMORY_BASE; 
    message->kernelPhyBitmapBaseAddr = BITMAP_BASE;
    message->kernelVirBitmapBaseAddr = BITMAP_BASE + message->userBitmapLenght + message->kernelBitmapLenght;
    message->kernelBitmapLenght = message->kernelFreePages / 8;
    
    message->userFreePages = message->freePages - message->kernelFreePages;
    message->userPhyStart = message->usedMemory + message->kernelFreePages * PG_SIZE;
    message->userPhyBitmapBaseAddr = BITMAP_BASE + message->kernelBitmapLenght;
    message->userBitmapLenght = message->userFreePages / 8;

    printStr((char*)"MemoryMessage Init Done!\n");
}

void printMemoryMessage(struct MemoryMessage* message)
{
    /**********All Memory**********/
    printStr((char*)"UsedPageTableSize: ");
    printInt(message->usedPageTableSize);
    printChar('\n');
    printStr((char*)"UsedMemory: 0x");
    printInt(message->usedMemory);
    printChar('\n');
    printStr((char*)"FreeMemory: 0x");
    printInt(message->freeMemory);
    printChar('\n');
    printStr((char*)"FreePages 0x");
    printInt(message->freePages);
    printChar('\n');
        
    /********Kernel Memory*************/
    printStr((char*)"KernelFreePages: 0x");
    printInt(message->kernelFreePages);
    printChar('\n');
    printStr((char*)"KernelPhyStart: 0x");
    printInt(message->kernelPhyStart);
    printChar('\n');
    printStr((char*)"KernelVirStart: 0x");
    printInt(message->kernelVirStart);
    printChar('\n');
    printStr((char*)"KernelPhyBitmapBaseAddr: 0x");
    printInt(message->kernelPhyBitmapBaseAddr);
    printChar('\n');
    printStr((char*)"KernelVirBitmapBaseAddr: 0x");
    printInt(message->kernelVirBitmapBaseAddr);
    printChar('\n');
    printStr((char*)"KernelBitmapLenght 0x");
    printInt(message->kernelBitmapLenght);
    printChar('\n');
    
    /*********User Memory***********/
    printStr((char*)"UserFreePages: 0x");
    printInt(message->userFreePages);
    printChar('\n');
    printStr((char*)"UserPhyStart: 0x");
    printInt(message->userPhyStart);
    printChar('\n');
    printStr((char*)"UserPhyBitmapBaseAddr: 0x");
    printInt(message->userPhyBitmapBaseAddr);
    printChar('\n');
    printStr((char*)"UserBitmapLenght: 0x");
    printInt(message->userBitmapLenght);
    printChar('\n');
}

void initKernelMemory(struct KernelMemory* kMemory,const MemoryMessage* mm)
{
    initVirtualPool(&kMemory->kernelVPool,&mm->kernelVirBitmapBaseAddr,mm->kernelBitmapLenght,mm->kernelVirStart,mm->kernelFreePages * PG_SIZE);
    initPhysicalPool(&kMemory->kernelPPool,&mm->kernelPhyBitmapBaseAddr,mm->kernelBitmapLenght,mm->kernelPhyStart,mm->kernelFreePages * PG_SIZE);
}

uint32_t mallocPageInKernelMemory(uint32_t count)
{
    uint32_t vaddr = getPoolAddr(&memory,PF_KERNEL_VIRTUAL,count);   
    if(vaddr == 0)
        return 0;
        
    uint32_t vaddr_ = vaddr;
        
    uint32_t paddr = 0;
    while(count--)
    {
        paddr = getPoolAddr(&memory,PF_KERNEL_PHYSICAL,1);
        if(paddr == 0)
            return 0;

        makePageMap(vaddr,paddr);
        vaddr += PG_SIZE;
    }
    return vaddr_; 
}

void initUserMemory(struct UserMemory* uMemory,const MemoryMessage* mm)
{
    initPhysicalPool(&uMemory->userPPool,&mm->userPhyBitmapBaseAddr,mm->userBitmapLenght,mm->userPhyStart,mm->userFreePages * PG_SIZE);
}

uint32_t mallocPageInUserMemory()
{
    uint32_t paddr = getPoolAddr(&memory,PF_USER_PHYSICAL,1);
    if(paddr == 0)
        return 0;
    return paddr;
}

void initMemory(struct Memory* memory_)
{
    initMemoryMessage(&memory_->memoryMeesage);
    initKernelMemory(&memory_->kernelMemory,&memory_->memoryMeesage);
    initUserMemory(&memory_->userMemory,&memory_->memoryMeesage);
}

uint32_t* getVaddrPDE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return idx;
}

static uint32_t* getVaddrPTE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return idx;
}

static void KernelMemory::makePageMap(uint32_t vaddr,uint32_t paddr)
{
    uint32_t* pde = getVaddrPDE(vaddr);
    uint32_t* pte = getVaddrPTE(vaddr);
        
    if(*pde & 0x00000001)
    {
        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1); 
    }
    else
    {
        uint32_t pdePaddr = kernelPPool.getPaddr();
        *pde = (pdePaddr | PG_US_U | PG_RW_W | PG_P_1);
        uint32_t addr = *pte & 0xfffff000;
        memset((void*)&addr,0,PG_SIZE);   
        ASSERT(!(pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}


