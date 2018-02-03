/*************************************************************************
	> File Name: list.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年12月18日 星期一 19时31分36秒
 ************************************************************************/

#include"list.h"
#include"interrupt.h"

void listInit(struct List* list)
{
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

void listDestory(struct List* list)
{
    enum InterStatus status = interGetStatus();
    
    struct ListNode* currentNode = list->head.next;
    while(currentNode != &list->tail)
        listRemove(currentNode);

    interSetStatus(status);
}

void listInsertBefore(struct ListNode* currentNode,struct ListNode* node)
{
    enum InterStatus status = interGetStatus();
    struct ListNode* elem = currentNode->prev;
    elem->next = node;
    node->prev = elem;
    node->next = currentNode;
    currentNode->prev = node;

    interSetStatus(status);
}

void listInsertBack(struct ListNode* currentNode,struct ListNode* node)
{
    listInsertBefore(currentNode->next,node);
}

void listPushBack(struct List* list,struct ListNode* node)
{
    listInsertBefore(&list->tail,node);
}

void listPushFront(struct List* list,struct ListNode* node)
{
    listInsertBack(&list->head,node);
}

void listRemove(struct ListNode* node)
{
    enum InterStatus status = interGetStatus();
    node->prev->next = node->next;
    node->next->prev = node->prev;
    interSetStatus(status);
}

struct ListNode* listPopFront(struct List* list)
{
    struct ListNode* _front = list->head.next;
    listRemove(list->head.next);
    return _front;
}

struct ListNode* listPopBack(struct List* list)
{
    struct ListNode* _back = list->tail.prev;
    listRemove(list->tail.prev);
    return _back;
}

uint32_t listLength(struct List* list)
{
    uint32_t count = 0;
    struct ListNode* node = list->head.next;

    while(node != &list->tail)
    {
        ++count;
        node = node->next;
    }

    return count;
}

bool listEmpty(struct List* list)
{
    return list->head.next == &list->tail ? true : false;
}

bool listFind(struct List* list,struct ListNode* node)
{
     struct ListNode* currentNode = list->head.next;
     while(currentNode != &list->tail)
     {
         if(currentNode == node)
             return true;
         currentNode = currentNode->next;
     }
     return false;
}

struct ListNode* listTraversal(struct List* list,function func,int args)
{
    struct ListNode* currentNode = list->head.next;
    if(listEmpty(list))
        return NULL;

    while(currentNode != &list->tail)
    {
        if(func(currentNode,args))
            return currentNode;
        currentNode = currentNode->next;
    }

    return NULL;
}

