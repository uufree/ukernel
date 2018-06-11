#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64

/*环形队列，应对kernel中出现的生产者消费者问题*/
struct ioqueue 
{
    struct lock lock;
    /*生产者，buf满时睡眠，否则生产数据*/
    struct task_struct* producer;
    /*消费者，buf不为空时消费数据，否则睡眠*/
    struct task_struct* consumer;
    char buf[bufsize];		/*缓冲区*/
    int32_t head;			/*队首,数据往队首处写入*/
    int32_t tail;			/*队尾,数据从队尾处读出*/
};

void ioqueue_init(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char byte);

#endif
