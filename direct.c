/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "links.h"
#include "strsubs.h"
#include "line.h"
#include "tokens.h"
#include "direct.h"
#include "getval.h"

LinkT *EmptyLink = 0;
DefInfoT *deflist = 0;
extern int skipflag;
extern int skiplevel;
extern FILE *outfile;
extern int commentflag1;

int IsMacro(LinkT *list)
{
    if (!list) return 0;
    if (strcmp(list->str, "#")) return 0;
    list = list->next;
    if (!list) return 0;
    if (strcmp(list->str, "define")) return 0;
    list = list->next;
    if (!list) return 0;
    if (list->str[0] != ' ' && list->str[0] != '\t') return 0;
    list = list->next;
    if (!list) return 0;
    list = list->next;
    if (!list) return 0;
    if (strcmp(list->str, "(")) return 0;
    return 1;
}

void AddToDefList(LinkT *tokenlist, int type)
{
    DefInfoT *definfo = (DefInfoT *)LinkCreate("dummy", sizeof(DefInfoT));
    definfo->tokenlist = DuplicateTokenList(tokenlist);
    deflist = (DefInfoT *)ListAppend((LinkT *)deflist, (LinkT *)definfo);
    definfo->type = type;
}

DefInfoT *DestroyDefList(DefInfoT *deflist)
{
    DefInfoT *definfo = deflist;

    while (definfo)
    {
	ListDestroy(definfo->tokenlist);
	definfo = definfo->next;
    }
    ListDestroy((LinkT *)deflist);

    return 0;
}

DefInfoT *AddToMacroList(char *nstr, LinkT *tokenlist, DefInfoT *macrolist)
{
    //DefInfoT *definfo = (DefInfoT *)LinkCreate("dummy", sizeof(DefInfoT));
    DefInfoT *definfo = (DefInfoT *)LinkCreate(nstr, sizeof(DefInfoT));
    definfo->tokenlist = tokenlist;
    definfo->type = DEFTYPE_MACRO;
    macrolist = (DefInfoT *)ListAppend((LinkT *)macrolist, (LinkT *)definfo);
    return macrolist;
}

void PrintDefines(void)
{
    int i = 0;
    int j;
    LinkT *list;
    DefInfoT *definfo = deflist;

    printf("Define List\n");
    while (definfo)
    {
        printf("%3d: ", i++);
	list = definfo->tokenlist;
	for (j = 0; list; j = 1)
	{
	    if (j) printf(" ");
	    printf(" <%s>", list->str);
	    list = list->next;
	}
	printf(", %d", definfo->type);
	printf("\n");
	definfo = definfo->next;
    }
}

void PrintMacroError(char *str)
{
    PrintFileLine();
    printf(" - Error in macro - %s\n", str);
    exit(1);
}

void PrintMacroList(DefInfoT *macrolist)
{
    LinkT *link;

    printf("PrintMacroList\n");
    while (macrolist)
    {
	link = macrolist->tokenlist;
	while (link)
	{
	    printf(" %s", link->str);
	    link = link->next;
	}
	printf("\n");
	macrolist = macrolist->next;
    }
}

void PrintListDelim(LinkT *list)
{
    printf("PrintListDelim: ");
    while (list && list->str)
    {
        printf("<%s>", list->str);
        list = list->next;
    }
    printf("\n");
}

