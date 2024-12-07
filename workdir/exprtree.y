%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "exprtree.h"
 #include "exprtree.c"
 int yylex(void);
 FILE* fout;
 int op;
%}

%union{
 struct tnode *no;

}
%type <no> expr NUM program END
%token NUM PLUS MINUS MUL DIV END
%left PLUS MINUS
%left MUL DIV

%%

program : expr END {
    $$ = $2;
    op = evaluate($1);
    printf("Answer : %d\n", op);
    fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, 4096\n");
    codeGen($1, fout);
    fprintf(fout, "MOV [4096], R0\nMOV R1, \"Write\"\nPUSH R1\nMOV R1, -2\nPUSH R1\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nMOV R0, \"Exit\"\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n");
    fclose(fout);
    exit(1);
   }
  ;

expr : expr PLUS expr  {$$ = makeOperatorNode('+',$1,$3);}
  | expr MINUS expr   {$$ = makeOperatorNode('-',$1,$3);}
  | expr MUL expr {$$ = makeOperatorNode('*',$1,$3);}
  | expr DIV expr {$$ = makeOperatorNode('/',$1,$3);}
  | '(' expr ')'  {$$ = $2;}
  | NUM   {$$ = $1;}
  ;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(void) {
 fout = fopen("program.xsm", "w");
 yyparse();
 return 0;
}

