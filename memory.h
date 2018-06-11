#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"

/*内存池标记,用于判断用哪个内存池*/
enum pool_flags 
{
    PF_KERNEL = 1,    /*内核内存池*/
    PF_USER = 2	      /*用户内存池*/
};

#define	 PG_P_1	  1	    /*页表项或页目录项存在属性位*/
#define	 PG_P_0	  0	    /*页表项或页目录项存在属性位*/
#define	 PG_RW_R  0	    /*R/W 属性位值, 读/执行*/
#define	 PG_RW_W  2	    /*R/W 属性位值, 读/写/执行*/
#define	 PG_US_S  0	    /*U/S 属性位值, 系统级*/
#define	 PG_US_U  4	    /*U/S 属性位值, 用户级*/

/*虚拟地址管理，用户 OR Kernel*/
struct virtual_addr 
{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

/*内存块*/
struct mem_block 
{
    struct list_elem free_elem;
};

/*内存块描述符*/
struct mem_block_desc 
{
    uint32_t block_size;		 /*内存块大小*/
    uint32_t blocks_per_arena;	 /*本arena中可容纳此mem_block的数量*/
    struct list free_list;	     /*目前可用的mem_block链表*/
};

#define DESC_CNT 7

/*保存物理内存的信息*/
extern struct pool kernel_pool, user_pool;

/*内存管理初始化*/
void mem_init(void);

/* 从内核物理内存池中申请pg_cnt页内存,成功则返回其虚拟地址,失败则返回NULL */
void* get_kernel_pages(uint32_t pg_cnt);

/* 分配pg_cnt个页空间,成功则返回起始虚拟地址,失败时返回NULL */
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);

void malloc_init(void);

/* 得到虚拟地址vaddr对应的pte指针*/
uint32_t* pte_ptr(uint32_t vaddr);

/* 得到虚拟地址vaddr对应的pde的指针 */
uint32_t* pde_ptr(uint32_t vaddr);

/* 得到虚拟地址映射到的物理地址 */
uint32_t addr_v2p(uint32_t vaddr);

/* 将地址vaddr与pf池中的物理地址关联,仅支持一页空间分配 */
void* get_a_page(enum pool_flags pf, uint32_t vaddr);

/*在用户空间中申请内存,并返回其虚拟地址 */
void* get_user_pages(uint32_t pg_cnt);

/*为初始化malloc做准备*/
void block_desc_init(struct mem_block_desc* desc_array);

/* 在堆中申请size字节内存 */
void* sys_malloc(uint32_t size);

/*释放以虚拟地址vaddr为起始的cnt个物理页框*/
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt);

/* 将物理地址pg_phy_addr回收到物理内存池 */
void pfree(uint32_t pg_phy_addr);

void sys_free(void* ptr);
#endif
