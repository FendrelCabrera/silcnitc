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

"begin" { return BGN; }
"end" { return END; }
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
"type" { return TYPE; }
"endtype" { return ENDTYPE; }
"null" { 
    yylval.node = TreeCreate(TLookup("void"), N_CONST, NULL, (Constant) 0, NULL, NULL, NULL, NULL);
    return NULLC;
}
"initialize" { return INIT; }
"alloc" { return ALLOC; }
"free" { return FREE; }
"class" { return CLASS; }
"endclass" { return ENDCLASS; }
"self" {
    yylval.sval = strdup(yytext);
    return SELF;
}
"new" { return NEW; }
"delete" { return DEL; }
[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.sval = strdup(yytext);
	return ID;
}
\"[^"]*\" {
    yylval.node = TreeCreate(TLookup("str"), N_CONST, NULL, (Constant) strdup(yytext), NULL, NULL, NULL, NULL);
    return STRC;
}
[ \t] {}
\n {++line;}
[\+\-\*/\(\);=<>!,%\[\]&\{\}|\.] { return *yytext;}
. { printf("unknown character: %c\n", *yytext); exit(1);}

%%

int yywrap(void) {
 return 1;
}
