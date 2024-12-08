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
 
    Typetable *tStack[3], *returnType = NULL;
    int tLast = -1;
%}

%union{
    struct ASTNode *node;
    struct Paramstruct *paramlist;
    struct Lsymbol *lentry;
    char *sval;
    int nval;
    struct Fieldlist *flist;
    struct Typetable *type;
}

%type <node> program FDefBlock FDef MainBlock Body Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Repeatstmt Dowhilestmt ReturnStmt STRC Assignable expr ArgList NULLC FIELD InitStmt AllocStmt FreeStmt
%type <paramlist> ParamList Param
%type <sval> ID
%type <nval> NUM
%type <lentry> Lid
%type <flist> FieldDeclList FieldDecl
%type <type> TypeName
%token DECL ENDDECL ID NUM INT STRT MAIN BREAK CONTINUE READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE REPEAT UNTIL RET STRC BGN END BREAKPOINT AND OR TYPE ENDTYPE NULLC ALLOC INIT FREE
%left OR
%left AND
%left '<' '>' '!' '='
%left '+' '-'
%left '*' '/' '%'

%%
program: TypeDefBlock GDeclBlock FDefBlock MainBlock {
             $$ = TreeCreate(NULL, N_SLIST, NULL, (Constant) 0, NULL, $3, $4, NULL);
             printf("Parsing complete\n");

             for (Gsymbol *g = GST; g != NULL; g = g->next) {
                 if ( g->size == -1 ) {
                     printf("Func %s declared but not defined\n", g->name);
                     exit(1);
                 }
             }

             printT($$, 0);
             
             fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\nPUSH R0\nCALL F0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095+gstSize);
             // R0 is pushed for storing return value of main
             // trying to store it outside stack will cause error
		     
             codeGen($$, fout);
             
		     fclose(fout);
             exit(1);
         }
       | TypeDefBlock GDeclBlock MainBlock {
             $$ = $3;
             printf("Parsing complete\n");
             printT($$, 0);
            
             fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nMOV SP, %d\nPUSH R0\nCALL F0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095+gstSize);
           
             codeGen($$, fout);
             
		     fclose(fout);
             exit(1);
         }
       ;

TypeDefBlock  : /* empty */
              | TYPE TypeDefList ENDTYPE { printTT(); }
              ;

TypeDefList   : TypeDefList TypeDef
              | TypeDef
              ;

TypeDef       : ID '{' FieldDeclList '}'   { 
                    int fsize = 0;
                    for (Fieldlist *f = $3; f != NULL; f = f->next) fsize++;

                    if (fsize > 8) {
                        printf("Type %s should not have more than 8 fields\n", $1);
                        exit(1);
                    }

                    Typetable *Tptr = TInstall($1, fsize, $3);
                    for (Fieldlist *f = $3; f != NULL; f = f->next) {
                        if (strcmp (f->type->name, $1) == 0) {
                            free(f->type);
                            f->type = Tptr;
                        } else if (f->type->size == -1) {
                            printf("%d: Type %s not defined\n", line, f->type->name);
                            exit(1);
                        }
                    }
                }
              ;

FieldDeclList : FieldDeclList FieldDecl {
                    Fieldlist *f = $1;
                    $$ = $1;
                    
                    while (f != NULL) {
                        if (strcmp(f->name, $2->name) == 0) {
                            printf("%d: Field %s redeclared\n", line, f->name);
                            exit(1);
                        }

                        if (f->next != NULL) f = f->next;
                        else break;
                    }

                    f->next = $2;
                    $2->fieldIndex = f->fieldIndex + 1;
                }
              | FieldDecl {$$ = $1; $1->fieldIndex = 0;}; 
              ;

FieldDecl    : TypeName ID ';' {
                   Fieldlist *f = (Fieldlist *)malloc(sizeof(Fieldlist));
                   f->name = $2;
                   f->type = $1;
                   f->next = NULL;
                   $$ = f;
               }
             ;

