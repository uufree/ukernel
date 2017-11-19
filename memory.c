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

void initMemoryMessage()
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

void printMemoryMessage()
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
void initKernelMemory(struct KernelMemory* kMemory,const MemoryMessage* mm)
{
    uint32_t length = mm->kernelFreePages / 8;
    mm->kernelFreePages % 8 ? ++length : length+=0;

    initVirtualPool(&kernelVPool,&mm->kernelBitmapBaseAddr,length,K_VIR_MEMORY_BASE,mm->kernelFreePages * PG_SIZE);
    initPhysicalPool(&kernelPPool,(void*)BITMAP_BASE,length,mm->kernelPhyStart,mm->kernelFreePages * PG_SIZE);
}

static uint32_t* getVaddrPDE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return idx;
}

static uint32_t* getVaddrPTE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return idx;
}

void KernelMemory::makePageMap(uint32_t vaddr,uint32_t paddr)
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

    uint32_t KernelMemory::mallocPage(uint32_t count)
    {
        uint32_t vaddr = kernelVPool.getVaddr(count);   
        if(vaddr == 0)
            return 0;
        
        uint32_t vaddr_ = vaddr;
        
        uint32_t paddr;
        while(count--)
        {
            paddr = kernelPPool.getPaddr();
            if(paddr == 0)
                return 0;

            makePageMap(vaddr,paddr);
            vaddr += PG_SIZE;
        }
        return vaddr_; 
    }
    
    void* KernelMemory::palloc(uint32_t count)
    {
        static uint32_t addr = mallocPage(count);
        return &addr;
    }
    
    UserMemory::UserMemory(const MemoryMessage* mm) :
        userPPool(&mm->userBitmapBaseAddr,mm->userFreePages,mm->userPhyStart,mm->userFreePages * PG_SIZE)
    {};


