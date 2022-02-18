#!/bin/bash

lex $1

# 在Linux上使用下面这一行
# gcc -o "${1%.*}" -lfl lex.yy.c

# 在macOS上使用下面这一行
gcc -o "${1%.*}" lex.yy.c -ly -ll

rm lex.yy.c
