#!/bin/bash

yacc -d exprtree.y
lex exprtree.l
gcc -g lex.yy.c y.tab.c -o exprtree
