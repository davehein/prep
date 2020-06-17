#if 0
#include "test.h"
#include "stdio.h"
#endif

#define GARBAGE 1

#ifdef TEST3
// TEST3 IS DEFINED
#else
// TEST3 IS NOT DEFINED
#endif

#if defined(GARBAGE) || GARBAGE
1
#else
2
#endif
TEST3
