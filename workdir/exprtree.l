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
"decl" {return DECL;}
"enddecl" {return ENDDECL;}
"str" {return STRT;}
"int" {return INT;}
[a-zA-Z_][a-zA-Z0-9_]* {
	char *c = strdup(yytext);
	yylval.str = c;
	return ID;
}
\"[^"]*\" {
    char *c = strdup(yytext);
    yylval.no = createTree(0, 2, c, 23, NULL, NULL, NULL);
    return STRC;
}
[ \t\n] {}
[\+\-\*/\(\);=<>!,%\[\]&] {return *yytext;}
. {printf("unknown character: %c\n", *yytext); exit(1);}

%%

int yywrap(void) {
 return 1;
}

