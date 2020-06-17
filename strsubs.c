/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
#include <string.h>
#include "strsubs.h"

int isdigit(int val)
{
    return (val >= '0' && val <= '9');
}

int isdigits(char *ptr)
{
    while (*ptr)
    {
        if (*ptr < '0' || *ptr > '9') return 0;
	ptr++;
    }
    return 1;
}


int alldigits(char *ptr1, char *ptr2)
{
    while (ptr1 <= ptr2)
    {
	if (!isdigit(*ptr1++)) return 0;
    }
    return 1;
}

int toupper(int val)
{
    if (val >= 'a' && val <= 'z') val -= 'a' - 'A';
    return val;
}

int strcmpnocase(char *str1, char *str2)
{
    int val1 = toupper(*str1++);
    int val2 = toupper(*str2++);

    while (val1 && val2 && val1 == val2)
    {
	val1 = toupper(*str1++);
	val2 = toupper(*str2++);
    }
    return (val1 != val2);
}

int isalphanum(int val)
{
    return (val >= '0' && val <= '9') || (val >= 'A' && val <= 'Z') || (val >= 'a' && val <= 'z');
}

void RemoveCRLF(char *buf)
{
    int len = strlen(buf) - 1;
    buf += len;
    while (len >= 0)
    {
        if (*buf != 0x0a && *buf != 0x0d) break;
        *buf-- = 0;
        len--;
    }
}

char *SkipChar(char *ptr, int val)
{
    while (*ptr)
    {
        if (*ptr != val) break;
        ptr++;
    }
    return ptr;
}

char *FindChar(char *ptr, int val)
{
    while (*ptr)
    {
        if (*ptr == val) break;
        ptr++;
    }
    return ptr;
}

char *FindChars(char *ptr, char *vals)
{
    char *vptr;

    while (*ptr)
    {
        vptr = vals;
        while (*vptr)
        {
            if (*ptr == *vptr) break;
            vptr++;
        }
        if (*ptr == *vptr) break;
        ptr++;
    }
    return ptr;
}

char *SkipChars(char *ptr, char *vals)
{
    char *vptr;

    while (*ptr)
    {
        vptr = vals;
        while (*vptr)
        {
            if (*ptr == *vptr) break;
            vptr++;
        }
        if (*ptr != *vptr) break;
        ptr++;
    }
    return ptr;
}

int SearchList(char *str, char **list, int searchmode)
{
    int i;

    if (searchmode == SEARCH_EXACT)
    {
        for (i = 0;list[i] != 0; i++)
            if (strcmp(str, list[i]) == 0) break;
    }
    else
    {
        for (i = 0;list[i] != 0; i++)
            if (strncmp(str, list[i], strlen(list[i])) == 0) break;
    }

    if (list[i] == 0) i = -1;
    else if (searchmode == SEARCH_WORD)
    {
        int len = strlen(list[i]);
        int val = str[len];
        if (val != 0 && val != ' ' && val != '\t') i = -1;
    }

    return i;
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
