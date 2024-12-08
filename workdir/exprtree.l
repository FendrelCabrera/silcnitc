%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "y.tab.h"
 #include "exprtree.h"
%}

%%
"decl" { return DECL; }
"enddecl" { return ENDDECL;}
[0-9]+ {
	yylval.nval = atoi(yytext);
    return NUM;
}
"int" { return INT;}
"str" { return STRT;}
"begin" { return BGN; }
"end" { return END; }
"main" { return MAIN;}
"break" { return BREAK;}
"continue" { return CONTINUE;}
"read" { return READ;}
"write" { return WRITE;}
"if" { return IF;}
"then" { return THEN;}
"else" { return ELSE;}
"endif" { return ENDIF;}
"while" { return WHILE;}
"do" { return DO;}
"endwhile" { return ENDWHILE;}
"repeat" { return REPEAT;}
"until" { return UNTIL;}
"return" { return RET;}
"breakpoint" { return BREAKPOINT;}
("&&"|"and"|"AND") { return AND; }
("||"|"or"|"OR") { return OR; }
[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.sval = strdup(yytext);
	return ID;
}
\"[^"]*\" {
    yylval.node = TreeCreate(T_STR, N_CONST, NULL, (Constant) strdup(yytext), NULL, NULL, NULL, NULL);
    return STRC;
}
[ \t\n] {}
[\+\-\*/\(\);=<>!,%\[\]&\{\}|] { return *yytext;}
. { printf("unknown character: %c\n", *yytext); exit(1);}

%%

int yywrap(void) {
 return 1;
}
