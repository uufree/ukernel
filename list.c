/*************************************************************************
	> File Name: list.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 18时17分30秒
 ************************************************************************/

#include"list.h"
#include"interrupt.h"

void list_init(struct List* list)
{
    list->head.prev = (void*)0;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = (void*)0;
}

void list_destory(struct List* list)
{
    enum InterStatus status = inter_disable();
    struct ListNode* currentNode = list->head.next;
    while(currentNode != &list->tail)
    {
        list_remove(currentNode);
        currentNode = currentNode->next;
    }
    inter_set_status(status);
}

void list_insert_before(struct ListNode* currentNode,struct ListNode* node)
{
    enum InterStatus status = inter_disable();
    struct ListNode* elem = currentNode->prev;
    elem->next = node;
    node->prev = elem;
    node->next = currentNode;
    currentNode->prev = node;
    inter_set_status(status);
}

void list_insert_back(struct ListNode* currentNode,struct ListNode* node)
{
    list_insert_before(currentNode->next,node);
}

void list_push_back(struct List* list,struct ListNode* node)
{
    list_insert_before(&list->tail,node);
}

void list_push_front(struct List* list,struct ListNode* node)
{
    list_insert_back(&list->head,node);
}

void list_remove(struct ListNode* node)
{
    enum InterStatus status = inter_disable();
    node->prev->next = node->next;
    node->next->prev = node->prev;
    inter_set_status(status);
}

struct ListNode* list_pop_front(struct List* list)
{
    struct ListNode* front = list->head.next;
    list_remove(list->head.next);
    return front;
}

struct ListNode* list_pop_back(struct List* list)
{
    struct ListNode* back = list->tail.prev;
    list_remove(list->tail.prev);
    return back;
}

uint32_t list_length(struct List* list)
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

uint32_t list_empty(struct List* list)
{
    return !(list->head.next == &list->tail ? 1 : 0);
}

uint32_t list_find(struct List* list,struct ListNode* node)
{
     struct ListNode* currentNode = list->head.next;
     while(currentNode != &list->tail)
     {
         if(currentNode == node)
             return 1;
         currentNode = currentNode->next;
     }
     return 0;
}


