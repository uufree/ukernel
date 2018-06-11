#include "memory.h"
#include "bitmap.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "print.h"
#include "string.h"
#include "sync.h"
#include "interrupt.h"

/***************  位图地址 ********************
 * 因为0xc009f000是内核主线程栈顶，0xc009e000是内核主线程的pcb.
 * 一个页框大小的位图可表示128M内存, 位图位置安排在地址0xc009a000,
 * 这样本系统最大支持4个页框的位图,即512M内存 */
#define MEM_BITMAP_BASE 0xc009a000
/*************************************/

/*获取页目录项的索引位置与页表项的索引位置*/
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/* 0xc0000000是内核从虚拟地址3G起. 0x100000意指跨过低端1M内存*/
#define K_HEAP_START 0xc0100000

/*内存池结构,生成两个实例用于管理内核内存池和用户内存池的物理地址*/
struct pool 
{
    struct bitmap pool_bitmap;	 /*本内存池用到的位图结构,用于管理物理内存*/
    uint32_t phy_addr_start;	 /*本内存池所管理物理内存的起始地址*/
    uint32_t pool_size;		     /*本内存池字节容量*/
    struct lock lock;		     /*申请内存时互斥*/
};

/*内存仓库arena元信息*/
struct arena 
{
    struct mem_block_desc* desc;	 /*此arena关联的mem_block_desc*/
    /*large为ture时,cnt表示的是页框数;否则cnt表示空闲mem_block数量*/
    uint32_t cnt;
    bool large;		   
};

/*内核内存块描述数组*/
struct mem_block_desc k_block_descs[DESC_CNT];
/*生成内核内存池与用户内存池*/
struct pool kernel_pool, user_pool; 
/*用于给内核分配虚拟地址*/
struct virtual_addr kernel_vaddr;	

/*在虚拟内存池中分配count个页*/
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt) 
{
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if (pf == PF_KERNEL) 
    {
        /*在内核内存池中分配虚拟内存*/
        bit_idx_start  = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1)
	        return NULL;
        
        /*新分配的虚拟地址连续可直接计算*/
        while(cnt < pg_cnt)
	        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    } 
    else 
    {	     
        /*在用户进程内存池中分配虚拟地址*/
        struct task_struct* cur = running_thread();
        bit_idx_start  = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1)
	        return NULL;

        while(cnt < pg_cnt)
	        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;

        /*防止用户虚拟地址分配超过限制*/
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));
    }
    return (void*)vaddr_start;
}

uint32_t* pte_ptr(uint32_t vaddr) 
{
    /*先访问页目录表，紧接着根据索引获取页表，最后根据索引获取页表项*/
    uint32_t* pte = (uint32_t*)(0xffc00000 + \
	    ((vaddr & 0xffc00000) >> 10) + \
	    PTE_IDX(vaddr) * 4);
    return pte;
}

uint32_t* pde_ptr(uint32_t vaddr) 
{
    /*思路相同，将页目录表回绕两次，第三次去页目录表中的索引*/
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

/*获取一个物理页地址*/
static void* palloc(struct pool* m_pool) 
{
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);    
    if(bit_idx == -1 ) 
        return NULL;
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
    uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}

/*将虚拟地址与物理地址进行映射*/
static void page_table_add(void* _vaddr, void* _page_phyaddr) 
{
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);

    /*首先判断页目录项中的页表是否存在*/
    if (*pde & 0x00000001) 
    {
        /*若页表项已经存在，说明出现了错误*/
        ASSERT(!(*pte & 0x00000001));
        /*页表项不存在的话，可以直接使用物理地址填充*/
        if (!(*pte & 0x00000001)) 
	        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);/*US=1,RW=1,P=1*/
    } 
    else 
    {	   
        /*页目录项不存在，需要先创建页目录项再创建页表项*/
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
        /*填充页目录项*/
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        
        /*将刚刚分配的整个页表清零*/
        memset((void*)((int)pte & 0xfffff000), 0, PG_SIZE); 
        ASSERT(!(*pte & 0x00000001));
        /*填充页表项*/
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); /*US=1,RW=1,P=1*/
    }
}

void* malloc_page(enum pool_flags pf, uint32_t pg_cnt) 
{
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);
    void* vaddr_start = vaddr_get(pf, pg_cnt);/*首先分配虚拟地址*/
    if(vaddr_start == NULL)
        return NULL;
    
    uint32_t vaddr = (uint32_t)vaddr_start;
    uint32_t cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    
    /*物理内存可以不连续分配*/
    while (cnt-- > 0) 
    {
        void* page_phyaddr = palloc(mem_pool);/*逐页分配物理地址*/

        if(page_phyaddr == NULL)
	        return NULL;

        page_table_add((void*)vaddr, page_phyaddr);/*将虚拟地址与物理地址映射*/ 
        vaddr += PG_SIZE;   /*递增虚拟地址,虚拟地址连续*/		 
    }

    return vaddr_start;
}

