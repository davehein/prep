#define TOKENS_NEW_WAY

void getctokens(char *instr, LinkT **ptokenlist);
void printtokenlist(LinkT *tokenlist);
char *MergeTokens(LinkT *tokenlist);
LinkT *AddToken(char *str, LinkT **ptokenlist);
LinkT *AddTokenNum(int num, LinkT **ptokenlist);
LinkT *InsertToken(char *str, LinkT **ptokenlist, LinkT *token);
LinkT *ReplaceToken(char *str, LinkT **ptokenlist, LinkT *token);
void RemoveToken(LinkT **ptokenlist, LinkT *token);
int ConvertTokenListToArray(LinkT *tokenlist, char **tokens, int max);
LinkT *DuplicateTokenList(LinkT *tokenlist);
LinkT *CheckTokenList(LinkT *tokenlist);
void DebugPrintTokenList(LinkT *tokenlist);
