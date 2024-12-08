%{
 #include <stdlib.h>
 #include <stdio.h>
 #include "exprtree.h"
 #include "exprtree.c"
 #include <string.h>
 #include <math.h>
 #include "printTree.c"

 int yylex(void);
 extern FILE *yyin;
 FILE* fout;
 
 int tStack[3], tLast = -1;
 int returnType = -1;
%}

%union{
    struct ASTNode *node;
    struct Paramstruct *paramlist;
    struct Lsymbol *lentry;
    char *sval;
    int nval;
}

%type <node> program FDefBlock FDef MainBlock Body Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Repeatstmt Dowhilestmt ReturnStmt STRC Assignable expr ArgList
%type <paramlist> ParamList Param
%type <sval> ID
%type <nval> NUM
%type <lentry> Lid
%token DECL ENDDECL ID NUM INT STRT MAIN BREAK CONTINUE READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE REPEAT UNTIL RET STRC BGN END BREAKPOINT AND OR TUPLE
%left OR
%left AND
%left '<' '>' '!' '='
%left '+' '-'
%left '*' '/' '%'

%%
program: GDeclBlock FDefBlock MainBlock {
             $$ = TreeCreate(-1, N_SLIST, NULL, (Constant) 0, NULL, $2, $3, NULL);
             printf("Parsing complete\n");
             // printGST();
             printT($$, 0);

             fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\nPUSH R0\nCALL F0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095+gstSize);
             // R0 is pushed for storing return value of main
             // trying to store it outside stack will cause error
		     
             codeGen($$, fout);
		     fclose(fout);
             printGST();
             exit(1);
         }
       | GDeclBlock MainBlock {
             $$ = $2; 
             printf("Parsing complete\n");
             printGST();
             printT($$, 0);

             fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\nPUSH R0\nCALL F0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095 + gstSize);
    
             codeGen($$, fout);
             fclose(fout);
             exit(1);
         }
       | MainBlock {
             $$ = $1; 
             printf("Parsing complete\n");
             printT($$, 0);

             fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\nPUSH R0\nCALL F0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095 + gstSize);
    
             codeGen($$, fout);
             fclose(fout);
             exit(1);
         }
       ;

GDeclBlock: DECL GDeclList ENDDECL | DECL ENDDECL ;
GDeclList: GDeclList GDecl | GDecl ;
GDecl: Type GIdList ';' {--tLast;} ;
GIdList: GIdList ',' Gid | Gid ;

Gid: ID {GInstall($1, tStack[tLast], 1, NULL);} 
   | ID '[' NUM ']' {GInstall($1, tStack[tLast], $3, NULL);}
   | ID '(' ParamList ')' {GInstall($1, tStack[tLast], 0, $3); LST = NULL;}
   | ID '(' ')' {GInstall ($1, tStack[tLast], 0, NULL); LST = NULL;}
   | '*' ID {
         if (tStack[tLast] == T_STR)
             GInstall($2, T_STRP, 1, NULL);
         else if (tStack[tLast] == T_INT)
             GInstall($2, T_INTP, 1, NULL);
     }
   ;

FDefBlock: FDefBlock FDef {$$ = TreeCreate(-1, N_SLIST, NULL, (Constant) 0, NULL, $1, $2, NULL);}
         | FDef {$$ = $1;}
         ;
FDef: Type ID '(' ParamList ')' '{' LDeclBlock Body '}' {
          Gsymbol *x = GLookup($2);
          if (x == NULL || x->size != 0) {
              printf("Function not declared: %s\n", $2);
              exit(1);
          } else if ( x->flabel != -1 ) {
              printf("Func redefined: %s\n", $2);
              exit(1);
          } else if (tStack[tLast] != x->type || x->type != $8->type) {
              printf("Func decl|def|body type mismatch: %s\n", $2);
              exit(1);
          } 

          Paramstruct *t1 = $4, *t2 = x->paramlist;
          int p = 0;
          while(t1 != NULL && t2 != NULL) {
              if (strcmp(t1->name, t2->name) || t1->type != t2->type) {
                  printf("Func parameter mismatch: %s\n", $2);
                  exit(1);
              }
              t1 = t1->next; t2 = t2->next; p++;
          }

          if(t1 == NULL && t2 == NULL) {
              // binding assignment
              Lsymbol *t = LST;
              for (int i = 0; i < p; i++) {
                  t->binding = -(i + 3);
                  t = t->next;
              }
              p = 1;
              while (t != NULL) {
                  t->binding = p++;
                  t = t->next;
              }

              $8->name = $2;
              $8->Gentry = x;
              $8->Lentry = LST;
              $$ = $8;

              x->flabel = ++fcount;
              LST = NULL;
              --tLast;
              returnType = -1;
          } else {
              printf("Func parameter mismatch: %s\n", $2);
              exit(1);
          }
      }
    | Type ID '(' ParamList ')' '{' Body '}' {
          Gsymbol *x = GLookup($2);
          if (x == NULL || x->size != 0) {
              printf("Function not declared: %s\n", $2);
              exit(1);
          } else if ( x->flabel != -1 ) {
              printf("Func redefined: %s\n", $2);
              exit(1);
          } else if (tStack[tLast] != x->type || x->type != $7->type) {
              printf("Func decl|def|body type mismatch: %s\n", $2);
              exit(1);
          } 

          Paramstruct *t1 = $4, *t2 = x->paramlist;
          int p = 0;
          while(t1 != NULL && t2 != NULL) {
              if (strcmp(t1->name, t2->name) || t1->type != t2->type) {
                  printf("Func parameter mismatch: %s\n", $2);
                  exit(1);
              }
              t1 = t1->next; t2 = t2->next; p++;
          }

          if(t1 == NULL && t2 == NULL) {
              // binding assignment
              Lsymbol *t = LST;
              for (int i = 0; i < p; i++) {
                  t->binding = -(i + 3);
                  t = t->next;
              }
              p = 1;
              while (t != NULL) {
                  t->binding = p++;
                  t = t->next;
              }

              $7->name = $2;
              $7->Gentry = x;
              $7->Lentry = LST;
              $$ = $7;
              
              x->flabel = 1;
              LST = NULL;
              --tLast;
              returnType = -1;
          } else {
              printf("Func parameter mismatch: %s\n", $2);
              exit(1);
          }
      }
    ;