void* get_kernel_pages(uint32_t pg_cnt) 
{
    lock_acquire(&kernel_pool.lock);
    void* vaddr =  malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL) 
        memset(vaddr, 0, pg_cnt * PG_SIZE);/*将分配的内存清零*/
    lock_release(&kernel_pool.lock);
    return vaddr;
}

void* get_user_pages(uint32_t pg_cnt) 
{
    lock_acquire(&user_pool.lock);
    void* vaddr = malloc_page(PF_USER, pg_cnt);
    if(vaddr != NULL) 
        memset(vaddr, 0, pg_cnt * PG_SIZE);/*将分配的内存清零*/
    lock_release(&user_pool.lock);
    return vaddr;
}

void* get_a_page(enum pool_flags pf, uint32_t vaddr) 
{
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

   /* 先将虚拟地址对应的位图置1 */
    struct task_struct* cur = running_thread();
    int32_t bit_idx = -1;

/* 若当前是用户进程申请用户内存,就修改用户进程自己的虚拟地址位图 */
    if(cur->pgdir != NULL && pf == PF_USER) 
    {
        /*在用户虚拟内存池中分配虚拟内存*/
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);
    } 
    else if (cur->pgdir == NULL && pf == PF_KERNEL)
    {
        /*在内核虚拟内存池中分配虚拟内存*/
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
    } 
    else
      PANIC("get_a_page:not allow kernel alloc userspace or user alloc kernelspace by get_a_page");
    
    /*不管是内核线程还是用户线程，都是从统一的物理内存池中分配内存*/
    void* page_phyaddr = palloc(mem_pool);
    if (page_phyaddr == NULL) 
    {
        lock_release(&mem_pool->lock);
        return NULL;
    }

    page_table_add((void*)vaddr, page_phyaddr); 
    lock_release(&mem_pool->lock);
    return (void*)vaddr;
}

uint32_t addr_v2p(uint32_t vaddr) 
{
    uint32_t* pte = pte_ptr(vaddr);
/* (*pte)的值是页表所在的物理页框地址,
 * 去掉其低12位的页表项属性+虚拟地址vaddr的低12位 */
    return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

/*返回arena中第idx个内存块的地址*/
/*每个内存页的起始位置都存在一个arena，描述这个内存页中所维护内存的信息*/
static struct mem_block* arena2block(struct arena* a, uint32_t idx) 
{
    return (struct mem_block*)((uint32_t)a + sizeof(struct arena) + idx * a->desc->block_size);
}

/* 返回内存块b所在的arena地址 */
static struct arena* block2arena(struct mem_block* b) 
{
   return (struct arena*)((uint32_t)b & 0xfffff000);
}

void* sys_malloc(uint32_t size) 
{
    enum pool_flags PF;
    struct pool* mem_pool;
    uint32_t pool_size;
    struct mem_block_desc* descs;
    struct task_struct* cur_thread = running_thread();

    /*判断用哪个内存池*/
    if (cur_thread->pgdir == NULL) 
    {     
        /*若为内核线程*/
        PF = PF_KERNEL; 
        pool_size = kernel_pool.pool_size;
        mem_pool = &kernel_pool;
        descs = k_block_descs;
    } 
    else 
    {	
        /*使用用户线程*/
        PF = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }

   /* 若申请的内存不在内存池容量范围内则直接返回NULL */
    if (!(size > 0 && size < pool_size)) 
        return NULL;
   
    struct arena* a;
    struct mem_block* b;	
    lock_acquire(&mem_pool->lock);
    
    /*如果分配内存的大小超过1024字节，直接分配页框*/
    if (size > 1024) 
    {
        /*获取需要的页框数*/
        uint32_t page_cnt=DIV_ROUND_UP(size+sizeof(struct arena), PG_SIZE);    
        a = (struct arena*)malloc_page(PF, page_cnt);

        if (a != NULL) 
        {
	        memset(a, 0, page_cnt * PG_SIZE);/*将分配的内存清0*/	
            /*对于分配的大块页框,将desc置为NULL, cnt置为页框数,large置为true */
	        a->desc = NULL;
	        a->cnt = page_cnt;
	        a->large = true;
	        lock_release(&mem_pool->lock);
	        /*arena在页框中起始也占据一定的内存，跨越这个内存并返回剩余空间*/
	        return (void*)(a + 1);
        } 
        else 
        { 
	        lock_release(&mem_pool->lock);
	        return NULL; 
        }
    } 
    else 
    {    
        /*若申请的内存小于等于1024,可在各种规格的mem_block_desc中去适配*/
        uint8_t desc_idx;
      
        /*在列表中搜寻第一个比需求内存大的内存链表*/
        for(desc_idx = 0; desc_idx < DESC_CNT; desc_idx++)
	        if(size <= descs[desc_idx].block_size)   
	            break;
	 

        /*如果这个内存列表上没有空闲位置，新分配一个arena*/
        if (list_empty(&descs[desc_idx].free_list)) 
        {
	        a = (struct arena*)malloc_page(PF, 1);/*新分配一个内存页做arena*/ 
	        if(a == NULL) 
            {
	            lock_release(&mem_pool->lock);
	            return NULL;
	        }
	        memset(a, 0, PG_SIZE);

            /*更新这个新分配的arena的信息*/
	        a->desc = &descs[desc_idx];
	        a->large = false;
	        a->cnt = descs[desc_idx].blocks_per_arena;
	        uint32_t block_idx;
	        enum intr_status old_status = intr_disable();

	        /*将新分配的arena拆分成内存块,并添加到相应的的free_list中*/
	        for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena; block_idx++) 
            {
	            b = arena2block(a, block_idx);
	            ASSERT(!elem_find(&a->desc->free_list, &b->free_elem));
	            list_append(&a->desc->free_list, &b->free_elem);	
	        }
	        intr_set_status(old_status);
        }    

   /* 开始分配内存块 */
        b = elem2entry(struct mem_block, free_elem, list_pop(&(descs[desc_idx].free_list)));
        memset(b, 0, descs[desc_idx].block_size);

        a = block2arena(b);  // 获取内存块b所在的arena
        a->cnt--;		   // 将此arena中的空闲内存块数减1
        lock_release(&mem_pool->lock);
        return (void*)b;
    }
}

