This is a test

This
is
a
test

#define is WHAT

#define waddle(a,b,c) a*b+c
#define widdle(a,b,c) a*b+c

This
is x
a
test

This*is/a+test
This was a test

d = waddle(3,10,20)
e = widdle(3,10,20)

#define GARBAGE 1

1
2
#ifdef GARBAGE
3
#else
4
#endif
5
6
#ifndef GARBAGE
#ifdef GARBAGE
6a
#else
6b
#endif
7

#else
8
#ifdef GARBAGE
8a
#else
8b
#endif
#ifndef GARBAGE
8c
#else
8d
#endif
#endif
9
10
GARBAGE
#if 0
11
#else
12
#endif
13
#if 1
14
#else
15
#endif
16
#if defined(GARBAGE)
17
#else
18
#endif
19
#if GARBAGE && 0
20
#else
21
#endif
