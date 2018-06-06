/*************************************************************************
	> File Name: memory.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时18分18秒
 ************************************************************************/

#ifndef _MEMORY_H
#define _MEMORY_H

#include"stdint.h"
#include"print.h"
#include"bitmap.h"
#include"memory_pool.h"

#define BITMAP_BASE  0xc009a000
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
    uint32_t all_memory;
    uint32_t used_page_table_size;
    uint32_t used_memory;
    uint32_t free_memory;
    uint32_t free_pages;

    uint32_t kernel_free_pages;
    uint32_t kernel_phy_start;
    uint32_t kernel_vir_start;
    uint32_t kernel_phy_bitmap_base_addr;
    uint32_t kernel_vir_bitmap_base_addr;
    uint32_t kernel_bitmap_lenght;

    uint32_t user_free_pages;
    uint32_t user_phy_start;
    uint32_t user_phy_bitmap_base_addr;
    uint32_t user_bitmap_lenght;
};

//内核内存池，分配出来的是直接可供使用的内核内存
struct KernelMemory
{
    struct VirtualPool kernel_vir_pool;
    struct PhysicalPool kernel_phy_pool;
};

//用户物理内存池，分配出来的仅仅只是物理内存，还需要在用户空间做映射
struct UserMemory
{
    struct PhysicalPool user_phy_pool;
};

//全局变量
struct Memory
{
    struct MemoryMessage memory_message;
    struct KernelMemory kernel_memory;
    struct UserMemory user_memory;
};

//所有操作围绕着这一个全局变量
struct Memory memory;

//MemoryMessage operator
void init_memory_message(struct MemoryMessage* message);
void print_memory_message(struct MemoryMessage* message);

//Kernel Memory operator
void init_kernel_memory(struct KernelMemory* kMemory,struct MemoryMessage* memoryMeesage);
uint32_t* malloc_page_kernel_memory(uint32_t count);

//user Memory operator
void init_user_memory(struct UserMemory* uMemory,struct MemoryMessage* memoryMeesage);
uint32_t* malloc_page_user_memory(); 

//memory operator
void init_memory(); 

//usually operator
uint32_t* get_vir_addr_PDE(uint32_t vaddr);
static uint32_t* get_vir_addr_PTE(uint32_t vaddr);
void make_page_map(uint32_t vaddr,uint32_t paddr);


#endif