ParamList: ParamList ',' Param {
               Paramstruct *t = $1;
               while(t->next != NULL)
                   t = t->next;
               t->next = $3;
           }
         | Param {$$ = $1;}
         ;
Param: Type Lid {
           $$ = (Paramstruct *)malloc(sizeof(Paramstruct));
           $$->name = $2->name;
           $$->type = ($2->type == T_STRP || $2->type == T_INTP)?(T_INT):($2->type);
           $$->next = NULL;

           tLast--;
       }
     ;

Type: INT {tStack[++tLast] = T_INT;}
    | STRT {tStack[++tLast] = T_STR;}
    ;

LDeclBlock: DECL LDeclList ENDDECL | DECL ENDDECL ;
LDeclList: LDeclList LDecl | LDecl ;
LDecl: Type LIdList ';' {--tLast;} ;
LIdList: LIdList ',' Lid | Lid ;

Lid: ID {$$ = LInstall($1, tStack[tLast]);}
   | '*' ID {
         if (tStack[tLast] == T_STR)
             $$ = LInstall($2, T_STRP);
         else if (tStack[tLast] == T_INT)
             $$ = LInstall($2, T_INTP);
     }
   ;

MainBlock: INT MAIN '(' ')' '{' LDeclBlock Body '}' {
               int p = 1;
               for (Lsymbol *l = LST; l != NULL; l = l->next)
                   l->binding = p++;

               $7->nodetype = N_MAIN;
               $7->Lentry = LST;
               LST = NULL;
               returnType = -1;
               $$ = $7;
           } 
         | INT MAIN '(' ')' '{' Body '}' {
               int p = 1;
               for (Lsymbol *l = LST; l != NULL; l = l->next)
                   l->binding = p++;

               $6->nodetype = N_MAIN;
               $6->Lentry = LST;
               LST = NULL;
               returnType = -1;
               $$ = $6;
           }
         ;

Body: BGN Slist ReturnStmt END {
          // $$ = $2; $$->nodetype = N_BODY; $$->type = returnType;
          $$ = TreeCreate(returnType, N_BODY, NULL, (Constant) 0, NULL, $2, $3, NULL);
      } 
    ;

Slist: Slist Stmt {$$ = TreeCreate(-1, N_SLIST, NULL, (Constant) 0, NULL, $1, $2, NULL);} 
     | Stmt {$$ = $1;}
     ;

Stmt: InputStmt {$$ = $1;}
    | OutputStmt {$$ = $1;}
    | AsgStmt {$$ = $1;}
    | Ifstmt {$$ = $1;}
    | Whilestmt {$$ = $1;}
    | BREAK ';' {$$ = TreeCreate(-1, N_BREAK, NULL, (Constant)0, NULL, NULL, NULL, NULL);}
    | CONTINUE ';' {$$ = TreeCreate(-1, N_CONTINUE, NULL, (Constant)0, NULL, NULL, NULL, NULL);}
    | Repeatstmt {$$ = $1;}
    | Dowhilestmt {$$ = $1;}
    | ReturnStmt {$$ = $1;}
    | BREAKPOINT ';' {$$ = TreeCreate(-1, N_BRKP, NULL, (Constant) 0, NULL, NULL, NULL, NULL);}
    ;

