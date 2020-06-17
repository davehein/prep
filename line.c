/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
//*****************************************************************************
// The routines in this file are used to process single lines of C code.
// The main routine is getcline, which extracts a single C line.  It determines
// the number of nested levels and the indentatin that should be used.  It
// also determines the type of C line that is being returned.
// These routine handle limit use of #include and conditional compiling.
//*****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "links.h"
#include "line.h"
#include "strsubs.h"
#include "tokens.h"
#include "direct.h"

extern int commentflag;
extern int commentflag1;
extern int skipflag;
extern int printskip;

static char inbuf[CLINE_MAX_SIZE];
static int endoffile = 0;

extern FILE *inputfile[10];
extern int inputline[10];
extern char inputfname[10][80];
extern int inputfileindex;
extern FILE *outfile;

void PrintFileLine(void)
{
    printf("%s: Line %d", inputfname[inputfileindex], inputline[inputfileindex]);
}

static LinkT *GetLine(void)
{
    LinkT *tokenlist = 0;

    while (tokenlist == 0)
    {
        if (fgets(inbuf, 1000, inputfile[inputfileindex]) == 0)
        {
            fclose(inputfile[inputfileindex]);
	    if (inputfileindex == 0)
	    {
                endoffile = 1;
                break;
	    }
	    inputfileindex--;
	    continue;
        }
	inputline[inputfileindex]++;
        RemoveCRLF(inbuf);
        commentflag1 = commentflag;
        getctokens(inbuf, &tokenlist);
        if (tokenlist == 0)
        {
            if (!skipflag || printskip == PRINTSKIP_BLANK)
                fprintf(outfile, "\n");
            else if (skipflag && printskip == PRINTSKIP_COMMENT)
                fprintf(outfile, "// \n");
        }
    }
    return tokenlist;
}

LinkT *getcline(void)
{
    LinkT *tokenlist = GetLine();

    if (endoffile || !tokenlist) return 0;
    if (tokenlist->str[0] != '#' && !skipflag)
        tokenlist = ExpandLine(tokenlist);
    ProcessDirective(tokenlist);

    return tokenlist;
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
