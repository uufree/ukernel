#include "process.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"    
#include "list.h"    
#include "tss.h"    
#include "interrupt.h"
#include "string.h"
#include "console.h"

extern void intr_exit(void);

/* 构建用户进程初始上下文信息 */
void start_process(void* filename_) 
{
    void* function = filename_;
    struct task_struct* cur = running_thread();/*获取当前进程*/
    cur->self_kstack += sizeof(struct thread_stack);/*保存当前进程的上下文*/
    /*构建当前进程的中断栈*/
    struct intr_stack* proc_stack = (struct intr_stack*)cur->self_kstack;	 
    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;		
    /*根据需求选择用户段选择子（特权级变化）*/
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function;	 // 待执行的用户程序地址
    proc_stack->cs = SELECTOR_U_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void*)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);/*构建用户栈*/
    proc_stack->ss = SELECTOR_U_DATA; 
    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (proc_stack) : "memory");
}

/*在内核空间中激活页目录表*/
void page_dir_activate(struct task_struct* p_thread) 
{
    /*内核页目录表的物理位置*/
    uint32_t pagedir_phy_addr = 0x100000;
    
    if(p_thread->pgdir != NULL)/*用户页目录表的物理位置*/
        pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);

    /*修改cr3寄存器*/
    asm volatile ("movl %0, %%cr3" : : "r" (pagedir_phy_addr) : "memory");
}

/*击活线程或进程的页表,并且更新tss中的esp0为进程的特权级0的栈*/
void process_activate(struct task_struct* p_thread) 
{
    ASSERT(p_thread != NULL);
    page_dir_activate(p_thread);/*更新用户OR内核的页目录表*/

    /*内核自身的特权级就是0，更新时无需更新特权级栈*/
    if(p_thread->pgdir) 
        update_tss_esp(p_thread);/*更新进程的tss中的特权级为0的栈*/
}

/* 创建页目录表,将当前页表的表示内核空间的pde复制,
 * 成功则返回页目录的虚拟地址,否则返回-1 */
uint32_t* create_page_dir(void) 
{
    uint32_t* page_dir_vaddr = get_kernel_pages(1);/*为进程申请页目录表*/
    if (page_dir_vaddr == NULL) 
    {
        console_print_str((char*)"create_page_dir: get_kernel_page failed!");
        return NULL;
    }
    
    /*将内核页目录表的769~1022项复制到用户进程页目录表中相同的位置*/
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300*4), (uint32_t*)(0xfffff000+0x300*4), 1024);
    /*获取新分配的用户页目录表中的物理位置*/
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
    /*调整1023项为自身的位置*/
    page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    return page_dir_vaddr;
}

/* 创建用户进程虚拟地址位图 */
void create_user_vaddr_bitmap(struct task_struct* user_prog) 
{
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8 , PG_SIZE);/*计算维护用户虚拟地址需要的位图空间*/
    user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);/*在内核中获取count块page用来维护虚拟地址空间*/
    user_prog->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

/* 创建用户进程 */
void process_execute(void* filename, char* name) 
{ 
    struct task_struct* thread = get_kernel_pages(1);/*为PCB申请一块位置*/
    init_thread(thread, name, default_prio);/*在内核中初始化内核栈*/ 
    create_user_vaddr_bitmap(thread);/*在内核中创建维护用户虚拟地址的位图空间*/
    thread_create(thread, start_process, filename);
    thread->pgdir = create_page_dir();/*创建用户页目录表*/
    block_desc_init(thread->u_block_desc);/*初始化用户态下的内存分配*/
    
    /*将创建的PCB加入用户列表*/
    enum intr_status old_status = intr_disable();
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    intr_set_status(old_status);
}

