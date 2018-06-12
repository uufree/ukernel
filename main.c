#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "process.h"
#include "syscall-init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "dir.h"
#include "fs.h"
#include "assert.h"
#include "shell.h"

#include "ide.h"

void init(void);

int main(void) 
{
    print_str("I am kernel\n");
    init_all();
    cls_screen();
    console_print_str("[uuchen@localhost /]$ ");
    thread_exit(running_thread(), true);
    return 0;
}

/*init进程*/
void init(void) 
{
    uint32_t ret_pid = fork();
    if(ret_pid) 
    {  
        int status;
        int child_pid;
        while(1) 
        {
	        child_pid = wait(&status);
	        printf("I`m init, My pid is 1, I recieve a child, It`s pid is %d, status is %d\n", child_pid, status);
        }
    } 
    else 
      my_shell();

   panic("init: should not be here");
}
