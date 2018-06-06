/*************************************************************************
	> File Name: init.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时14分39秒
 ************************************************************************/

#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"

/*负责初始化所有模块 */
void init_all() {
   print_str((char*)"init_all\n");
   idt_init();    // 初始化中断
   init_memory();	  // 初始化内存管理系统
   thread_init(); // 初始化线程相关结构
   timer_init();  // 初始化PIT
}

