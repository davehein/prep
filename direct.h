#define DEFTYPE_NONE    0
#define DEFTYPE_INT     1
#define DEFTYPE_FLOAT   2
#define DEFTYPE_STRING  3
#define DEFTYPE_MACRO   4
#define DEFTYPE_UNKNOWN 5
#define DEFTYPE_DEFINE  6

#define SKIP_NONE       0
#define SKIP_MODE       1
#define SKIP_LINE       2

typedef struct DefInfoS {
    struct DefInfoS *next;
    char *str; // Needed so that LinkCreate and LinkDestroy can be used.
    LinkT *tokenlist;
    int type;
} DefInfoT;

void InitDirect(void);
void PrintConstants(void);
int EvaluateNumber(char **tokens, int num);
LinkT *ExpandLine(LinkT *tokenlist);
int GetConstantTypeList(LinkT *tokenlist, double *fval, int *ival);
void AddToDefList(LinkT *tokenlist, int type);
int ProcessDirective(LinkT *tokenlist);
int ProcessIncludeDirective(LinkT *tokenlist);
DefInfoT *DestroyDefList(DefInfoT *definfo);
int IsMacro(LinkT *list);