/*用户物理内存的分配高于内核物理地址*/
void pfree(uint32_t pg_phy_addr) 
{
    struct pool* mem_pool;
    uint32_t bit_idx = 0;
    if(pg_phy_addr >= user_pool.phy_addr_start) 
    {     
        /*用户物理内存池*/
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
    } 
    else 
    {	  
        /*内核物理内存池*/
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
    }
    bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0);	
}

/* 去掉页表中虚拟地址vaddr的映射,只去掉vaddr对应的pte */
static void page_table_pte_remove(uint32_t vaddr) 
{
    uint32_t* pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1;	/*仅仅将pte中的P位置0即可，表示物理页不存在*/
    asm volatile ("invlpg %0"::"m" (vaddr):"memory");   /*更新TLB*/
}

/* 在虚拟地址池中释放以_vaddr起始的连续pg_cnt个虚拟页地址 */
static void vaddr_remove(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt) 
{
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;

    if(pf == PF_KERNEL) 
    {  
        /*内核虚拟内存池*/
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        while(cnt < pg_cnt) 
	        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
    } 
    else 
    {  
        /*用户虚拟内存池*/
        struct task_struct* cur_thread = running_thread();
        bit_idx_start = (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PG_SIZE;
        while(cnt < pg_cnt)
	        bitmap_set(&cur_thread->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
    }    
}

void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt) 
{
    uint32_t pg_phy_addr;
    uint32_t vaddr = (int32_t)_vaddr, page_cnt = 0;
    ASSERT(pg_cnt >=1 && vaddr % PG_SIZE == 0); 
    pg_phy_addr = addr_v2p(vaddr);  /*获取虚拟地址vaddr对应的物理地址*/

    /*确保待释放的物理内存在低端1M+1k大小的页目录+1k大小的页表地址范围外*/
    ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= 0x102000);
   
    if(pg_phy_addr >= user_pool.phy_addr_start) 
    {   
        /*位于user_pool内存池*/
        vaddr -= PG_SIZE;
        while (page_cnt < pg_cnt) 
        {
	        vaddr += PG_SIZE;   /*获取虚拟*/
	        pg_phy_addr = addr_v2p(vaddr);  /*获取虚拟地址对应的物理地址*/
	        ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= user_pool.phy_addr_start);
	        pfree(pg_phy_addr);/*归还物理内存*/
	        page_table_pte_remove(vaddr);/*清除虚拟地址对应的pte*/
	        page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);/*清除虚拟地址中的位图*/
    } 
    else 
    {	     
        /*位于kernel_pool内存池,清理的流程与用户相似*/
        vaddr -= PG_SIZE;	      
        while (page_cnt < pg_cnt) 
        {
	        vaddr += PG_SIZE;
	        pg_phy_addr = addr_v2p(vaddr);
	        ASSERT((pg_phy_addr % PG_SIZE) == 0 && \
	            pg_phy_addr >= kernel_pool.phy_addr_start && \
	            pg_phy_addr < user_pool.phy_addr_start);
	
	        pfree(pg_phy_addr);
	        page_table_pte_remove(vaddr);
	        page_cnt++;
        }
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
}

