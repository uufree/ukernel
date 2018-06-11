#include "sync.h"
#include "list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

/* 初始化信号量 */
void sema_init(struct semaphore* psema, uint8_t value) 
{
    psema->value = value;       
    list_init(&psema->waiters); 
}

/* 初始化锁plock */
void lock_init(struct lock* plock) 
{
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1);  
}

/* 信号量down操作 */
void sema_down(struct semaphore* psema) 
{
    enum intr_status old_status = intr_disable();
    while(psema->value == 0) 
    {	
        /*value为0,表示已经被别人持有*/
        ASSERT(!elem_find(&psema->waiters, &running_thread()->general_tag));
        /* 当前线程不应该已在信号量的waiters队列中 */
        if(elem_find(&psema->waiters, &running_thread()->general_tag)) 
	        PANIC("sema_down: thread blocked has been in waiters_list\n");
        /*若信号量的值等于0,则当前线程把自己加入该锁的等待队列,然后阻塞自己*/
        list_append(&psema->waiters, &running_thread()->general_tag); 
        thread_block(TASK_BLOCKED);    /*线程被阻塞在此处*/
    }
    
    /*若value为1或被唤醒后,会执行下面的代码,也就是获得了锁*/
    psema->value--;
    ASSERT(psema->value == 0);	    
    intr_set_status(old_status);
}

/* 信号量的up操作 */
void sema_up(struct semaphore* psema) 
{
    enum intr_status old_status = intr_disable();
    ASSERT(psema->value == 0);	    
    if (!list_empty(&psema->waiters)) 
    {
        /*从阻塞队列中唤醒一个请求锁的PCB*/
        struct task_struct* thread_blocked = elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }
    psema->value++;
    ASSERT(psema->value == 1);	    
    intr_set_status(old_status);
}

/* 获取锁plock */
void lock_acquire(struct lock* plock) 
{
    if (plock->holder != running_thread()) 
    { 
        /*其他线程尝试获取锁*/
        sema_down(&plock->semaphore);    /*阻塞在此处*/
        plock->holder = running_thread();
        ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    } 
    else 
        plock->holder_repeat_nr++;/*当前线程尝试获取锁，递增申请次数*/
}

/* 释放锁plock */
void lock_release(struct lock* plock) 
{
    ASSERT(plock->holder == running_thread());
    if(plock->holder_repeat_nr > 1) 
    {
        /*处理递归锁的情形*/
        plock->holder_repeat_nr--;
        return;
    }
    ASSERT(plock->holder_repeat_nr == 1);

    /*将锁还原至初始化状态*/
    plock->holder = NULL;	   
    plock->holder_repeat_nr = 0;
    sema_up(&plock->semaphore);	   
}

