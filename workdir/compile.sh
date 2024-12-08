#!/bin/bash

yacc -d -v exprtree.y
lex exprtree.l
gcc -g lex.yy.c y.tab.c -o exprtree
