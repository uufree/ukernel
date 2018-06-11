#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"

static struct lock console_lock;

void console_init() 
{lock_init(&console_lock);}

void console_acquire() 
{lock_acquire(&console_lock);}

void console_release() 
{lock_release(&console_lock);}

void console_print_str(char* str) 
{
    console_acquire(); 
    print_str(str); 
    console_release();
}

void console_print_char(uint8_t char_asci) 
{
    console_acquire(); 
    print_char(char_asci); 
    console_release();
}

void console_print_int(uint32_t num) 
{
    console_acquire(); 
    print_int(num); 
    console_release();
}

