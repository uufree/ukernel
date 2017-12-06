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
    printStr((char*)"MemoryMessage Init Start!\n");
    
    message->allMemory = (*(uint32_t*)(0xb00));
    message->usedPageTableSize = 256 * PG_SIZE;
    message->usedMemory = message->usedPageTableSize + 0x100000;
    message->freeMemory = message->allMemory - message->usedMemory;
    message->freePages = message->freeMemory / PG_SIZE;

    message->kernelFreePages = message->freePages / 2;
    message->userFreePages = message->freePages - message->kernelFreePages;
    
    message->kernelBitmapLenght = message->kernelFreePages / 8;
    message->userBitmapLenght = message->userFreePages / 8;
    
    message->kernelPhyStart = message->usedMemory;
    message->kernelVirStart = K_VIR_MEMORY_BASE; 
    
    message->userPhyStart = message->usedMemory + message->kernelFreePages * PG_SIZE;
    
    message->userPhyBitmapBaseAddr = BITMAP_BASE + message->kernelBitmapLenght;
    
    message->kernelPhyBitmapBaseAddr = BITMAP_BASE;
    message->kernelVirBitmapBaseAddr = BITMAP_BASE + message->userBitmapLenght + message->kernelBitmapLenght;
    
    printStr((char*)"MemoryMessage Init Done!\n");
}

void printMemoryMessage(struct MemoryMessage* message)
{
    /**********All Memory**********/
    printStr((char*)"*************All Memory****************\n");
    printStr((char*)"AllMemory: 0x");
    printInt(message->allMemory);
    printChar('\n');
    printStr((char*)"UsedPageTableSize: 0x");
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
    printStr((char*)"*************Kernel Memory****************\n");
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
    printStr((char*)"*************User Memory****************\n");
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
    printStr((char*)"****************************************\n");
}

void initKernelMemory(struct KernelMemory* kMemory,struct MemoryMessage* mm)
{
    initVirtualPool(&kMemory->kernelVPool,mm->kernelVirBitmapBaseAddr,mm->kernelBitmapLenght,mm->kernelVirStart,mm->kernelFreePages * PG_SIZE);
     
    initPhysicalPool(&kMemory->kernelPPool,mm->kernelPhyBitmapBaseAddr,mm->kernelBitmapLenght,mm->kernelPhyStart,mm->kernelFreePages * PG_SIZE);
}

uint32_t* mallocPageInKernelMemory(uint32_t count)
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
    
    return (uint32_t*)vaddr_; 
}

void initUserMemory(struct UserMemory* uMemory,struct MemoryMessage* mm)
{
    initPhysicalPool(&uMemory->userPPool,mm->userPhyBitmapBaseAddr,mm->userBitmapLenght,mm->userPhyStart,mm->userFreePages * PG_SIZE);
}

uint32_t* mallocPageInUserMemory()
{
    uint32_t paddr = getPoolAddr(&memory,PF_USER_PHYSICAL,1);
    if(paddr == 0)
        return 0;
    return (uint32_t*)paddr;
}

void initMemory()
{
    initMemoryMessage(&memory.memoryMeesage);
    initKernelMemory(&memory.kernelMemory,&memory.memoryMeesage);
    initUserMemory(&memory.userMemory,&memory.memoryMeesage);
//    printMemoryMessage(&memory.memoryMeesage);
    printStr((char*)"Init Memory Done!\n");
}

//这两段有争议，为什么可以再函数内返回局部变量？重点标注一下
//目的是获得一个指针，但是方式似乎有所不妥
//两天后解答上述问题：在ubuntu16.04下会出现段错误，因为现成的系统会有段保护措施，所以会造成段访问错误
uint32_t* getVaddrPTE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return idx;
}

uint32_t* getVaddrPDE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return idx;
}

//2017.12.03
//这块又有问题了..哎，啥时候才能把bug一锅端啊？
//memset对于新分配内存的初始化有问题
//问题在于交给memset的地址必须为虚拟地址..
//我靠,BITMAP_BASE多写了一个0..折腾的三天..
//我靠，虚拟地址转换写错的，折腾了三天..
void makePageMap(uint32_t vaddr,uint32_t paddr)
{
    uint32_t* pde = getVaddrPDE(vaddr);
    uint32_t* pte = getVaddrPTE(vaddr);
/*    
    printStr((char*)"vaddr: 0x");
    printInt((uint32_t)vaddr);
    printChar('\n');
        
    printStr((char*)"paddr: 0x");
    printInt((uint32_t)paddr);
    printChar('\n');
    
    printStr((char*)"pde: 0x");
    printInt((uint32_t)pde);
    printChar('\n');
    
    printStr((char*)"pte: 0x");
    printInt((uint32_t)pte);
    printChar('\n');
 
    printStr((char*)"*pte: 0x");
    printInt((uint32_t)*pte);
    printChar('\n');
*/
    if(*pde & 0x00000001)
    {
//        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1); 
    }
    else
    {
        uint32_t pdePaddr = getPoolAddr(&memory,PF_KERNEL_PHYSICAL,1);
        *pde = (pdePaddr | PG_US_U | PG_RW_W | PG_P_1);
        
        memset((void*)((uint32_t)pte & 0xfffff000),'\0',PG_SIZE); 
        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}


