/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "strsubs.h"
#include "links.h"
#include "tokens.h"
#include "line.h"

extern FILE *outfile;
extern int commentflag;
extern int commentflag1;

static char *special = " \t*()/\"'=+-%,;<>&|^!~#{}[].:";
static char *specstr[] = {"->", "<<", ">>", "<", ">", "=", "++", "--", "+", "-",
        "&&", "||", "&", "|", "!", "^", "~", "%", "**", "*", "/", "##", 0};

void DebugPrintTokenList(LinkT *tokenlist)
{
    while (tokenlist)
    {
	printf("<%s>", tokenlist->str);
	tokenlist = tokenlist->next;
    }
    printf("\n");
}

LinkT *DuplicateTokenList(LinkT *tokenlist)
{
    LinkT *dupelist = 0;

    while (tokenlist)
    {
	AddToken(tokenlist->str, &dupelist);
	tokenlist = tokenlist->next;
    }

    return dupelist;
}

LinkT *InsertToken(char *str, LinkT **ptokenlist, LinkT *token)
{
    LinkT *prev = ListFindPrev(*ptokenlist, token);
    LinkT *link = LinkCreate(str, sizeof(LinkT));

    if (token == *ptokenlist)
    {
	*ptokenlist = link;
    }
    else if (prev && prev->next == token)
    {
	prev->next = link;
    }
    else
    {
	printf("InsertToken: Could not find token in list\n");
	exit(1);
    }

    link->next = token;

    return link;
}

LinkT *ReplaceToken(char *str, LinkT **ptokenlist, LinkT *token)
{
    LinkT *prev = ListFindPrev(*ptokenlist, token);
    LinkT *link = LinkCreate(str, sizeof(LinkT));

    if (token == *ptokenlist)
    {
	*ptokenlist = link;
    }
    else if (prev && prev->next == token)
    {
	prev->next = link;
    }
    else
    {
	printf("ReplaceToken: Could not find token in list\n");
	exit(1);
    }

    link->next = token->next;
    LinkDestroy(token);

    return link;
}

void RemoveToken(LinkT **ptokenlist, LinkT *token)
{
    LinkT *prev = ListFindPrev(*ptokenlist, token);

    if (token == *ptokenlist)
    {
	*ptokenlist = token->next;
    }
    else if (prev && prev->next == token)
    {
	prev->next = token->next;
    }
    else
    {
	printf("RemoveToken: Could not find token in list\n");
	exit(1);
    }

    LinkDestroy(token);
}

LinkT *AddToken(char *str, LinkT **ptokenlist)
{
    LinkT *link = LinkCreate(str, sizeof(LinkT));
    *ptokenlist = ListAppend(*ptokenlist, link);
    return link;
}

void AddTokenN(char *ptr1, char *ptr2, LinkT **ptokenlist)
{
    int val = *ptr2;
    *ptr2 = 0;
    AddToken(ptr1, ptokenlist);
    *ptr2 = val;
}

void getctokens(char *instr, LinkT **ptokenlist)
{
    int i;
    char *ptr;
    int saveval;

    while (*instr)
    {
	if (commentflag)
	{
	    ptr = instr;
	    while (*ptr)
	    {
	        if (ptr[0] == '*' && ptr[1] == '/') break;
		ptr++;
	    }
	    if (*ptr)
	    {
                saveval = ptr[2];
                ptr[2] = 0;
                AddToken(instr, ptokenlist);
                ptr[2] = saveval;
                instr = ptr + 2;
		commentflag = 0;
	        continue;
	    }
	    else
	    {
                AddToken(instr, ptokenlist);
		break;
	    }
	}

        // Get white space token
        ptr = SkipChars(instr, " \t");
        if (ptr != instr)
        {
            AddTokenN(instr, ptr, ptokenlist);
            instr = ptr;
        }
        if (*instr == 0) break;

        // Get non-white space token
        ptr = FindChar(special, *instr);

        if (*ptr == 0)
        {
            ptr = FindChars(instr, special) - 1;
	    if (ptr[1] == '.' && alldigits(instr, ptr))
                ptr = FindChars(ptr + 2, special) - 1;
        }
	else if (instr[0] == '.' && isdigit(instr[1]))
	{
            ptr = FindChars(instr + 1, special) - 1;
	}
        else if (*instr == '"' || *instr == '\'')
        {
            ptr = instr + 1;
            while (1)
            {
                ptr = FindChar(ptr, *instr);
                if (*ptr == 0) break;
                if (ptr[-1] != '\\') break;
                else if (ptr - 2 >= instr && ptr[-2] == '\\') break;
                ptr++;
            }
        }
        else if (instr[0] == '/' && instr[1] == '/')
        {
            ptr = instr + strlen(instr) - 1;
        }
        else if (instr[0] == '/' && instr[1] == '*')
        {
	    commentflag = 1;
	    continue;
        }
        else if ((i = SearchList(instr, specstr, 0)) != -1)
        {
            ptr = instr + strlen(specstr[i]);
            if (*ptr != '=') ptr--;
        }
        else
        {
            ptr = instr;
        }
        saveval = ptr[1];
	ptr[1] = 0;
        AddToken(instr, ptokenlist);
	ptr[1] = saveval;
        instr = ptr + 1;
    }
}

