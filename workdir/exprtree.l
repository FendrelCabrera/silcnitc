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
	yylval.no = createTree(number, 1, NULL, 7, NULL, NULL, NULL);
	return NUM;
}
"read" {return READ;}
"write" {return WRITE;}
"begin" {return BGN;}
"end" {return END;}
"if" {return IF;}
"then" {return THEN;}
"else" {return ELSE;}
"endif" {return ENDIF;}
"while" {return WHILE;}
"do" {return DO;}
"endwhile" {return ENDWHILE;}
"break" {return BREAK;}
"continue" {return CONTINUE;}
"repeat" {return REPEAT;}
"until" {return UNTIL;}
[a-z] {
	char *c = (char *)malloc(sizeof(char));
	*c = *yytext;
	yylval.no = createTree(0, 1, c, 8, NULL, NULL, NULL);
	return ID;
}
[ \t\n] {}
[\+\-\*/\(\);=<>!] {return *yytext;}
. {yyerror("unknown character: %c\n", *yytext); exit(1);}

%%

int yywrap(void) {
 return 1;
}

