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
    list->_head._prev = NULL;
    list->_head._next = &list->_tail;
    list->_tail._prev = &list->_head;
    list->_tail._next = NULL;
}

void listDestory(struct List* list)
{
    enum InterStatus status = interGetStatus();
    
    struct ListNode* currentNode = list->_head._next;
    while(currentNode != &list->_tail)
    {
        listRemove(currentNode);
        currentNode = currentNode->_next;
    }

    interSetStatus(status);
}

void listInsertBefore(struct ListNode* currentNode,struct ListNode* node)
{
    enum InterStatus status = interGetStatus();
    struct ListNode* elem = currentNode->_prev;
    elem->_next = node;
    node->_prev = elem;
    node->_next = currentNode;
    currentNode->_prev = node;

    interSetStatus(status);
}

void listInsertBack(struct ListNode* currentNode,struct ListNode* node)
{
    listInsertBefore(currentNode->_next,node);
}

void listPushBack(struct List* list,struct ListNode* node)
{
    listInsertBefore(&list->_tail,node);
}

void listPushFront(struct List* list,struct ListNode* node)
{
    listInsertBack(&list->_head,node);
}

void listRemove(struct ListNode* node)
{
    enum InterStatus status = interGetStatus();
    node->_prev->_next = node->_next;
    node->_next->_prev = node->_prev;
    interSetStatus(status);
}

struct ListNode* listPopFront(struct List* list)
{
    struct ListNode* front = list->_head._next;
    listRemove(list->_head._next);
    return front;
}

struct ListNode* listPopBack(struct List* list)
{
    struct ListNode* back = list->_tail._prev;
    listRemove(list->_tail._prev);
    return back;
}

uint32_t listLength(struct List* list)
{
    uint32_t count = 0;
    struct ListNode* node = list->_head._next;

    while(node != &list->_tail)
    {
        ++count;
        node = node->_next;
    }

    return count;
}

bool listEmpty(struct List* list)
{
    return !(list->_head._next == &list->_tail ? true : false);
}

bool listFind(struct List* list,struct ListNode* node)
{
     struct ListNode* currentNode = list->_head._next;
     while(currentNode != &list->_tail)
     {
         if(currentNode == node)
             return true;
         currentNode = currentNode->_next;
     }
     return false;
}

struct ListNode* listTraversal(struct List* list,function func,int args)
{
    struct ListNode* currentNode = list->_head._next;
    if(listEmpty(list))
        return NULL;

    while(currentNode != &list->_tail)
    {
        if(func(currentNode,args))
            return currentNode;
        currentNode = currentNode->_next;
    }

    return NULL;
}

