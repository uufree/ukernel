#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "stdint.h"
typedef char* va_list;
#define va_start(ap, v) ap = (va_list)&v  // 把ap指向第一个固定参数v

/*对用户参数的函数调用栈进行操作*/
/*由于函数压栈是从右往左进行的，当前参数（ap）+4就是向右一个参数*/
#define va_arg(ap, t) *((t*)(ap += 4))	  // ap指向下一个参数并返回其值
#define va_end(ap) ap = NULL		  // 清除ap

uint32_t printf(const char* str, ...);
uint32_t printk(const char* str, ...);
uint32_t vsprintf(char* str, const char* format, va_list ap);
uint32_t sprintf(char* buf, const char* format, ...);

#endif
