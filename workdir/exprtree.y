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

%type <no> expr NUM program Slist Stmt ID
%token NUM PLUS MINUS MUL DIV END BGN READ ID WRITE
%left PLUS MINUS
%left MUL DIV

%%
program: BGN Slist END ';' {
       		$$ = $2;
		/*
      		fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, 4121\n");
		codeGen($$, fout);
		fprintf(fout, "MOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0");
		fclose(fout);
		*/
		evaluate($$);
		printf("Parsing success.\n");
		exit(1);
	}
       | BGN END ';' {printf("Empty program\n"); exit(1);}
       ;

Slist: Slist Stmt {$$ = createTree(0, 0, 0, 'c', $1, $2);} 
     | Stmt {$$ = $1;}
     ;

Stmt: READ '(' ID ')' ';' {$$ = createTree(0, 0, 0, 'r', $3, NULL);}
    | WRITE '(' expr ')' ';' {$$ = createTree(0, 0, 0, 'w', $3, NULL);}
    | ID '=' expr ';' {$$ = createTree(0, 0, 0, '=', $1, $3);}
    ;

expr : expr PLUS expr  {$$ = createTree(0, 0, 0, '+', $1, $3);}
  | expr MINUS expr   {$$ = createTree(0, 0, 0, '-', $1, $3);}
  | expr MUL expr {$$ = createTree(0, 0, 0, '*', $1, $3);}
  | expr DIV expr {$$ = createTree(0, 0, 0, '/', $1, $3);}
  | '(' expr ')'  {$$ = $2;}
  | NUM	 {$$ = $1;}
  | ID {$$ = $1;}
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

