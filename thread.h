/*************************************************************************
	> File Name: thread.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月04日 星期一 15时11分33秒
 ************************************************************************/

#ifndef _THREAD_H
#define _THREAD_H

#include"stdint.h"

typedef void threadFunc(void*);

enum TaskStatus
{
    TASK_RUNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};


#endif