TypeName: ID  {
              if (strcmp ($1, "void") == 0 || strcmp($1, "bool") == 0) {
                  printf("%d: Field of type %s not allowed\n", line, $1);
                  exit(1);
              }

              $$ = TLookup($1);
              if ($$ == NULL) {
                  $$ = (Typetable *)malloc(sizeof(Typetable));
                  $$->name = $1;
                  $$->size = -1;
                  $$->fields = NULL;
                  $$->next = NULL;
              }
          } 
        ;

GDeclBlock: /* empty */ | DECL GDeclList ENDDECL { printGST(); };
GDeclList: GDeclList GDecl | GDecl ;
GDecl: Type GIdList ';' {--tLast;} ;
GIdList: GIdList ',' Gid | Gid ;

Gid: ID {GInstall($1, tStack[tLast], 1, NULL);} 
   | ID '[' NUM ']' {GInstall($1, tStack[tLast], $3, NULL);}
   | ID '(' ParamList ')' {GInstall($1, tStack[tLast], 0, $3); LST = NULL;}
   | '*' ID {
         /*
         if (tStack[tLast] == T_STR)
             $$ = GInstall($2, T_STRP, 1, NULL);
         else if (tStack[tLast] == T_INT)
             $$ = GInstall($2, T_INTP, 1, NULL);
         */
     }
   ;

FDefBlock: FDefBlock FDef {$$ = TreeCreate(NULL, N_SLIST, NULL, (Constant) 0, NULL, $1, $2, NULL);}
         | FDef {$$ = $1;}
         ;
