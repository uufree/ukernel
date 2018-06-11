#include "timer.h"
#include "io.h"
#include "print.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

#define IRQ0_FREQUENCY	   100
#define INPUT_FREQUENCY	   1193180
#define COUNTER0_VALUE	   INPUT_FREQUENCY / IRQ0_FREQUENCY
#define CONTRER0_PORT	   0x40
#define COUNTER0_NO	       0
#define COUNTER_MODE	   2
#define READ_WRITE_LATCH   3
#define PIT_CONTROL_PORT   0x43

/*自内核中断启动以来的总共的滴答数*/
uint32_t ticks;         

/*初始化时钟中断*/
static void frequency_set(uint8_t counter_port,uint8_t counter_no,uint8_t rwl, uint8_t counter_mode,uint16_t counter_value) 
{
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

/* 时钟的中断处理函数 */
static void intr_timer_handler(void) 
{
    struct task_struct* cur_thread = running_thread();
    /*检测task_struct的内核栈是否溢出*/
    ASSERT(cur_thread->stack_magic == 0x19970630);  
    cur_thread->elapsed_ticks++;	  /*记录此线程占用的cpu时间滴答数*/
    ticks++;	  /*更新全局的cpu滴答总数*/

    if(cur_thread->ticks == 0) 	  /*如果当前线程的时间到了，更换下一个线程*/
        schedule(); 
    else 				  
        cur_thread->ticks--;    /*否则的话，递减当前线程的时钟计数*/
}

/* 初始化PIT8253 */
void timer_init() 
{
    print_str((char*)"timer_init start\n");
    /* 设置8253的定时周期,也就是发中断的周期 */
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    /*注册时钟中断*/
    register_handler(0x20,(void*)intr_timer_handler);
    print_str((char*)"timer_init done\n");
}

static void sleep_to_ticks(uint32_t sleep_ticks)
{
    uint32_t start_ticks = ticks;
    while(ticks - start_ticks < sleep_ticks)
        thread_yield();
}

void msleep(uint32_t mseconds)
{
    uint32_t sleep_ticks = mseconds / 10;/*1000ms / 100(频率)*/
    ASSERT(sleep_ticks > 0);
    sleep_to_ticks(sleep_ticks);
}
