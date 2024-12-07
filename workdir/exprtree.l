%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "y.tab.h"
 #include "exprtree.h"

 int number;

%}

%%

[0-9]+ {
	number = atoi(yytext);
	yylval.no = createTree(number, 0, 0, 'n', NULL, NULL);
	return NUM;
}
"read" {return READ;}
"write" {return WRITE;}
"begin" {return BGN;}
"end" {return END;}
[a-z] {
	yylval.no = createTree(0,  0, *yytext, 'v', NULL, NULL);
	return ID;
}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return MUL;}
"/" {return DIV;}
[ \t] {}
[\(\);=] {return *yytext;}
[\n] {}
. {yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
 return 1;
}