LinkT *ExpandLine(LinkT *tokenlist)
{
    LinkT *head = tokenlist;
    LinkT *list = tokenlist;
    LinkT *prev = 0;
    LinkT *link;
    LinkT *next;
    LinkT *tail;
    char *str;
    DefInfoT *definfo;
    int loopcount = 0;
    int parencount;

    // Skip first token if commentflag1 is set
    if (commentflag1 && list)
    {
        prev = list;
        list = list->next;
    }

    while (list)
    {
	if (++loopcount >= 1000)
            FatalError("Infinite loop in macro expansion", 0);

	str = list->str;
        // Check for comment
        if (str[0] == '/')
        {
            // Exit loop if // found
            if (str[1] == '/')
                break;
            // Continue loop if /* found
            if (str[1] == '*')
            {
                prev = list;
                list = list->next;
                continue;
            }
        }
	definfo = FindDefine(str);

	if (definfo && definfo->type == DEFTYPE_DEFINE)
	{
	    link = definfo->tokenlist;
            if (link) link = link->next;
            if (link) link = link->next;
	    if (link == 0 || (link->str[0] == 0 && link->next == 0))
	    {
	        next = list->next;

		if (prev)
		    prev->next = next;
		else
		    head = next;

	    }
	    else
	    {
	        next = DuplicateTokenList(link);

		if (prev)
		    prev->next = next;
		else
		    head = next;

		tail = ListTail(next);
		tail->next = list->next;
	    }
	    LinkDestroy(list);
	    list = next;
	}
        // TODO: Need to clean up macro handling
	else if (definfo && definfo->type == DEFTYPE_MACRO)
	{
	    DefInfoT *macrolist = 0;
	    LinkT *inlist;
	    LinkT *maclist;
	    LinkT *inprev;

	    link = definfo->tokenlist;
	    // Extract the parameters
	    inlist = list->next;
	    maclist = definfo->tokenlist;
            if (maclist) maclist = maclist->next;
	    if (inlist->str[0] != '(') PrintMacroError("Expected \"(\"");
	    if (maclist->str[0] != '(') PrintMacroError("Expected \"(\"");
	    while (1)
	    {
	        inlist = inlist->next;
	        maclist= maclist->next;
	        if (!inlist || !maclist) PrintMacroError("Encountered end of line");
	        macrolist = AddToMacroList(maclist->str, inlist, macrolist);
	        maclist = maclist->next;
	        if (!maclist) PrintMacroError("Encountered end of line");
		if (maclist->str[0] != ',' && maclist->str[0] != ')') PrintMacroError("Unexpected character");
		inprev = 0;
		parencount = 0;
		while (1)
		{
		    if (inlist->str[0] == '(' || inlist->str[0] == '[')
		        parencount++;
		    else if (inlist->str[0] == ')' || inlist->str[0] == ']')
		        parencount--;
		    if (parencount <= 0 && inlist->str[0] == ',') break;
		    if (parencount <  0 && inlist->str[0] == ')') break;
		    inprev = inlist;
	            inlist = inlist->next;
	            if (!inlist) PrintMacroError("Encountered end of line");
		}
		if (inprev == 0)
		    macrolist->tokenlist = 0;
		else
		{
		    inprev->next = 0;
		}
		if (strcmp(inlist->str, maclist->str) != 0) PrintMacroError("Number of parameters doesn't match");
		if (inlist->str[0] == ')') break;
	    }
	    maclist = maclist->next;
            if (!maclist) PrintMacroError("Encountered end of line");
	    //Expand the rest of the macro and copy to the input line
	    list = 0;
	    inprev = prev;
	    while (maclist)
	    {
		// Check macrolist for token
		definfo = (DefInfoT *)ListFindLinkString((LinkT *)macrolist, maclist->str);
	        if (definfo)
	        {
	            link = definfo->tokenlist;

		    if (link && link->str[0])
	            {
	                link = DuplicateTokenList(link);

                        if (list == 0) list = link;
		        if (inprev)
		            inprev->next = link;
		        else
		            head = link;
		        link = ListTail(link);
	            }
		}
		else
		{
		    link = LinkCreate(maclist->str, sizeof(LinkT));
                    if (list == 0) list = link;
		    if (inprev)
		        inprev->next = link;
		    else
		        head = link;
		}
		inprev = link;
		maclist = maclist->next;
	    }

            inlist = inlist->next;
	    if (inprev)
	        inprev->next = inlist;
	    else
	        head = inlist;
            if (list == 0)
	    {
		list = inlist;
		prev = inprev;
	    }
	    else if (prev)
	        prev->next = list;

	    DestroyDefList(macrolist);
	}
	else
	{
	    prev = list;
	    list = list->next;
	}
    }
    return head;
}

int ProcessDirective(LinkT *tokenlist)
{
    char *buffer = "";
    char *buffer1 = "";
    LinkT *tokenlist0 = tokenlist;

    if (commentflag1 || tokenlist == 0) return 0;


    buffer = tokenlist->str;
    if (strcmp(buffer, "#") != 0) return 0;

    tokenlist = tokenlist->next;
    if (tokenlist == 0)
        FatalError("Empty preprocessor directive", 0);

    buffer = tokenlist->str;
    tokenlist = tokenlist->next;
    if (tokenlist)
    {
	if (tokenlist->str[0] != '\n') buffer1 = tokenlist->str;
	tokenlist = tokenlist->next;
	if (!tokenlist || tokenlist->str[0] == '\n')
	    tokenlist = EmptyLink;
    }

    if (!strcmp(buffer, "define"))
    {
	int type = DEFTYPE_DEFINE;
        if (IsMacro(tokenlist0)) type = DEFTYPE_MACRO;
        if (skipflag) return 0;
	if (1)
	{
            LinkT *prev = 0;
            LinkT *list = tokenlist;

	    if (*buffer1 == 0)
                FatalError("#define has no parameters", 0);

	    // Remove newline from tokenlist
	    while (list)
	    {
		if (list->str[0] == '\n')
		{
		    if (prev)
		        prev->next = 0;
		    else
		        tokenlist = EmptyLink;
		    break;
		}
		prev = list;
		list = list->next;
	    }
	    AddToDefList(tokenlist, type);
	}
        skipflag = SKIP_LINE;
	return 0;
    }

    if (strncmp(buffer, "pragma", 7) == 0) return 0;

    if (strncmp(buffer, "include", 8) == 0)
    {
        if (!skipflag)
	    ProcessIncludeDirective(tokenlist0);
        return 0;
    }

    if (!strcmp(buffer, "ifdef"))
    {
        if (skipflag)
            skiplevel++;
        else if (!FindDefine(tokenlist->str))
            skipflag = SKIP_MODE;
        else
            skipflag = SKIP_LINE;
    }
    else if (!strcmp(buffer, "ifndef"))
    {
        if (skipflag)
            skiplevel++;
        else if (FindDefine(tokenlist->str))
            skipflag = SKIP_MODE;
        else
            skipflag = SKIP_LINE;
    }
    else if (!strcmp(buffer, "if"))
    {
        if (skipflag)
            skiplevel++;
        else if (GetVal(&tokenlist, 0))
            skipflag = SKIP_LINE;
        else
            skipflag = SKIP_MODE;
    }
    else if (!strcmp(buffer, "else"))
    {
        if (!skiplevel)
        {
            if (skipflag)
                skipflag = SKIP_LINE;
            else
                skipflag = SKIP_MODE;
        }
    }
    else if (!strcmp(buffer, "endif"))
    {
        if (skiplevel)
            skiplevel--;
        else
            skipflag = SKIP_LINE;
    }

    return 0;
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
