/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
#include <stdlib.h>
#include "links.h"
#include "strsubs.h"

int total_mem_alloc = 0;
int total_mem_freed = 0;

typedef struct mem_alloc_link_s {
    struct mem_alloc_link_s *next;
    void *ptr;
    int size;
} mem_alloc_link_t;

static mem_alloc_link_t *mem_alloc_list = 0;

LinkT *FindToken(char *str)
{
    LinkT *link = 0;
    mem_alloc_link_t *mlink = mem_alloc_list;

    if (str == 0) return link;

    // Locate the string point in the allocated memory list
    while (mlink)
    {
	if (mlink->ptr == str)
	{
	    // The next entry should be the token link
	    mlink = mlink->next;
	    if (mlink)
	    {
	        link = (LinkT *)(mlink->ptr);
	        if (link && link->str != str) link = 0;
	    }
	    break;
	}
	mlink = mlink->next;
    }
    return link;
}

static void mem_alloc_list_add(void *ptr, int size)
{
    mem_alloc_link_t *link = malloc(sizeof(mem_alloc_link_t));
    if (link == 0)
    {
	printf("mem_alloc_list_add: Out of memory\n");
	exit(1);
    }
    link->next = mem_alloc_list;
    link->ptr = ptr;
    link->size = size;
    mem_alloc_list = link;
}

static void mem_alloc_list_remove(void *ptr)
{
    mem_alloc_link_t *prev = 0;
    mem_alloc_link_t *link = mem_alloc_list;

    // Search list for ptr
    while (link)
    {
	if (link->ptr == ptr)
	{
	    if (prev)
	    {
		prev->next = link->next;
	    }
	    else
	    {
		mem_alloc_list = link->next;
	    }
	    total_mem_freed += link->size;
	    free(link);
	    break;
	}
	prev = link;
	link = link->next;
    }
}

void mem_free(void *ptr)
{
    mem_alloc_list_remove(ptr);
    free(ptr);
}

void *mem_alloc(int size)
{
    void *ptr;
    if (size < 0) size = 1;
    ptr = malloc(size);
    if (ptr == 0)
    {
	printf("mem_alloc: Out of memory\n");
        printf("Could not allocate %d bytes\n", size);
	printf("total_mem_alloc = %d\n", total_mem_alloc);
	exit(1);
    }
    mem_alloc_list_add(ptr, size);
    total_mem_alloc += size;
    return ptr;
}

void *mem_alloc_zero(int size)
{
    void *ptr;
    if (size < 0) size = 1;
    ptr = mem_alloc(size);
    memset(ptr, 0, size);
    return ptr;
}

char *str_alloc(int size)
{
    char *ptr = mem_alloc(size);
    *ptr = 0;
    return ptr;
}

LinkT *LinkCreate(char *str, int size)
{
    LinkT *link = mem_alloc_zero(size);
    link->next = 0;
    if (str == 0)
        link->str = 0;
    else
    {
	link->str = mem_alloc(strlen(str) + 1);
	strcpy(link->str, str);
    }
    return link;
}

void LinkDestroy(LinkT *link)
{
    if (link == 0) return;
    if (link->str != 0) mem_free(link->str);
    mem_free(link);
}

LinkT *ListDestroy(LinkT *list)
{
    LinkT *link;
    while (list)
    {
	link = list;
	list = list->next;
        if (link->str != 0) mem_free(link->str);
        mem_free(link);
    }
    return 0;
}

LinkT *ListTail(LinkT *list)
{
    if (list == 0) return 0;
    while (list->next) list = list->next;
    return list;
}

LinkT *ListAppend(LinkT *list, LinkT *link)
{
    LinkT *tail;
    if (list == 0) list = link;
    else
    {
        tail = ListTail(list);
        tail->next = link;
    }
    return list;
}

LinkT *ListInsertAfter(LinkT *list, LinkT *link)
{
    LinkT *next;
    if (list == 0) return link;
    if (link == 0) return list;
    next = list->next;
    list->next = link;
    link = ListTail(link);
    link->next = next;
    return list;
}

LinkT *ListFindPrev(LinkT *list, LinkT *link)
{
    if (list == 0) return 0;
    if (link == 0) return 0;
    if (list == link) return 0;
    while (list->next && list->next != link) list = list->next;
    return list;
}

LinkT *ListAppendLinkCreate(LinkT *list, char *str, int size)
{
    LinkT *link = LinkCreate(str, size);
    LinkT *tail;
    if (list == 0) return link;
    tail = ListTail(list);
    tail->next = link;
    return list;
}

LinkT *ListDuplicate(LinkT *list)
{
    LinkT *link;
    LinkT *tail = 0;
    LinkT *dupe = 0;

    while (list)
    {
        link = LinkCreate(list->str, sizeof(LinkT));
        if (tail) tail->next = link;
        else      dupe = link;
        tail = link;
        list = list->next;
    }

    return dupe;
}

void ListPrint(LinkT *list)
{
    int i = 0;
    while (list)
    {
	if (list->str)
	    printf("%d: %s\n", i++, list->str);
	else
	    printf("%d: NULL\n", i++);
	list = list->next;
    }
}

LinkT *ListFindLinkString(LinkT *list, char *str)
{
    if (str == 0) return 0;

    while (list)
    {
	if (strcmp(list->str, str) == 0) break;
	list = list->next;
    }

    return list;
}
/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