/* 回收内存ptr */
void sys_free(void* ptr) 
{
    ASSERT(ptr != NULL);
    enum pool_flags PF;
    struct pool* mem_pool;

    /*判断是线程还是进程*/
    if (running_thread()->pgdir == NULL) 
    {
        /*内核线程*/
	    ASSERT((uint32_t)ptr >= K_HEAP_START);
	    PF = PF_KERNEL; 
	    mem_pool = &kernel_pool;
    } 
    else 
    {
        /*用户进程*/
	    PF = PF_USER;
	    mem_pool = &user_pool;
    }

    lock_acquire(&mem_pool->lock);   
    struct mem_block* b = ptr;
    struct arena* a = block2arena(b);/*将mem_block转化为arena*/
    ASSERT(a->large == 0 || a->large == 1);
    if (a->desc == NULL && a->large == true)/*如果是那种大于1024的内存*/ 
	    mfree_page(PF, a, a->cnt); 
    else 
    {				 
        /*小于等于1024的内存块*/
	    list_append(&a->desc->free_list, &b->free_elem);

	    /*判断这个arena中的小内存块如果全部都没有使用，释放arena*/
	    if (++a->cnt == a->desc->blocks_per_arena) 
        {
	        uint32_t block_idx;
	        for (block_idx = 0; block_idx < a->desc->blocks_per_arena; block_idx++) 
            {
	            struct mem_block*  b = arena2block(a, block_idx);
	            ASSERT(elem_find(&a->desc->free_list, &b->free_elem));
	            list_remove(&b->free_elem);
	        }
	        mfree_page(PF, a, 1); 
	    } 
    }    
    lock_release(&mem_pool->lock); 
}

/* 初始化内存池 */
static void mem_pool_init(uint32_t all_mem) 
{
    print_str((char*)"   mem_pool_init start\n");
    /*页表大小=页目录表+769~1022页表（1023指向页目录表自身)*/
    uint32_t page_table_size = PG_SIZE * 256;	
    uint32_t used_mem = page_table_size + 0x100000;	/*0x100000为低端1M内存*/
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages = free_mem / PG_SIZE;/*计算空闲的页数目*/
    uint16_t kernel_free_pages = all_free_pages / 4;/*user:kernel = 3:1*/
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    uint32_t kbm_length = kernel_free_pages / 8;    /*内核位图长度*/
    uint32_t ubm_length = user_free_pages / 8;  /*User的位图长度*/

    uint32_t kp_start = used_mem;/*内核的物理地址从低地址开始分配*/
    /*用户的物理内存地址从内核物理内存地址之后开始分配*/
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;	  

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start   = up_start;

    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    user_pool.pool_size	 = user_free_pages * PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len	  = ubm_length;

// 内核使用的最高地址是0xc009f000,这是主线程的栈地址.(内核的大小预计为70K左右)
// 32M内存占用的位图是2k.内核内存池的位图先定在MEM_BITMAP_BASE(0xc009a000)处.
    /*两个位图地址固定*/
    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
							       
    /*用户内存池的位图紧跟在内核内存池位图之后*/
    user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);
    
    print_str((char*)"      kernel_pool_bitmap_start:");print_int((int)kernel_pool.pool_bitmap.bits);
    print_str((char*)" kernel_pool_phy_addr_start:");print_int(kernel_pool.phy_addr_start);
    print_str((char*)"\n");
    print_str((char*)"      user_pool_bitmap_start:");print_int((int)user_pool.pool_bitmap.bits);
    print_str((char*)" user_pool_phy_addr_start:");print_int(user_pool.phy_addr_start);
    print_str((char*)"\n");

    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    lock_init(&kernel_pool.lock);
    lock_init(&user_pool.lock);

    /*下面初始化内核虚拟地址的位图,按实际物理内存大小生成数组*/
    /*用于维护内核堆的虚拟地址，长度的内核物理地址一样*/
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;  

    /*内核虚拟位图紧邻着上述的两个维护物理内存的位图存放*/
    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    print_str((char*)"   mem_pool_init done\n");
}

void block_desc_init(struct mem_block_desc* desc_array) 
{				   
    /*16，32，64,128，256，512，1024*/
    uint16_t block_size = 16;/*从大小16的内存块起始*/
    for(uint8_t desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) 
    {
        desc_array[desc_idx].block_size = block_size;
        desc_array[desc_idx].blocks_per_arena = (PG_SIZE - sizeof(struct arena)) / block_size;/*每个内存页减去struct arena的大小之后计算数量*/	  
        list_init(&desc_array[desc_idx].free_list);
        block_size *= 2;        
    }
}


void mem_init() 
{
    print_str("mem_init start\n");
    /*0xb00这个位置存放着所有的内存数量*/
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);	    /*初始化内存池*/
    block_desc_init(k_block_descs);     /*初始化内存数组*/
    print_str("mem_init done\n");
}
