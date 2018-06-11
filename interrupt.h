#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"

/*定义中断处理*/
typedef void* intr_handler;

/*完成所有中断的初始化操作*/
void idt_init(void);

/* 定义中断的两种状态:
 * INTR_OFF值为0,表示关中断,
 * INTR_ON值为1,表示开中断 
 */

enum intr_status 
{		
    INTR_OFF,			 /*中断关闭*/
    INTR_ON		         /*中断打开*/
};

/*获取当前中断状态*/
enum intr_status intr_get_status(void);

/*设置当前中断状态*/
enum intr_status intr_set_status (enum intr_status);

/*开中断*/
enum intr_status intr_enable (void);

/*关闭中断*/
enum intr_status intr_disable (void);

/*向某个中断号注册中断*/
void register_handler(uint8_t vector_no, intr_handler function);

#endif
