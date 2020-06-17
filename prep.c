/*
############################################################################
# Copyright (c) 2010 - 2014 Dave Hein
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "links.h"
#include "line.h"
#include "tokens.h"
#include "strsubs.h"
#include "direct.h"

extern DefInfoT *deflist;
FILE *inputfile[10];
char inputfname[10][80];
int inputline[10];
int inputfileindex = 0;
FILE *outfile;
static int debugflag = 0;
int skipflag = SKIP_NONE;
int skiplevel = 0;
int printskip = PRINTSKIP_NONE;
int commentflag = 0;
int commentflag1 = 0;
LinkT *includelist = 0;

static LinkT *tokenlistin = 0;

int ParseCommandLineArgs(int argc, char **argv);
int ProcessIncludeDirective(LinkT *tokenlistin);

int OpenFiles(char *fname, char *outfname)
{
    int len;
    char filename[100];

    strncpy(inputfname[0], fname, 79);
    inputfname[0][79] = 0;
    inputline[0] = 0;
    inputfile[0] = fopen(inputfname[0], "r");

    if (inputfile[0] == 0)
    {
        printf("Could not open input file %s\n", fname);
	return 0;
    }

    if (!outfname)
    {
        strcpy(filename, fname);
        len = strlen(filename);
        if (filename[len-2] == '.' && filename[len-1] == 'c') filename[len-2] = 0;
        strcat(filename, ".spin");
    }
    else
        strcpy(filename, outfname);

    outfile = fopen(filename, "w");

    if (outfile == 0)
    {
        printf("Could not open output file %s\n", filename);
	fclose (inputfile[0]);
	return 0;
    }

    return 1;
}

void DestroyAllLists(void)
{
    tokenlistin = ListDestroy(tokenlistin);
    deflist = DestroyDefList(deflist);
}

int ProcessFile(char *infname, char *outfname)
{
    inputfileindex = 0;
    if (OpenFiles(infname, outfname) == 0) return 0;

    while ((tokenlistin = getcline()))
    {
        if (skipflag)
        {
            if (skipflag == SKIP_LINE) skipflag = SKIP_NONE;
            if (printskip == PRINTSKIP_COMMENT)
                fprintf(outfile, "// ");
            else
            {
                if (printskip == PRINTSKIP_BLANK)
                    fprintf(outfile, "\n");
                tokenlistin = ListDestroy(tokenlistin);
                continue;
            }
        }
        printtokenlist(tokenlistin);
        tokenlistin = ListDestroy(tokenlistin);
    }

    return 0;
}

void usage(void)
{
    printf("usage: prep [options] infile\n");
    printf("  options are:\n");
    printf("  -d           - Print debug information\n");
    printf("  -p#          - Set printskip\n");
    printf("  -o outfile   - Set the output file name\n");
    printf("  -D symbol    - Add symbol to defined list\n");
    printf("  -I directory - Add directory to include list\n");
    exit(1);
}

char *ParseCommandLine(int argc, char **argv)
{
    int i;
    char *outfname = 0;

    if (argc < 2)
        usage();

    inputfname[0][0] = 0;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'd')
                debugflag = 1;
            else if (argv[i][1] == 'p')
                printskip = atoi(&argv[i][2]);
            else if (argv[i][1] == 'o')
            {
                if (argv[i][2])
                    outfname = &argv[i][2];
                else if (i < argc - 1)
                    outfname = argv[++i];
                else
                    usage();
            }
            else if (argv[i][1] == 'D')
            {
                char *ptr;
                LinkT *link;
                if (argv[i][2])
                    ptr = &argv[i][2];
                else if (i < argc - 1)
                    ptr = argv[++i];
                else
                    usage();
                getctokens(ptr, &link);
                if (link->next && link->next->str[0] == '=')
                    link->next->str[0] = ' ';
                //link = LinkCreate(ptr, sizeof(LinkT));
                AddToDefList(link, DEFTYPE_DEFINE);
            }
            else if (argv[i][1] == 'I')
            {
                char *ptr;
                LinkT *link;
                if (argv[i][2])
                    ptr = &argv[i][2];
                else if (i < argc - 1)
                    ptr = argv[++i];
                else
                    usage();
                link = LinkCreate(ptr, sizeof(LinkT));
                link->next = includelist;
                includelist = link;
            }
            else
            {
	        printf("Invalid option %s\n", argv[i]);
                usage();
            }
        }
        else
        {
            strncpy(inputfname[0], argv[i], 79);
            inputfname[0][79] = 0;
        }
    }
    return outfname;
}

int main(int argc, char **argv)
{
    char *outfname = ParseCommandLine(argc, argv);
    if (!inputfname[0][0]) usage();
    ProcessFile(inputfname[0], outfname);

    return 0;
}

FILE *OpenIncludeFile(char *fname)
{
    int len;
    FILE *fd;
    LinkT *list;
    char path[200];

    if ((fd = fopen(fname, "r"))) return fd;

    list = includelist;
    while (list)
    {
        strcpy(path, list->str);
        len = strlen(path);
        if (path[len-1] != '/')
            strcat(path, "/");
        strcat(path, fname);
        if ((fd = fopen(path, "r"))) break;
        list = list->next;
    }

    return fd;
}

int ProcessIncludeDirective(LinkT *tokenlist)
{
    char *ptr;
#if 0
    LinkT *list = tokenlist;

printf("ProcessIncludeDirective: ");
while (list)
{
    printf("<%s>", list->str);
    list = list->next;
}
printf("\n");
#endif

    if (tokenlist == 0) return 0;
    if (strcmp(tokenlist->str, "#")) return 0;

    tokenlist = tokenlist->next;
    if (tokenlist == 0) return 0;
    if (strcmp(tokenlist->str, "include")) return 0;

    tokenlist = tokenlist->next;
    if (tokenlist == 0) return 0;
    tokenlist = tokenlist->next;
    if (tokenlist == 0) return 0;
    if (tokenlist->str[0] != '"') return 0;

    if (inputfileindex > 9)
    {
	printf("Too many nested includes\n");
	return 0;
    }

    inputfileindex++;
    strncpy(inputfname[inputfileindex], tokenlist->str + 1, 79);
    inputfname[inputfileindex][79] = 0;
    ptr = FindChar(inputfname[inputfileindex], '"');
    *ptr = 0;
    inputline[inputfileindex] = 0;
#if 0
    inputfile[inputfileindex] = fopen(inputfname[inputfileindex], "r");
#else
    inputfile[inputfileindex] = OpenIncludeFile(inputfname[inputfileindex]);
#endif
    if (inputfile[inputfileindex] == 0)
    {
	inputfileindex--;
	printf("%s, %d - ", inputfname[inputfileindex], inputline[inputfileindex]);
	printf("Could not open include file %s\n", inputfname[inputfileindex+1]);
        printtokenlist(tokenlist);
    }

    skipflag = SKIP_LINE;

    return 1;
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
