/*************************************************************************
	> File Name: memory.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时18分45秒
 ************************************************************************/

#include"memory.h"
#include"print.h"
#include"debug.h"
#include"string.h"

void init_memory_message(struct MemoryMessage* message)
{
    print_str((char*)"MemoryMessage Init Start!\n");
    
    message->all_memory = (*(uint32_t*)(0xb00));
    message->used_page_table_size = 256 * PG_SIZE;
    message->used_memory = message->used_page_table_size + 0x100000;
    message->free_memory = message->all_memory - message->used_memory;
    message->free_pages = message->free_memory / PG_SIZE;

    message->kernel_free_pages = message->free_pages / 2;
    message->user_free_pages = message->free_pages - message->kernel_free_pages;
    
    message->kernel_bitmap_lenght = message->kernel_free_pages / 8;
    message->user_bitmap_lenght = message->user_free_pages / 8;
    
    message->kernel_phy_start = message->used_memory;
    message->kernel_vir_start = K_VIR_MEMORY_BASE; 
    
    message->user_phy_start = message->used_memory + message->kernel_free_pages * PG_SIZE;
    
    message->user_phy_bitmap_base_addr = BITMAP_BASE + message->kernel_bitmap_lenght;
    
    message->kernel_phy_bitmap_base_addr = BITMAP_BASE;
    message->kernel_vir_bitmap_base_addr = BITMAP_BASE + message->user_bitmap_lenght + message->kernel_bitmap_lenght;
    
    print_str((char*)"MemoryMessage Init Done!\n");
}

void print_memory_message(struct MemoryMessage* message)
{
    /**********All Memory**********/
    print_str((char*)"*************All Memory****************\n");
    print_str((char*)"AllMemory: 0x");
    print_int(message->all_memory);
    print_char('\n');
    print_str((char*)"UsedPageTableSize: 0x");
    print_int(message->used_page_table_size);
    print_char('\n');
    print_str((char*)"UsedMemory: 0x");
    print_int(message->used_memory);
    print_char('\n');
    print_str((char*)"FreeMemory: 0x");
    print_int(message->free_memory);
    print_char('\n');
    print_str((char*)"FreePages 0x");
    print_int(message->free_pages);
    print_char('\n');
        
    /********Kernel Memory*************/
    print_str((char*)"*************Kernel Memory****************\n");
    print_str((char*)"KernelFreePages: 0x");
    print_int(message->kernel_free_pages);
    print_char('\n');
    print_str((char*)"KernelPhyStart: 0x");
    print_int(message->kernel_phy_start);
    print_char('\n');
    print_str((char*)"KernelVirStart: 0x");
    print_int(message->kernel_vir_start);
    print_char('\n');
    print_str((char*)"KernelPhyBitmapBaseAddr: 0x");
    print_int(message->kernel_phy_bitmap_base_addr);
    print_char('\n');
    print_str((char*)"KernelVirBitmapBaseAddr: 0x");
    print_int(message->kernel_vir_bitmap_base_addr);
    print_char('\n');
    print_str((char*)"KernelBitmapLenght 0x");
    print_int(message->kernel_bitmap_lenght);
    print_char('\n');
    
    /*********User Memory***********/
    print_str((char*)"*************User Memory****************\n");
    print_str((char*)"UserFreePages: 0x");
    print_int(message->user_free_pages);
    print_char('\n');
    print_str((char*)"UserPhyStart: 0x");
    print_int(message->user_phy_start);
    print_char('\n');
    print_str((char*)"UserPhyBitmapBaseAddr: 0x");
    print_int(message->user_phy_bitmap_base_addr);
    print_char('\n');
    print_str((char*)"UserBitmapLenght: 0x");
    print_int(message->user_bitmap_lenght);
    print_char('\n');
    print_str((char*)"****************************************\n");
}

void init_kernel_memory(struct KernelMemory* kMemory,struct MemoryMessage* mm)
{
    init_virtual_pool(&kMemory->kernel_vir_pool,mm->kernel_vir_bitmap_base_addr,mm->kernel_bitmap_lenght,mm->kernel_vir_start,mm->kernel_free_pages * PG_SIZE);
     
    init_physical_pool(&kMemory->kernel_phy_pool,mm->kernel_phy_bitmap_base_addr,mm->kernel_bitmap_lenght,mm->kernel_phy_start,mm->kernel_free_pages * PG_SIZE);
}

uint32_t* malloc_page_kernel_memory(uint32_t count)
{
    uint32_t vaddr = get_pool_addr(&memory,PF_KERNEL_VIRTUAL,count);   
    
    if(vaddr == 0)
        return 0;
    
    uint32_t vaddr_ = vaddr;
    uint32_t paddr = 0;
    
    while(count--)
    {
        paddr = get_pool_addr(&memory,PF_KERNEL_PHYSICAL,1);
        
        if(paddr == 0)
            return 0;
    
        make_page_map(vaddr,paddr);
        vaddr += PG_SIZE;
    }
    
    return (uint32_t*)vaddr_; 
}

void init_user_memory(struct UserMemory* uMemory,struct MemoryMessage* mm)
{
    init_physical_pool(&uMemory->user_phy_pool,mm->user_phy_bitmap_base_addr,mm->user_bitmap_lenght,mm->user_phy_start,mm->user_free_pages * PG_SIZE);
}

uint32_t* malloc_page_user_memory()
{
    uint32_t paddr = get_pool_addr(&memory,PF_USER_PHYSICAL,1);
    if(paddr == 0)
        return 0;
    return (uint32_t*)paddr;
}

void init_memory()
{
    init_memory_message(&memory.memory_message);
    init_kernel_memory(&memory.kernel_memory,&memory.memory_message);
    init_user_memory(&memory.user_memory,&memory.memory_message);
//    printMemoryMessage(&memory.memoryMeesage);
    print_str((char*)"Init Memory Done!\n");
}

//这两段有争议，为什么可以再函数内返回局部变量？重点标注一下
//目的是获得一个指针，但是方式似乎有所不妥
//两天后解答上述问题：在ubuntu16.04下会出现段错误，因为现成的系统会有段保护措施，所以会造成段访问错误
uint32_t* get_vir_addr_PTE(uint32_t vaddr)
{
    uint32_t* idx = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return idx;
}

uint32_t* get_vir_addr_PDE(uint32_t vaddr)
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
void make_page_map(uint32_t vaddr,uint32_t paddr)
{
    uint32_t* pde = get_vir_addr_PDE(vaddr);
    uint32_t* pte = get_vir_addr_PTE(vaddr);
    
    if(*pde & 0x00000001)
    {
//        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1); 
    }
    else
    {
        uint32_t pdePaddr = get_pool_addr(&memory,PF_KERNEL_PHYSICAL,1);
        *pde = (pdePaddr | PG_US_U | PG_RW_W | PG_P_1);
        
        memset((void*)((uint32_t)pte & 0xfffff000),'\0',PG_SIZE); 
        ASSERT(!(*pte & 0x00000001));
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}




