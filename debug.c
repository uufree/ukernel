#include "debug.h"
#include "print.h"
#include "interrupt.h"

/* 打印文件名,行号,函数名,条件并使程序悬停 */
void panic_spin(char* filename,	       \
	        int line,	       \
		const char* func,      \
		const char* condition) \
{
   interDisable();	// 因为有时候会单独调用panic_spin,所以在此处关中断。
   printStr("\n\n\n!!!!! error !!!!!\n");
   printStr("filename:");printStr(filename);printStr("\n");
   printStr("line:0x");printInt(line);printStr("\n");
   printStr("function:");printStr((char*)func);printStr("\n");
   printStr("condition:");printStr((char*)condition);printStr("\n");
   while(1);
}
