/*************************************************************************
	> File Name: list.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月18日 星期一 19时31分32秒
 ************************************************************************/

#ifndef _LIST_H
#define _LIST_H

//内核开发中通用的数据结构

#include"stdint.h"

struct ListNode
{
    struct ListNode* _prev;
    struct ListNode* _next;
};

struct List
{
    struct ListNode _head;
    struct ListNode _tail;
};

typedef bool (function)(struct ListNode* node,int args);

//List OPS
void listInit(struct List* list);
void listDestory(struct List* list);
void listInsertBefore(struct ListNode* current,struct ListNode* newNode);
void listInsertBack(struct ListNode* current,struct ListNode* newNode);
void listPushBack(struct List* list,struct ListNode* node);
void listPushFront(struct List* list,struct ListNode* node);
struct ListNode* listPopFront(struct List* list);
struct ListNode* listPopBack(struct List* list);
void listRemove(struct ListNode* node);
uint32_t listLength(struct List* list);
bool listEmpty(struct List* list);
bool listFind(struct List* list,struct ListNode* node);
struct ListNode* listTraversal(struct List* list,function func,int args);

#endif
