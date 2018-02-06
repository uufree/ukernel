/*************************************************************************
	> File Name: thread.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月07日 星期四 14时57分10秒
 ************************************************************************/

#include"thread.h"
#include"string.h"
#include"global.h"
#include"memory.h"
#include"interrupt.h"
#include"list.h"

#define PG_SIZE 4096

struct TaskStruct* mainThread;
struct List threadList;
struct List allThreadList;
static struct ListNode* threadTag;

extern void switchTo(struct TaskStruct* currentThread,struct TaskStruct* nextThread);

struct TaskStruct* runingThread()
{
    uint32_t esp;
    asm("mov %%esp,%0" : "=g"(esp));
    return ((struct TaskStruct*)(esp & 0xfffff000));
}

static void kernelThread(ThreadFunction* func,void* funcArgs)
{
    interEnable();
    func(funcArgs);
}

void createThread(struct TaskStruct* pthread,ThreadFunction func,void* funcArgs)
{
    pthread->kernelStack -= sizeof(struct InterruptStack); 
    pthread->kernelStack -= sizeof(struct ThreadStack);
    
    struct ThreadStack* stack = (struct ThreadStack*)pthread->kernelStack;
    stack->eip = kernelThread;
    stack->func = func;
    stack->funcArgs = funcArgs;
    stack->ebp = stack->ebx = stack->edi = stack->esi = 0;
}

void initThread(struct TaskStruct* task,char* name,int prio)
{
    memset(task,'\0',TASK_THREAD_SIZE);
    strcpy(task->name,name);
    
    if(task == mainThread)
        task->status = TASK_RUNING;
    else
        task->status = TASK_READY;

    task->kernelStack = (uint32_t*)((uint32_t)task + TASK_THREAD_SIZE);
    task->priority = prio;
    task->ticks = prio;
    task->allTicks = 0;
    task->pageDir = NULL;
    task->stackMagic = 0x19970630;
}

struct TaskStruct* threadStart(char* name,int prio,ThreadFunction func,void* funcArgs)
{
    struct TaskStruct* task = (struct TaskStruct*)mallocPageInKernelMemory(1);
    initThread(task,name,prio);
    createThread(task,func,funcArgs);
    listPushBack(threadList,&task->tag);
    listPushBack(allThreadList,&task->allTag);

    asm volatile("movl %0,%%esp;pop %%ebp;pop %%ebx;pop %%edi;pop %%esi;ret": :"g"(task->kernelStack):"memory");
}