Assignable: ID {$$ = TreeCreate(-1, N_ID, $1, (Constant)0, NULL, NULL, NULL, NULL);}
          | ID '[' expr ']' {
                $$ = TreeCreate(-1, N_ID, $1, (Constant)0, NULL, $3, NULL, NULL);
            }
          | ID '[' expr ']' '[' expr ']' {
                $$ = TreeCreate(-1, N_ID, $1, (Constant)0, NULL, $3, $6, NULL);
            }
          | '*' ID {
                // $$ = createTree(0, -1, $2, 27, NULL, NULL, NULL);
                $$ = TreeCreate(-1, N_PTR, $2, (Constant)0, NULL, NULL, NULL, NULL);
            }
          ;

InputStmt: READ '(' Assignable ')' ';' {
                $$ = TreeCreate(-1, N_READ, NULL, (Constant) 0, NULL, $3, NULL, NULL);
           }
         ;

OutputStmt: WRITE '(' expr ')' ';' {
                $$ = TreeCreate(-1, N_WRITE, NULL, (Constant) 0, NULL, $3, NULL, NULL);
            } ;


AsgStmt: Assignable '=' expr ';' {
            $$ = TreeCreate(-1, N_ASGN, NULL, (Constant) 0, NULL, $1, $3, NULL);
         }
       ;

Ifstmt: IF '(' expr ')' THEN Slist ELSE Slist ENDIF ';' {
            $$ = TreeCreate(-1, N_IF, NULL, (Constant) 0, NULL, $3, $6, $8);
        }
      | IF '(' expr ')' THEN Slist ENDIF ';' {
            $$ = TreeCreate(-1, N_IF, NULL, (Constant) 0, NULL, $3, $6, NULL);
        }
      ;

Whilestmt: WHILE '(' expr ')' DO Slist ENDWHILE ';' {
               $$ = TreeCreate(-1, N_WHILE, NULL, (Constant) 0, NULL, $3, $6, NULL);
           }
         ;

Repeatstmt: REPEAT Slist UNTIL '(' expr ')' ';' {
                $$ = TreeCreate(-1, N_REPEAT, NULL, (Constant) 0, NULL, $2, $5, NULL);
            }
          ;

Dowhilestmt: DO Slist WHILE '(' expr ')' ';' {
                $$ = TreeCreate(-1, N_DOWHILE, NULL, (Constant) 0, NULL, $2, $5, NULL);
             }
           ;

ReturnStmt: RET expr ';' {
                if (returnType != -1 && returnType != $2->type) {
                    printf("Return stmt type mismatch\n");
                    exit(1);
                } 
                returnType = $2->type;
                $$ = TreeCreate(returnType, N_RET, NULL, (Constant) 0, NULL, $2, NULL, NULL);
            } ;

expr:   expr '+' expr  {
            $$ = TreeCreate(T_INT, N_PLUS, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '-' expr {
            $$ = TreeCreate(T_INT, N_MINUS, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '*' expr {
            $$ = TreeCreate(T_INT, N_MUL, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '/' expr {
            $$ = TreeCreate(T_INT, N_DIV, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '%' expr {
            $$ = TreeCreate(T_INT, N_MOD, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   '(' expr ')'  {$$ = $2;}
    |   expr '<' expr {
            $$ = TreeCreate(T_BOOL, N_LT, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '>' expr {
            $$ = TreeCreate(T_BOOL, N_GT, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '<' '=' expr {
            $$ = TreeCreate(T_BOOL, N_LE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '>' '=' expr {
            $$ = TreeCreate(T_BOOL, N_GE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '!' '=' expr {
            $$ = TreeCreate(T_BOOL, N_NE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '=' '=' expr {
            $$ = TreeCreate(T_BOOL, N_EQ, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr OR expr {
            $$ = TreeCreate(T_BOOL, N_OR, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr AND expr {
            $$ = TreeCreate(T_BOOL, N_AND, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   NUM	 {
            $$=TreeCreate(T_INT, N_CONST, NULL, (Constant) $1, NULL, NULL, NULL, NULL);  
        } 
    |   STRC {$$ = $1;}
    |   Assignable {
            $1->value.intval = 1;
            $$ = $1;
        } 
    |   '&' Assignable {$2->type = T_INT; $$ = $2;}
    |   ID '(' ArgList ')' {$$ = TreeCreate(-1, N_FUNC, $1, (Constant) 0, $3, NULL, NULL, NULL);}
    ;

ArgList: ArgList ',' expr {
            ASTNode *t = $1;
            while (t->ptr3 != NULL)
                t = t->ptr3;
            t->ptr3 = $3;
            $3->ptr3 = NULL;
            $$ = $1;
         }
       | expr {$$ = $1; $$->ptr3 = NULL;}
       | /* empty */ {$$ = NULL;}
       ;


%%

int yyerror(char const *s)
{
    printf("yyerror %s\n",s);
}


int main(int argc, char **args) {
    if (argc > 1) 
        yyin = fopen(args[1], "r");
    fout = fopen("program.xsm", "w");
    yyparse();
    return 0;
}

