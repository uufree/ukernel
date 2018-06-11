#include "debug.h"
#include "print.h"
#include "interrupt.h"

/* 打印文件名,行号,函数名,条件并使程序悬停 */
void panic_spin(char* filename,int line,const char* func,const char* condition) {
    intr_disable();	
    print_str((char*)"\n\n\n!!!!! error !!!!!\n");
    print_str((char*)"filename:");print_str(filename);print_str((char*)"\n");
    print_str((char*)"line:0x");print_int(line);print_str((char*)"\n");
    print_str((char*)"function:");print_str((char*)func);print_str((char*)"\n");
    print_str((char*)"condition:");print_str((char*)condition);print_str((char*)"\n");
    while(1);
}
