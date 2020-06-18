/*
############################################################################
# Copyright (c) 2020 Dave Hein
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "links.h"
#include "direct.h"
#include "line.h"
#include "getval.h"

extern DefInfoT *deflist;

void FatalError(char *str, void *parm)
{
    PrintFileLine();
    printf(" - ");
    printf(str, parm);
    printf("\n");
    exit(1);
}

DefInfoT *FindDefine(char *str)
{
    LinkT *list;
    DefInfoT *definfo = deflist;

    while (definfo)
    {
        list = definfo->tokenlist;
        if (list && list->str && !strcmp(list->str, str))
            break;
        definfo = definfo->next;
    }

    return definfo;
}

void CheckEOL(LinkT *tokenlist)
{
    if (tokenlist) return;
    FatalError("encountered EOL", 0);
}

LinkT *SkipBlanksAndComments(LinkT *tokenlist)
{
    for (; tokenlist; tokenlist = tokenlist->next)
    {
        if (tokenlist->str[0] == ' ') continue;
        if (tokenlist->str[0] == '\t') continue;
        if (tokenlist->str[0] == '/' && tokenlist->str[1] == '/') continue;
        if (tokenlist->str[0] == '/' && tokenlist->str[1] == '*') continue;
        break;
    }
    return tokenlist;
}

// List of C operators and their precedences
//  *   /   %   +   -   <<   >>   <   <=   >   >=   ==   !=   &   ^   |   &&   ||
// 13  13  13  12  12   11   11  10   10  10   10    9    9   8   7   6    5    4

typedef int (*OpFuncT)(int, int);
typedef struct OpListS {
    char *opname;
    int precedence;
    OpFuncT opfunc;
} OpListT;

int MultFunc(int val1, int val2)    { return val1 *  val2; }
int DivFunc(int val1, int val2)     { return val1 /  val2; }
int ModFunc(int val1, int val2)     { return val1 %  val2; }
int PlusFunc(int val1, int val2)    { return val1 +  val2; }
int MinusFunc(int val1, int val2)   { return val1 -  val2; }
int ShiftLFunc(int val1, int val2)  { return val1 << val2; }
int ShiftRFunc(int val1, int val2)  { return val1 >> val2; }
int LessFunc(int val1, int val2)    { return val1 <  val2; }
int LessEqFunc(int val1, int val2)  { return val1 <= val2; }
int GreatFunc(int val1, int val2)   { return val1 >  val2; }
int GreatEqFunc(int val1, int val2) { return val1 >= val2; }
int EqualFunc(int val1, int val2)   { return val1 == val2; }
int NotEqFunc(int val1, int val2)   { return val1 != val2; }
int BitAndFunc(int val1, int val2)  { return val1 &  val2; }
int BitXorFunc(int val1, int val2)  { return val1 ^  val2; }
int BitOrFunc(int val1, int val2)   { return val1 |  val2; }

OpListT oplist[] = {
    {"*",  13, &MultFunc},
    {"/",  13, &DivFunc},
    {"%",  13, &ModFunc},
    {"+",  12, &PlusFunc},
    {"-",  12, &MinusFunc},
    {"<<", 11, &ShiftLFunc},
    {">>", 11, &ShiftRFunc},
    {"<",  10, &LessFunc},
    {"<=", 10, &LessEqFunc},
    {">",  10, &GreatFunc},
    {">=", 10, &GreatEqFunc},
    {"==",  9, &EqualFunc},
    {"!=",  9, &NotEqFunc},
    {"&",   8, &BitAndFunc},
    {"^",   7, &BitXorFunc},
    {"|",   6, &BitOrFunc},
    {0,     0, 0}};

int FindOp(char *str)
{
    int index;

    for (index = 0; oplist[index].opname; index++)
    {
        if (!strcmp(str, oplist[index].opname)) return index;
    }

    return -1;
}
    
int GetValue(LinkT **ptokenlist)
{
    int val;
    DefInfoT *definfo;
    LinkT *tokenlist = SkipBlanksAndComments(*ptokenlist);
    CheckEOL(tokenlist);
    if (!strcmp(tokenlist->str, "!"))
    {
        tokenlist = tokenlist->next;
        val = !GetVal(&tokenlist, 99);
    }
    else if (!strcmp(tokenlist->str, "("))
    {
        tokenlist = tokenlist->next;
        val = GetVal(&tokenlist, 0);
    }
    else if (!strcmp(tokenlist->str, "defined"))
    {
        tokenlist = SkipBlanksAndComments(tokenlist->next);
        CheckEOL(tokenlist);
        if (strcmp(tokenlist->str, "("))
            FatalError("no opening paren - %s", tokenlist->str);
        tokenlist = tokenlist->next;
        CheckEOL(tokenlist);
        val = (FindDefine(tokenlist->str) != 0);
        tokenlist = SkipBlanksAndComments(tokenlist->next);
        CheckEOL(tokenlist);
        if (strcmp(tokenlist->str, ")"))
            FatalError("no closing paren - %s", tokenlist->str);
        tokenlist = tokenlist->next;
    }
    else if (isdigit(tokenlist->str[0]))
    {
        val = atoi(tokenlist->str);
        tokenlist = tokenlist->next;
    }
    else if ((definfo = FindDefine(tokenlist->str)))
    {
        LinkT *list = definfo->tokenlist->next;
        val = GetVal(&list, 0);
        tokenlist = tokenlist->next;
    }
    else
        FatalError("undefined symbol - %s", tokenlist->str);

    *ptokenlist = tokenlist;
    return val;
}

// Process <value> <operator> <expression>
int GetVal(LinkT **ptokenlist, int prec)
{
    int val, index;
    LinkT *tokenlist = *ptokenlist;

    val = GetValue(&tokenlist);

    // Process <operator> <expression>
    while (tokenlist)
    {
        tokenlist = SkipBlanksAndComments(tokenlist);
        if (!tokenlist) break;
        index = FindOp(tokenlist->str);
        if (index >= 0)
        {
            int this_prec = oplist[index].precedence;;
            if (this_prec < prec) break;
            tokenlist = tokenlist->next;
            val = (oplist[index].opfunc)(val, GetVal(&tokenlist, this_prec+1));
        }
        else if (!strcmp(tokenlist->str, "||"))
        {
            int this_prec = 4;
            if (this_prec < prec) break;
            if (val)
            {
                *ptokenlist = tokenlist;
                return 1;
            }
            tokenlist = tokenlist->next;
            val = val || GetVal(&tokenlist, this_prec+1);
        }
        else if (!strcmp(tokenlist->str, "&&"))
        {
            int this_prec = 5;
            if (this_prec < prec) break;
            if (!val)
            {
                *ptokenlist = tokenlist;
                return 0;
            }
            tokenlist = tokenlist->next;
            val = val && GetVal(&tokenlist, this_prec+1);
        }
#if 0
        else if (!strcmp(tokenlist->str, "+"))
        {
            int this_prec = 12;
            if (this_prec < prec) break;
            tokenlist = tokenlist->next;
            val += GetVal(&tokenlist, this_prec+1);
        }
        else if (!strcmp(tokenlist->str, "-"))
        {
            int this_prec = 12;
            if (this_prec < prec) break;
            tokenlist = tokenlist->next;
            val -= GetVal(&tokenlist, this_prec+1);
        }
        else if (!strcmp(tokenlist->str, "*"))
        {
            int this_prec = 13;
            if (this_prec < prec) break;
            tokenlist = tokenlist->next;
            val *= GetVal(&tokenlist, this_prec+1);
        }
#endif
        else if (!strcmp(tokenlist->str, ")"))
        {
            *ptokenlist = tokenlist->next;
            return val;
        }
        else
            FatalError("invalid operator - %s", tokenlist->str);
    }

    *ptokenlist = tokenlist;
    return val;
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
