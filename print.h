#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void print_char(uint8_t char_asci);
void print_str(char* message);
void print_int(uint32_t num);	 // 以16进制打印
void set_cursor(uint32_t cursor_pos);
void cls_screen(void);
#endif