FDef: Type ID '(' ParamList ')' '{' LDeclBlock Body '}' {
          Gsymbol *x = GLookup($2);
          if (x == NULL || x->size > 0) {
              printf("%d: Function not declared: %s\n", line, $2);
              exit(1);
          } else if ( x->size == 0 ) {
              printf("%d: Func redefined: %s\n", line, $2);
              exit(1);
          } else if (tStack[tLast] != x->type || x->type != $8->type) {
              printf("%d: Func decl|def|body type mismatch: %s\n", line, $2);
              exit(1);
          } 

          Paramstruct *t1 = $4, *t2 = x->paramlist;
          int p = 0;
          while(t1 != NULL && t2 != NULL) {
              if (strcmp(t1->name, t2->name) || t1->type != t2->type) {
                  printf("%d: Func parameter mismatch: %s\n", line, $2);
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

              x->size = 0;
              LST = NULL;
              --tLast;
              returnType = NULL;
          } else {
              printf("%d: Func parameter mismatch: %s\n", line, $2);
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
         | Param {$$ = $1;} | {$$ = NULL;}
         ;
Param: Type Lid {
           $$ = (Paramstruct *)malloc(sizeof(Paramstruct));
           $$->name = $2->name;
           // $$->type = ($2->type == T_STRP || $2->type == T_INTP)?(T_INT):($2->type);
           $$->type = $2->type;
           $$->next = NULL;

           tLast--;
       }
     ;

Type: ID {
          tStack[++tLast] = TLookup($1);
          if (tStack[tLast] == NULL) {
              printf("%d: Type %s not defined\n", line, $1);
              exit(1);
          } else if (strcmp($1, "void") == 0 || strcmp($1, "bool") == 0) {
              printf("%d: ID of type %s cannot be defined\n", line, $1);
              exit(1);
          }
      }
    ;

LDeclBlock: DECL LDeclList ENDDECL | ;
LDeclList: LDeclList LDecl | LDecl ;
LDecl: Type LIdList ';' {--tLast;} ;
LIdList: LIdList ',' Lid | Lid ;

Lid: ID {$$ = LInstall($1, tStack[tLast]);}
   | '*' ID {
         /*
         if (tStack[tLast] == T_STR)
             $$ = LInstall($2, T_STRP);
         else if (tStack[tLast] == T_INT)
             $$ = LInstall($2, T_INTP);
         */
     }
   ;

MainBlock: Type MAIN '(' ')' '{' LDeclBlock Body '}' {
               int p = 1;
               for (Lsymbol *l = LST; l != NULL; l = l->next)
                   l->binding = p++;

               $7->nodetype = N_MAIN;
               $7->Lentry = LST;
               LST = NULL;
               returnType = NULL;
               $$ = $7;
           } 
         ;

Body: BGN Slist ReturnStmt END {
          // $$ = $2; $$->nodetype = N_BODY; $$->type = returnType;
          $$ = TreeCreate(returnType, N_BODY, NULL, (Constant) 0, NULL, $2, $3, NULL);
      } 
    ;

Slist: Slist Stmt {$$ = TreeCreate(NULL, N_SLIST, NULL, (Constant) 0, NULL, $1, $2, NULL);} 
     | Stmt {$$ = $1;}
     ;

Stmt: InputStmt {$$ = $1;}
    | OutputStmt {$$ = $1;}
    | AsgStmt {$$ = $1;}
    | Ifstmt {$$ = $1;}
    | Whilestmt {$$ = $1;}
    | BREAK ';' {$$ = TreeCreate(NULL, N_BREAK, NULL, (Constant)0, NULL, NULL, NULL, NULL);}
    | CONTINUE ';' {$$ = TreeCreate(NULL, N_CONTINUE, NULL, (Constant)0, NULL, NULL, NULL, NULL);}
    | Repeatstmt {$$ = $1;}
    | Dowhilestmt {$$ = $1;}
    | ReturnStmt {$$ = $1;}
    | BREAKPOINT ';' {$$ = TreeCreate(NULL, N_BRKP, NULL, (Constant) 0, NULL, NULL, NULL, NULL);}
    | AllocStmt {$$ = $1;}
    | InitStmt {$$ = $1;}
    | FreeStmt {$$ = $1;}
    ;

Assignable: ID {$$ = TreeCreate(NULL, N_ID, $1, (Constant)0, NULL, NULL, NULL, NULL);}
          | ID '[' expr ']' {
                $$ = TreeCreate(NULL, N_ID, $1, (Constant)0, NULL, $3, NULL, NULL);
            }
          | ID '[' expr ']' '[' expr ']' {
                $$ = TreeCreate(NULL, N_ID, $1, (Constant)0, NULL, $3, $6, NULL);
            }
          | '*' ID {
                // $$ = createTree(0, -1, $2, 27, NULL, NULL, NULL);
                // $$ = TreeCreate(-1, N_PTR, $2, (Constant)0, NULL, NULL, NULL, NULL);
            }
          | FIELD { $$ = $1; }
          ;

FIELD: ID '.' ID {
           $$ = TreeCreate(NULL, N_ID, $1, (Constant) 0, NULL, NULL, NULL, NULL);
           $$->ptr3 = TreeCreate($$->type, N_FIELD, $3, (Constant) 0, NULL, NULL, NULL, NULL);
           $$->type = $$->ptr3->type;
           $$->nodetype = N_FIELD;
       }
     | FIELD '.' ID {
           $$ = $1;
           ASTNode *x = TreeCreate($$->type, N_FIELD, $3, (Constant) 0, NULL, NULL, NULL, NULL);

           ASTNode *t = $$->ptr3;
           for (; t->ptr1 != NULL; t = t->ptr3);
           t->ptr3 = x;

           $$->type = x->type;
       }
     ;

InputStmt: READ '(' Assignable ')' ';' {
                $$ = TreeCreate(NULL, N_READ, NULL, (Constant) 0, NULL, $3, NULL, NULL);
           }
         ;

OutputStmt: WRITE '(' expr ')' ';' {
                $$ = TreeCreate(NULL, N_WRITE, NULL, (Constant) 0, NULL, $3, NULL, NULL);
            } ;


AsgStmt: Assignable '=' expr ';' {
            $$ = TreeCreate(NULL, N_ASGN, NULL, (Constant) 0, NULL, $1, $3, NULL);
         }
       ;

Ifstmt: IF '(' expr ')' THEN Slist ELSE Slist ENDIF ';' {
            $$ = TreeCreate(NULL, N_IF, NULL, (Constant) 0, NULL, $3, $6, $8);
        }
      | IF '(' expr ')' THEN Slist ENDIF ';' {
            $$ = TreeCreate(NULL, N_IF, NULL, (Constant) 0, NULL, $3, $6, NULL);
        }
      ;

Whilestmt: WHILE '(' expr ')' DO Slist ENDWHILE ';' {
               $$ = TreeCreate(NULL, N_WHILE, NULL, (Constant) 0, NULL, $3, $6, NULL);
           }
         ;

Repeatstmt: REPEAT Slist UNTIL '(' expr ')' ';' {
                $$ = TreeCreate(NULL, N_REPEAT, NULL, (Constant) 0, NULL, $2, $5, NULL);
            }
          ;

Dowhilestmt: DO Slist WHILE '(' expr ')' ';' {
                $$ = TreeCreate(NULL, N_DOWHILE, NULL, (Constant) 0, NULL, $2, $5, NULL);
             }
           ;

ReturnStmt: RET expr ';' {
                if (returnType != NULL && returnType != $2->type) {
                    printf("%d: Return stmt type mismatch\n", line);
                    exit(1);
                } 
                returnType = $2->type;
                $$ = TreeCreate(returnType, N_RET, NULL, (Constant) 0, NULL, $2, NULL, NULL);
            } ;

InitStmt: INIT '(' ')' ';' {$$ = TreeCreate(NULL, N_INIT, NULL, (Constant) 0, NULL, NULL, NULL,  NULL);}
        | Assignable '=' INIT '(' ')' ';' {$$ = TreeCreate(NULL, N_INIT, NULL, (Constant) 0, NULL, NULL, NULL, NULL);}
        ;

AllocStmt: Assignable '=' ALLOC '(' ')' ';' {
               $$ = TreeCreate(NULL, N_ALLOC, NULL, (Constant) 0, NULL, $1, NULL, NULL);
           }
         ;

FreeStmt: FREE '(' Assignable ')' ';' {
              $$ = TreeCreate(NULL, N_FREE, NULL, (Constant) 0, NULL, $3, NULL, NULL);
              $3->value.intval = 1;
          }
        ;

expr:   expr '+' expr  {
            $$ = TreeCreate(TLookup("int"), N_PLUS, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '-' expr {
            $$ = TreeCreate(TLookup("int"), N_MINUS, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '*' expr {
            $$ = TreeCreate(TLookup("int"), N_MUL, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '/' expr {
            $$ = TreeCreate(TLookup("int"), N_DIV, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '%' expr {
            $$ = TreeCreate(TLookup("int"), N_MOD, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   '(' expr ')'  {$$ = $2;}
    |   expr '<' expr {
            $$ = TreeCreate(TLookup("bool"), N_LT, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '>' expr {
            $$ = TreeCreate(TLookup("bool"), N_GT, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr '<' '=' expr {
            $$ = TreeCreate(TLookup("bool"), N_LE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '>' '=' expr {
            $$ = TreeCreate(TLookup("bool"), N_GE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '!' '=' expr {
            $$ = TreeCreate(TLookup("bool"), N_NE, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr '=' '=' expr {
            $$ = TreeCreate(TLookup("bool"), N_EQ, NULL, (Constant) 0, NULL, $1, $4, NULL);
        }
    |   expr OR expr {
            $$ = TreeCreate(TLookup("bool"), N_OR, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   expr AND expr {
            $$ = TreeCreate(TLookup("bool"), N_AND, NULL, (Constant) 0, NULL, $1, $3, NULL);
        }
    |   NUM	 {
            $$=TreeCreate(TLookup("int"), N_CONST, NULL, (Constant) $1, NULL, NULL, NULL, NULL);  
        } 
    |   STRC {$$ = $1;}
    |   NULLC {$$ = $1;}
    |   Assignable {
            $1->value.intval = 1;
            $$ = $1;
        } 
    |   '&' Assignable {$2->type = TLookup("int"); $$ = $2;}
    |   ID '(' ArgList ')' {$$ = TreeCreate(NULL, N_FUNC, $1, (Constant) 0, $3, NULL, NULL, NULL);}
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
       | {$$ = NULL;}
       ;


%%

int yyerror(char const *s)
{
    printf("%d: %s\n", line, s);
}


int main(int argc, char **args) {
    if (argc > 1) 
        yyin = fopen(args[1], "r");
    fout = fopen("program.xsm", "w");
    TypeTableCreate();
    yyparse();
    return 0;
}

