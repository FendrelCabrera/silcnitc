# Stage 3 : Adding Flow Control statements

## Objectives
+ Add support for if-then-else, while, break and continue statements
+ Add support for typed expressions
+ Implement label generation and translation

## Label Translation
`labelReplace.l` is a LEX file which handles label translation. It expects `program.xsm` to contain xsm code with labels and generates `test.xsm` after label translation.

```
lex labelReplace.l
gcc lex.yy.c
./a.out                
```