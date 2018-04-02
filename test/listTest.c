/*************************************************************************
	> File Name: listTest.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年04月03日 星期二 03时44分16秒
 ************************************************************************/

#include<stdio.h>
#include"../list.h"
#include<malloc.h>

struct clock
{
    struct ListNode node;
    int number;
};

bool printNode(struct ListNode* node)
{
    if(node)
        printf("hello");
    return true;
}

int main(int argc,char** argv)
{
    struct List list;
    listInit(&list);
    printf("Construct List Size: %d\n",listLength(&list)); 
    struct clock* c0 = (struct clock*)malloc(sizeof(struct clock));
    c0->number = 0; 
    struct clock* c1 = (struct clock*)malloc(sizeof(struct clock));
    c1->number = 1;
    struct clock* c2 = (struct clock*)malloc(sizeof(struct clock));
    c2->number = 2;

    listPushFront(&list,&c0->node);
    listPushFront(&list,&c1->node);
    printf("List Size: %d\n",listLength(&list)); 
    listInsertBack(&c0->node,&c2->node);
    printf("List Size: %d\n",listLength(&list)); 
    printf("List Size: %d\n",listFind(&list,&c1->node)); 
    listDestory(&list);
    printf("Destory List Size: %d\n",listLength(&list)); 
    free(c0);
    return 0;
}

