# Script file to build cspin using GCC
gcc -Wall prep.c line.c strsubs.c tokens.c links.c direct.c getval.c -lm -o prep
