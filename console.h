#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H

#include "stdint.h"

/*控制输出流互斥*/
void console_init(void);
void console_acquire(void);
void console_release(void);
void console_print_str(char* str);
void console_print_char(uint8_t char_asci);
void console_print_int(uint32_t num);

#endif