LinkT *CheckTokenList(LinkT *tokenlist)
{
    LinkT *token = tokenlist;
    LinkT *next;
    LinkT *prev = 0;
    char *str;
    char *str1;

    while (token)
    {
	str = token->str;

	if (strcmp(str, "++") == 0 || strcmp(str, "--") == 0)
	{
	    if (token->next)
	    {
		str1 = token->next->str;
	        if (str1[0] == '~')
	        {
		    PrintFileLine();
		    printf(" - Found %s %s.  Removing %s\n", str, str1, str1);
		    next = token->next;
		    token->next = next->next;
		    LinkDestroy(next);
		}
	    }
	}
	else if (str[0] == '~')
	{
	    LinkT *token1;
	    token1 = token->next;
	    if (token1 == 0) break;
            token1 = token1->next;
	    if (token1 == 0) { prev = token; token = token->next; continue; }
	    if (token1->str[0] != '[')
	        { prev = token; token = token->next; continue; }
	    for (token1 = token1->next; token1; token1 = token1->next)
	    {
		if (token1->str[0] == ']') break;
	    }
	    if (token1 == 0) { prev = token; token = token->next; continue; }
	    token1 = token1->next;
	    if (token1 == 0) { prev = token; token = token->next; continue; }
	    str1 = token1->str;
	    if (strcmp(str1, "++") == 0 || strcmp(str1, "--") == 0)
	    {
		PrintFileLine();
	        printf(" - Found %s ... %s.  Removing %s\n", str, str1, str);
		next = token;
	        token = token->next;
		if (prev == 0)
		    tokenlist = token;
		else
		    prev->next = token;
		LinkDestroy(next);
		continue;
	    }
	}
        prev = token;
        token = token->next;
    }
    return tokenlist;
}

int lastcharvar(char *str)
{
    int val;
    int len = strlen(str);
    if (len == 0) return 0;
    val = str[len-1];
    if (val >= 'A' && val <= 'Z') return 1;
    if (val >= 'a' && val <= 'z') return 1;
    if (val >= '0' && val <= '9') return 1;
    if (val == '_') return 1;
    if (val == '"') return 1;
    return 0;
}

int firstcharvar(char *str)
{
    int val = str[0];
    if (val >= 'A' && val <= 'Z') return 1;
    if (val >= 'a' && val <= 'z') return 1;
    if (val == '_') return 1;
    return 0;
}

void printtokenlist(LinkT *tokenlist)
{
    char *token;

    if (tokenlist == 0) return;

    while (tokenlist)
    {
	token = tokenlist->str;
        fprintf(outfile, "%s", token);
	tokenlist = tokenlist->next;
    }
    fprintf(outfile, "\n");
}

char *MergeTokens(LinkT *tokenlist)
{
    char *str;
    char *buffer = str_alloc(200);

    while (tokenlist)
    {
	str = tokenlist->str;
        if (str[0] != '\n')
        {
	    strcat(buffer, str);
	    strcat(buffer, " ");
        }
        else
        {
	    strcat(buffer, "\n");
        }
	tokenlist = tokenlist->next;
    }
    return buffer;
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
