#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "memory.h"
#include "process.h"
#include "sync.h"

struct task_struct* main_thread;    /*主线程PCB*/
struct task_struct* idle_thread;    /*CPU空闲时占用资源*/
struct list thread_ready_list;	    /*就绪队列*/
struct list thread_all_list;	    /*所有任务队列*/
struct lock pid_lock;		        /*分配pid锁*/
static struct list_elem* thread_tag;/*用于保存队列中的线程结点*/

extern void switch_to(struct task_struct* cur, struct task_struct* next);

/* 获取当前线程pcb指针 */
struct task_struct* running_thread() 
{
    uint32_t esp; 
    asm ("mov %%esp, %0" : "=g" (esp));
    return (struct task_struct*)(esp & 0xfffff000);
}

/*起始时，使用这个函数去执行线程函数*/
/*将arg，function以及调用函数返回地址压入栈中,之后使用ret弹出，执行{}中的函数*/
static void kernel_thread(thread_func* function, void* func_arg) 
{
    intr_enable();
    function(func_arg); 
}

/* 分配pid */
static pid_t allocate_pid(void) 
{
    static pid_t next_pid = 0;
    lock_acquire(&pid_lock);
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}

/*初始化线程栈thread_stack,将待执行的函数和参数放到thread_stack中相应的位置*/
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) 
{
    /*在起始处为中断栈预留空间，中断栈大小固定*/
    pthread->self_kstack -= sizeof(struct intr_stack);

    /*为内核栈预留空间*/
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;/*在内核栈的位置进行一次强制的转换*/
    kthread_stack->eip = kernel_thread; /*为内核栈填充相应的数据*/
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

/* 初始化线程基本信息 */
void init_thread(struct task_struct* pthread, char* name, int prio) 
{
    memset(pthread, 0, sizeof(*pthread));
    pthread->pid = allocate_pid();
    strcpy(pthread->name, name);

    if (pthread == main_thread) 
        pthread->status = TASK_RUNNING;
    else 
        pthread->status = TASK_READY;
    
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);/*内核栈顶*/
    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;
    pthread->stack_magic = (uint32_t)0x19970630;	  // 自定义的魔数
}

/*创建一个thread*/
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg) 
{
    /*为线程分配一块PCB*/
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);/*初始化内核栈与中断栈*/
    thread_create(thread, function, func_arg);/*初始化PCB中的其他内容*/
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);

    return thread;
}

/* 将kernel中的main函数完善为主线程 */
static void make_main_thread(void) 
{
/* 因为main线程早已运行,咱们在loader.S中进入内核时的mov esp,0xc009f000,
就是为其预留了tcb,地址为0xc009e000,因此不需要通过get_kernel_page另分配一页*/
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

/* 实现任务调度 */
void schedule() 
{
    ASSERT(intr_get_status() == INTR_OFF);

    struct task_struct* cur = running_thread(); 
    if (cur->status == TASK_RUNNING) 
    { 
        /*若当前线程只是因为时间片到期了，将这个PCB重新加入list中*/
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);
        cur->ticks = cur->priority; /*重置时间片*/
        cur->status = TASK_READY;
    } 
    else 
    { 
      /* 若此线程需要某事件发生后才能继续上cpu运行,
      不需要将其加入队列,因为当前线程不在就绪队列中。*/
    }
    
    if(list_empty(&thread_ready_list))
        thread_unblock(idle_thread);

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;	  
    /*从ready list中获取一个新的PCB，将其调度*/
    thread_tag = list_pop(&thread_ready_list);   
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;
    process_activate(next);/*激活新线程的页目录表*/
    switch_to(cur, next);
}

/* 当前线程将自己阻塞,标志其状态为stat. */
void thread_block(enum task_status stat) 
{
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || (stat == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    struct task_struct* cur_thread = running_thread();
    cur_thread->status = stat; /*置其状态为stat*/ 
    schedule();		      /*切换下一个线程*/
    intr_set_status(old_status);
}

/* 将线程pthread解除阻塞 */
void thread_unblock(struct task_struct* pthread) 
{
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) || (pthread->status == TASK_HANGING)));
    if(pthread->status != TASK_READY) 
    {
        ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
        if (elem_find(&thread_ready_list, &pthread->general_tag)) 
	        PANIC("thread_unblock: blocked thread in ready_list\n");

        /*将当前线程重新上调度表*/
        list_push(&thread_ready_list, &pthread->general_tag);
        pthread->status = TASK_READY;
    } 
    intr_set_status(old_status);
}

static void idle(void* arg)
{
    while(1)
    {
        thread_block(TASK_BLOCKED);
        /*使系统挂起，一直到接收到下一次中断，所以一定要保持开中断*/
        asm volatile("sti;hlt":::"memory");
    }
}

/* 初始化线程环境 */
void thread_init(void) 
{
    print_str((char*)"thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    lock_init(&pid_lock);
    make_main_thread(); /*将主线程在后端也保存为task_struct*/
    idle_thread = thread_start((char*)"idle",10,idle,NULL);/*创建idle线程*/
    print_str((char*)"thread_init done\n");
}


void thread_yield(void)
{
    struct task_struct* cur_thread = running_thread();
    enum intr_status old_status = intr_disable();
    list_append(&thread_ready_list,&cur_thread->general_tag);
    cur_thread->status = TASK_READY;
    schedule();
    intr_set_status(old_status);
}

