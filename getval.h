DefInfoT *FindDefine(char *str);
void CheckEOL(LinkT *tokenlist);
LinkT *SkipBlanksAndComments(LinkT *tokenlist);
int GetVal(LinkT **ptokenlist, int prec);
void FatalError(char *str);
