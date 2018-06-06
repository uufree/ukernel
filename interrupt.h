/*************************************************************************
	> File Name: interrupt.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时15分39秒
 ************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "stdint.h"
typedef void* intr_handler;
void idt_init(void);

/* 定义中断的两种状态:
 * INTR_OFF值为0,表示关中断,
 * INTR_ON值为1,表示开中断 */
enum InterStatus {		 // 中断状态
    INTER_OFF,			 // 中断关闭
    INTER_ON		         // 中断打开
};

enum InterStatus inter_get_status(void);
enum InterStatus inter_set_status (enum InterStatus);
enum InterStatus inter_enable(void);
enum InterStatus inter_disable (void);
void register_handler(uint8_t vector_no, intr_handler function);

#endif
