#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LinkS {
    struct LinkS *next;
    char *str;
} LinkT;

LinkT *LinkCreate(char *str, int size);
void LinkDestroy(LinkT *link);
LinkT *ListDestroy(LinkT *list);
LinkT *ListTail(LinkT *list);
LinkT *ListAppend(LinkT *list, LinkT *link);
LinkT *ListInsertAfter(LinkT *list, LinkT *link);
LinkT *ListFindPrev(LinkT *list, LinkT *link);
LinkT *ListRemoveLink(LinkT *list, LinkT *link);
LinkT *ListAppendLinkCreate(LinkT *list, char *str, int size);
void ListPrint(LinkT *list);
LinkT *ListRemoveLinkString(LinkT *list, char *str);
void *mem_alloc(int size);
void *mem_alloc_zero(int size);
char *str_alloc(int size);
char *AddToSymbolList(char *);
void RemoveFromSymbolList(char *str);
LinkT *FindToken(char *str);
void PrepExit(int errval);
LinkT *ListDuplicate(LinkT *list);
LinkT *ListFindLinkString(LinkT *list, char *str);
