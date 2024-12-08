%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "exprtree.h"
 #include "exprtree.c"
 int yylex(void);
 extern FILE *yyin;
 FILE* fout;
 int op;
%}

%union{
 struct tnode *no;
}

%type <no> expr program Slist Stmt ID NUM InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Repeatstmt Dowhilestmt
%token NUM END BGN READ ID WRITE IF ENDIF WHILE ENDWHILE THEN ELSE DO BREAK CONTINUE REPEAT UNTIL
%left '<' '>' '!' '='
%left '+' '-'
%left '*' '/'

%%
program: BGN Slist END ';' {
       		$$ = $2;
		    printf("Parsing success.\n");
		    
      		fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, 4121\n");
		    codeGen($$, fout);
		    fprintf(fout, "MOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n");
		    fclose(fout);
		    
		    // evaluate($$);
            // prefix($$);
            // printf("\n");
		    exit(1);
	     }
       | BGN END ';' {printf("Empty program\n"); exit(1);}
       ;

Slist: Slist Stmt {$$ = createTree(0, -1, NULL, 0, $1, $2, NULL);} 
     | Stmt {$$ = $1;}
     ;

Stmt: InputStmt {$$ = $1;}
    | OutputStmt {$$ = $1;}
    | AsgStmt {$$ = $1;}
    | Ifstmt {$$ = $1;}
    | Whilestmt {$$ = $1;}
    | BREAK ';' {$$=createTree(0, -1, NULL, 19, NULL, NULL, NULL);}
    | CONTINUE ';' {$$=createTree(0, -1, NULL, 20, NULL, NULL, NULL);}
    | Repeatstmt {$$ = $1;}
    | Dowhilestmt {$$ = $1;}
    ;

InputStmt: READ '(' ID ')' ';' {$$ = createTree(0, -1, NULL, 1, $3, NULL, NULL);};

OutputStmt: WRITE '(' expr ')' ';' {$$ = createTree(0, -1, NULL, 2, $3, NULL, NULL);};

AsgStmt: ID '=' expr ';' {$$ = createTree(0, -1, NULL, 15, $1, $3, NULL);};

Ifstmt: IF '(' expr ')' THEN Slist ELSE Slist ENDIF ';' {$$=createTree(0, -1, NULL, 17, $3, $6, $8);}
      | IF '(' expr ')' THEN Slist ENDIF ';' {$$ = createTree(0, -1, NULL, 16, $3, $6, NULL);}
      ;

Whilestmt: WHILE '(' expr ')' DO Slist ENDWHILE ';' {$$ = createTree(0, -1, NULL, 18, $3, $6, NULL);}
         ;

Repeatstmt: REPEAT Slist UNTIL '(' expr ')' ';' {$$ = createTree(0, -1, NULL, 21, $2, $5, NULL);}
          ;

Dowhilestmt: DO Slist WHILE '(' expr ')' ';' {$$ = createTree(0, -1, NULL, 22, $2, $5, NULL);}
           ;

expr : expr '+' expr  {$$ = createTree(0, 1, NULL, 3, $1, $3, NULL);}
  | expr '-' expr   {$$ = createTree(0, 1, NULL, 4, $1, $3, NULL);}
  | expr '*' expr {$$ = createTree(0, 1, NULL, 5, $1, $3, NULL);}
  | expr '/' expr {$$ = createTree(0, 1, NULL, 6, $1, $3, NULL);}
  | '(' expr ')'  {$$ = $2;}
  | expr '<' expr {$$ = createTree(0, 0, NULL, 9, $1, $3, NULL);}
  | expr '>' expr {$$ = createTree(0, 0, NULL, 11, $1, $3, NULL);}
  | expr '<' '=' expr {$$ = createTree(0, 0, NULL, 10, $1, $4, NULL);}
  | expr '>' '=' expr {$$ = createTree(0, 0, NULL, 12, $1, $4, NULL);}
  | expr '!' '=' expr {$$ = createTree(0, 0, NULL, 14, $1, $4, NULL);}
  | expr '=' '=' expr {$$ = createTree(0, 0, NULL, 13, $1, $4, NULL);}
  | NUM	 {$$ = $1;} 
  | ID {$$ = $1;} 
  ;

%%

yyerror(char const *s)
{
    printf("yyerror %s",s);
}


int main(int argc, char **args) {
    if (argc > 1) 
        yyin = fopen(args[1], "r");
    fout = fopen("program.xsm", "w");
    yyparse();
    return 0;
}

