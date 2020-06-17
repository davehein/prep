#define SEARCH_START 0
#define SEARCH_EXACT 1
#define SEARCH_WORD  2

char *SkipChar(char *ptr, int val);
char *FindChar(char *ptr, int val);
char *FindChars(char *ptr, char *vals);
char *SkipChars(char *ptr, char *vals);
void RemoveCRLF(char *buf);
int isalphanum(int val);
int SearchList(char *str, char **list, int searchmode);
char *strcpyx(char *str, char *first, char *last);
int isdigit(int val);
int alldigits(char *ptr1, char *ptr2);
int isdigits(char *ptr);
