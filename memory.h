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

//存储所有关于内存的信息
struct MemoryMessage
{
    static uint32_t usedPageTableSize;
    static uint32_t usedMemory;
    static uint32_t freeMemory;
    static uint32_t freePages;

    static uint32_t kernelFreePages;
    static uint32_t kernelPhyStart;
    static uint32_t kernelVirStart;
    static uint32_t kernelPhyBitmapBaseAddr;
    static uint32_t kernelVirBitmapBaseAddr;
    static uint32_t kernelBitmapLenght;

    static uint32_t userFreePages;
    static uint32_t userPhyStart;
    static uint32_t userPhyBitmapBaseAddr;
    static uint32_t userBitmapLenght;
};

//内核内存池，分配出来的是直接可供使用的内核内存
struct KernelMemory
{
    struct VirtualPool kernelVPool;
    struct PhysicalPool kernelPPool;
};

//用户物理内存池，分配出来的仅仅只是物理内存，还需要在用户空间做映射
struct UserMemory
{
    struct PhysicalPool userPPool;
};

//全局变量
struct Memory
{
    struct MemoryMessage memoryMeesage;
    struct KernelMemory kernelMemory;
    struct UserMemory userMemory;
};

//MemoryMessage operator
void initMemoryMessage(struct MemoryMessage* message);
void printMemoryMessage(struct MemoryMessage* message);

//Kernel Memory operator
void initKernelMemory(struct KernelMemory* kMemory,const MemoryMessage* memoryMeesage);
uint32_t mallocPageInKernelMemory(uint32_t count);

//user Memory operator
void initUserMemory(struct UserMemory* uMemory,const MemoryMessage* memoryMeesage);
uint32_t mallocPageInUserMemory(); 

//memory operator
void initMemory(struct Memory* memory); 

//usually operator
uint32_t* getVaddrPDE(uint32_t vaddr);
static uint32_t* getVaddrPTE(uint32_t vaddr);
void makePageMap(uint32_t vaddr,uint32_t paddr);

//所有操作围绕着这一个全局变量
struct Memory memory;

#endif
