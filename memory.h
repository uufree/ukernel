/*************************************************************************
	> File Name: memory.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 20时18分28秒
 ************************************************************************/

#ifndef _MEMORY_H
#define _MEMORY_H

#include"stdint.h"
#include"print.h"
#include"bitmap.h"
#include"MemoryPool.h"

#define BITMAP_BASE  0xc0009a00
#define K_VIR_MEMORY_BASE  0xc0100000
    
#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0 
#define PG_US_U 4
    
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22) 
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

struct MemoryMessage
{
    static uint32_t usedPageTableSize;
    static uint32_t usedMemory;
    static uint32_t freeMemory;
    static uint32_t freePages;

    static uint32_t kernelFreePages;
    static uint32_t kernelPhyStart;
    static uint32_t kernelBitmapBaseAddr;
    static uint32_t kernelBitmapLenght;

    static uint32_t userFreePages;
    static uint32_t userPhyStart;
    static uint32_t userBitmapBaseAddr;
    static uint32_t userBitmapLenght;
};

void initMemoryMessage();
void printMemoryMessage();

struct KernelMemory
{
    VirtualPool kernelVPool;
    PhysicalPool kernelPPool;
};
//Kernel Memory operator
void initKernelMemory(struct KernelMemory* kMemory,const MemoryMessage* memoryMeesage);
uint32_t mallocPageInKernelMemory(uint32_t count);

//usually operator
static uint32_t* getVaddrPDE(uint32_t vaddr);
static uint32_t* getVaddrPTE(uint32_t vaddr);
static void makePageMap(uint32_t vaddr,uint32_t paddr);

#endif
