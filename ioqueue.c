#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

/*初始化io队列ioq*/
void ioqueue_init(struct ioqueue* ioq) 
{
    lock_init(&ioq->lock);     
    ioq->producer = ioq->consumer = NULL;  
    ioq->head = ioq->tail = 0; 
}

/*返回pos在缓冲区中的下一个位置值，处理回绕情形*/
static int32_t next_pos(int32_t pos) 
{return (pos + 1) % bufsize;}

/*判断队列是否已满*/
bool ioq_full(struct ioqueue* ioq) 
{
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(ioq->head) == ioq->tail;
}

/*判断队列是否已空*/
static bool ioq_empty(struct ioqueue* ioq) 
{
    ASSERT(intr_get_status() == INTR_OFF);
    return ioq->head == ioq->tail;
}

/*使当前生产者或消费者在此缓冲区上等待*/
static void ioq_wait(struct task_struct** waiter) 
{
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

/* 唤醒waiter */
static void wakeup(struct task_struct** waiter) 
{
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter); 
    *waiter = NULL;
}

/*消费者从ioq队列中获取一个字符*/
char ioq_getchar(struct ioqueue* ioq) 
{
    ASSERT(intr_get_status() == INTR_OFF);
    
    /*如果消费队列为空，使消费者陷入睡眠*/
    while(ioq_empty(ioq)) 
    {
        lock_acquire(&ioq->lock);	 
        ioq_wait(&ioq->consumer);   /*没有数据的时候，会阻塞在这个位置*/
        lock_release(&ioq->lock);
    }
    
    /*执行到这块意味着消费者已经被唤醒*/
    char byte = ioq->buf[ioq->tail];	  
    ioq->tail = next_pos(ioq->tail);	

    /*buf中已经有了空闲的位置，唤醒生产者*/
    if(ioq->producer != NULL) 
        wakeup(&ioq->producer);		

    return byte; 
}

/*生产者往ioq队列中写入一个字符byte*/
void ioq_putchar(struct ioqueue* ioq, char byte) 
{
    ASSERT(intr_get_status() == INTR_OFF);

    /*如果消费队列已满，使生产者陷入睡眠*/
    while(ioq_full(ioq)) 
    {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }
    
    /*生产者已经被唤醒*/
    ioq->buf[ioq->head] = byte;      
    ioq->head = next_pos(ioq->head); 

    if(ioq->consumer != NULL)
        wakeup(&ioq->consumer);         
}

