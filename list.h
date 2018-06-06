/*************************************************************************
	> File Name: list.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时17分03秒
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H

#include"stdint.h"

#define offset(structType,member) (int)(&((structType*)0)->member)
#define elem2entry(structType,member,elemPtr) \
    (structType*)((int)elemPtr - offset(structType,member))

struct ListNode
{
    struct ListNode* prev;
    struct ListNode* next;
};

struct List
{
    struct ListNode head;
    struct ListNode tail;
};

//List OPS
void list_init(struct List* list);
void list_destory(struct List* list);
void list_insert_before(struct ListNode* current,struct ListNode* newNode);
void list_insert_back(struct ListNode* current,struct ListNode* newNode);
void list_push_back(struct List* list,struct ListNode* node);
void list_push_front(struct List* list,struct ListNode* node);
struct ListNode* list_pop_front(struct List* list);
struct ListNode* list_pop_back(struct List* list);
void list_remove(struct ListNode* node);
uint32_t list_length(struct List* list);
uint32_t list_empty(struct List* list);
uint32_t list_find(struct List* list,struct ListNode* node);


#endif
