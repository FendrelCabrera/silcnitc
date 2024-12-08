%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "exprtree.h"
 #include "exprtree.c"
 int yylex(void);
 extern FILE *yyin;
 FILE* fout;
 int varDeclType;
%}

%union{
 struct tnode *no;
 char *str;
}

%type <no> expr program Slist Stmt NUM InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Repeatstmt Dowhilestmt STRC Assignable
%type <str> ID
%token NUM END BGN READ ID WRITE IF ENDIF WHILE ENDWHILE THEN ELSE DO BREAK CONTINUE REPEAT UNTIL DECL ENDDECL INT STRT STRC
%left '<' '>' '!' '='
%left '+' '-'
%left '*' '/' '%'

%%
program: BGN Declarations Slist END ';' {
       		$$ = $3;
		    printf("Parsing success.\n");
		    printStEntries();
      		fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\n", 4095 + stSize);
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

Declarations: DECL DeclList ENDDECL
            | DECL ENDDECL
            ;

DeclList: DeclList Decl
        | Decl
        ;

Decl: Type VarList ';'
    ;

Type: INT {varDeclType = 1;}
    | STRT {varDeclType = 2;}
    ;

VarList: VarList ',' ID {Install($3, varDeclType, 1, -1);}
       | ID {Install($1, varDeclType, 1, -1);}
       | VarList ',' ID '[' NUM ']' {Install($3, varDeclType, $5->val, $5->val); free($5);}
       | ID '[' NUM ']' {Install($1, varDeclType, $3->val, $3->val); free($3);}
       | VarList ',' ID '[' NUM ']' '[' NUM ']' {Install($3, varDeclType, $5->val * $8->val, $8->val); free($5); free($8);}
       | ID '[' NUM ']' '[' NUM ']' {Install($1, varDeclType, $3->val * $6->val, $6->val); free($3); free($6);}
       | VarList ',' '*' ID {Install($4, varDeclType+2, 1, -1);}
       | '*' ID {Install($2, varDeclType+2, 1, -1);}
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

Assignable: ID {$$ = createTree(0, -1, $1, 8, NULL, NULL, NULL);}
          | ID '[' expr ']' {
                $$ = createTree(0, -1, $1, 25, $3, NULL, NULL);
            }
          | ID '[' expr ']' '[' expr ']' {
                $$ = createTree(0, -1, $1, 26, $3, $6, NULL);
            }
          | '*' ID {$$ = createTree(0, -1, $2, 27, NULL, NULL, NULL);}
          ;

InputStmt: READ '(' Assignable ')' ';' {
                $$ = createTree(0, -1, NULL, 1, $3, NULL, NULL);
           }
         ;

OutputStmt: WRITE '(' expr ')' ';' {$$ = createTree(0, -1, NULL, 2, $3, NULL, NULL);};

AsgStmt: Assignable '=' expr ';' {
            $$ = createTree(0, -1, NULL, 15, $1, $3, NULL);
         }
       ;

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
  | expr '%' expr {$$ = createTree(0, 1, NULL, 24, $1, $3, NULL);}
  | '(' expr ')'  {$$ = $2;}
  | expr '<' expr {$$ = createTree(0, 0, NULL, 9, $1, $3, NULL);}
  | expr '>' expr {$$ = createTree(0, 0, NULL, 11, $1, $3, NULL);}
  | expr '<' '=' expr {$$ = createTree(0, 0, NULL, 10, $1, $4, NULL);}
  | expr '>' '=' expr {$$ = createTree(0, 0, NULL, 12, $1, $4, NULL);}
  | expr '!' '=' expr {$$ = createTree(0, 0, NULL, 14, $1, $4, NULL);}
  | expr '=' '=' expr {$$ = createTree(0, 0, NULL, 13, $1, $4, NULL);}
  | NUM	 {$$ = $1;} 
  | STRC {$$ = $1;}
  | Assignable {
        $1->val = 1;
        $$ = $1;
    } 
  | '&' Assignable {$2->type = 1; $$ = $2;}
  ;

%%

int yyerror(char const *s)
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

