/*************************************************************************
	> File Name: thread.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月04日 星期一 15时11分33秒
 ************************************************************************/

#ifndef _THREAD_H
#define _THREAD_H

#include"stdint.h"
#include"list.h"

typedef void ThreadFunction(void*);

#define TASK_THREAD_SIZE 4096

enum TaskStatus
{
    TASK_RUNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

struct InterruptStack
{
    uint32_t vecNo;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t espDirty;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t errCode;
    uint32_t (*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

struct ThreadStack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    void(*eip)(ThreadFunction* func,void* funcArgs);
    
//仅第一次使用    
    void* unUsed;
    ThreadFunction* func;
    void* funcArgs;
};

struct TaskStruct
{
    uint32_t* kernelStack;
    enum TaskStatus status;
    char name[16];
    uint8_t priority;
    uint8_t ticks;
    uint32_t allTicks;
    struct ListNode tag;
    struct ListNode allTag;
    uint32_t* pageDir;
    uint32_t stackMagic;
};

struct TaskStruct* runingThread();
void createThread(struct TaskStruct* pthread,ThreadFunction func,void* funcArgs);
void initThread(struct TaskStruct* task,char* name,int prio);
struct TaskStruct* threadStart(char* name,int prio,ThreadFunction func,void* funcArgs);
void schedule();

#endif
