%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exprtree.h"
    #include "exprtree.c"
    #include "printTree.c"

    int yylex(void);
    int yyerror(char const *s);
    extern FILE *yyin;

    void stackSetup();
    FILE* fout;
 
    Typetable *returnType = NULL;
%}

%union{
    struct ASTNode *node;
    struct Paramstruct *paramlist;
    struct Lsymbol *lentry;
    char *sval;
    int nval;
    struct Fieldlist *flist;
    struct Typetable *type;
    struct Gsymbol *gentry;
}

%type <node> program FDefBlock FDef Body Slist Stmt InputStmt OutputStmt AsgStmt Ifstmt Whilestmt Repeatstmt Dowhilestmt ReturnStmt STRC Assignable expr ArgList NULLC FIELD InitStmt AllocStmt FreeStmt FieldFunction
%type <paramlist> ParamList Param
%type <sval> ID SELF
%type <nval> NUM
%type <lentry> Lid LIdList
%type <type> TypeName 
%type <gentry> GIdList Gid
%token DECL ENDDECL ID NUM INT STRT MAIN BREAK CONTINUE READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE REPEAT UNTIL RET STRC BGN END BREAKPOINT AND OR TYPE ENDTYPE NULLC ALLOC INIT FREE CLASS ENDCLASS SELF EXTENDS NEW DEL
%left OR
%left AND
%left '<' '>' '!' '='
%left '+' '-'
%left '*' '/' '%'

%%
program: TypeDefBlock ClassDefBlock GDeclBlock FDefBlock {
             for (Gsymbol *g = GST; g != NULL; g = g->next) {
                 if ( g->size == -1 ) {
                     printf("Func %s declared but not defined\n", g->name);
                     exit(1);
                 }
             }

             printf("Parsing complete\n");
		     fclose(fout);
             exit(0);
         }
       ;

TypeDefBlock  : /* empty */ { printTT(); }
              | TYPE TypeDefList ENDTYPE { printTT(); }
              ;
TypeDefList   : TypeDefList TypeDef
              | TypeDef
              ;

TypeName: ID { Tptr = TInstall($1, 0, NULL); };
TypeDef: TypeName '{' FieldDeclList '}' { Tptr = NULL; };

FieldDeclList : FieldDeclList FieldDecl | FieldDecl ; 
FieldDecl    : ID ID ';' { 
                   if (Tptr != NULL) 
                       Type_Finstall(Tptr, $1, $2); 
                   else
                       Class_Finstall(Cptr, $1, $2);
               } ;

ClassDefBlock   : CLASS ClassDefList ENDCLASS {printCT();}
                | /* empty */ {printCT();}
                ;
ClassDefList    : ClassDefList ClassDef
                | ClassDef
                ;

ClassDef: ClassName '{' DECL CompList ENDDECL MethodDefns '}' {
              if (Cptr->vfuncptr == NULL) {
                  printf("Class %s should have atleast one method\n", Cptr->name);
                  exit(1);
              }

              for (Memberfunclist *m = Cptr->vfuncptr; m != NULL; 
                   m = m->next) {
                  if (m->isDefined == 0) {
                      printf("%d: Method %s declared but not defined for class %s\n", line, m->name, Cptr->name);
                      exit(1);
                  }
              }
              Cptr = NULL;
          }
        ;

ClassName: ID        {Cptr = CInstall($1, NULL);}
         | ID EXTENDS ID {Cptr = CInstall($1, $3);}
         ;

CompList: CompList CompDecl | CompDecl ;
CompDecl: FieldDecl | MDecl ;

/* MethodDecl: MethodDecl MDecl | MDecl ; */
MDecl: ID ID '(' ParamList ')' ';' {
           Typetable *t = TCheck($1);
           Class_Minstall(Cptr, $2, t, $4);
           LST = NULL;
       }
     ;

MethodDefns     : MethodDefns FDef
                | FDef
                ;

FieldFunction   : SELF '.' ID '(' ArgList ')' {
                      if (Cptr == NULL) {
                          printf("%d: Use of self not allowed outside class\n", line);
                          exit(1);
                      }
                      ASTNode *t = TreeCreate(NULL, N_ID, $1, (Constant) 0, NULL, NULL, NULL, NULL);
                      $$ = TreeCreate(NULL, N_METHOD, $3, (Constant) 0, $5, t, NULL, NULL);
                  }
                | ID '.' ID '(' ArgList ')' {
                      ASTNode *t = TreeCreate(NULL, N_ID, $1, (Constant) 0, NULL, NULL, NULL, NULL);
                      if ( t->type != NULL ) {
                          printf("%d: Type %s cannot have methods\n", line, t->type->name);
                          exit(1);
                      }

                      $$ = TreeCreate(NULL, N_METHOD, $3, (Constant) 0, $5, t, NULL, NULL);
                  }
                | FIELD '.' ID '(' ArgList ')' {
                      if ($1->type != NULL) {
                          printf("%d: Type %s cannot have methods\n", line, $1->type->name);
                          exit(1);
                      }
                      $$ = TreeCreate(NULL, N_METHOD, $3, (Constant) 0, $5, $1, NULL, NULL);
                  }
                ;

GDeclBlock: /* empty */ {
                printGST();
                stackSetup();
            } 
          | DECL GDeclList ENDDECL { 
                printGST(); 
                stackSetup();            
            }
          ;

GDeclList: GDeclList GDecl | GDecl ;
GDecl: ID GIdList ';' {
           Typetable *tp = TLookup($1); Classtable *cp = CLookup($1);

           if (tp != NULL) {
               for (Gsymbol *g = $2; g != NULL; g = g->next) 
                   g->type = tp;
           } else if (cp != NULL) {
               int ccount = 0;
               for (Gsymbol *g = $2; g != NULL; g = g->next) {
                   if (g->size == -1) {
                       printf("%d: Class instance cannot be used as return value\n", line);
                       exit(1);
                   } else if (g->size > 1) {
                       printf("%d: Class instance cannot be used in arrays\n", line);
                       exit(1);
                   }

                   g->size = 2;
                   g->binding += ccount;
                   g->ctype = cp;

                   ccount++;
               }
               gstSize += ccount;
           } else {
               printf("%d: Type/class %s not defined\n", line, $1);
               exit(1);
           }
       }
     ;
GIdList: GIdList ',' Gid {$$ = $1;} | Gid  {$$ = $1;};

Gid: ID {$$ = GInstall($1, NULL, NULL, 1, NULL);} 
   | ID '[' NUM ']' {$$ = GInstall($1, NULL, NULL, $3, NULL);}
   | ID '(' ParamList ')' {$$ = GInstall($1, NULL, NULL, 0, $3); LST = NULL;}
   | '*' ID {
         /*
         if (tStack[tLast] == T_STR)
             $$ = GInstall($2, T_STRP, 1, NULL);
         else if (tStack[tLast] == T_INT)
             $$ = GInstall($2, T_INTP, 1, NULL);
         */
     }
   ;

FDefBlock: FDefBlock FDef {/* $$ = TreeCreate(NULL, N_SLIST, NULL, (Constant) 0, NULL, $1, $2, NULL); */}
         | FDef {$$ = $1;}
         ;
FDef: ID ID '(' ParamList ')' '{' LDeclBlock Body '}' {
          Typetable *t = TCheck($1);
          Paramstruct *t1 = $4, *t2;
          if (Cptr == NULL) {
              Gsymbol *x = GLookup($2);
              if (x == NULL || x->size > 0) {
                  printf("%d: Function not declared: %s\n", line, $2);
                  exit(1);
              } else if ( x->size == 0 ) {
                  printf("%d: Func redefined: %s\n", line, $2);
                  exit(1);
              } else if (t != x->type || x->type != $8->type) {
                  printf("%d: Func decl|def|body type mismatch: %s\n", line, $2);
                  exit(1);
              }
              $8->Gentry = x;
              t2 = x->paramlist;
          } else {
              Memberfunclist *x = Class_Mlookup(Cptr, $2);
              if (x == NULL) {
                  printf("%d: Method %s not declared\n", line, $2);
                  exit(1);
              } else if (x->isDefined == 1) {
                  printf("%d: Method %s redefined\n", line, $2);
                  exit(1);
              } else if (t != x->type || x->type != $8->type) {
                  printf("%d: Method %s decl|def|body type mismatch\n", line, $2);
                  exit(1);
              }
              t2 = x->paramlist;
          }

          int p = 0, selfp;
          while(t1 != NULL && t2 != NULL) {
              if (strcmp(t1->name, t2->name) || t1->type != t2->type) {
                  printf("%d: Func parameter mismatch: %s\n", line, $2);
                  exit(1);
              }
              t1 = t1->next; t2 = t2->next; p++;
          }

          if(t1 == NULL && t2 == NULL) {
              // binding assignment
              Lsymbol *t = LST; selfp = -(p + 3); 
              for (int i = 0; i < p; i++) {
                  t->binding = -(i + 3);
                  t = t->next;
              }
              
              
              p = 1;

              while (t != NULL) {
                  if (t->next == NULL && Cptr != NULL)
                      t->binding = selfp;
                  else 
                      t->binding = p++;
                  t = t->next;
              }

              if (Cptr == NULL)
                  GLookup($2)->size = 0;
              else 
                  Class_Mlookup(Cptr, $2)->isDefined = 1;
              
              
              $8->name = $2;
              $8->Lentry = LST;
              // $$ = $8;
              
              printT($8, 0);
              codeGen($8, fout);

              LST = NULL;
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
Param: ID Lid {
           $2->type = TCheck($1);
           $$ = (Paramstruct *)malloc(sizeof(Paramstruct));
           $$->name = $2->name;
           $$->type = TCheck($1);
           $$->next = NULL;
       }
     ;

LDeclBlock: DECL LDeclList ENDDECL {
                if (Cptr != NULL) LInstall("self", NULL);
            }
          | DECL ENDDECL {if (Cptr != NULL) LInstall("self", NULL);}
          | {if (Cptr != NULL) LInstall("self", NULL);}
          ;
LDeclList: LDeclList LDecl | LDecl ;
LDecl: ID LIdList ';' {
           Typetable *t = TCheck($1);
           for (Lsymbol *l = $2; l != NULL; l = l->next)
               l->type = t;
       } ;
LIdList: LIdList ',' Lid { $$ = $1; } | Lid { $$ = $1; } ;

Lid: ID {$$ = LInstall($1, NULL);}
   | '*' ID {
         /*
         if (tStack[tLast] == T_STR)ë
             $$ = LInstall($2, T_STRP);
         else if (tStack[tLast] == T_INT)
             $$ = LInstall($2, T_INTP);
         */
     }
   ;

Body: BGN Slist ReturnStmt END {
          // $$ = $2; $$->nodetype = N_BODY; $$->type = returnType;
          $$ = TreeCreate(returnType, N_BODY, NULL, (Constant) 0, NULL, $2, $3, NULL);
      } 
    | BGN ReturnStmt END {
          $$ = TreeCreate(returnType, N_BODY, NULL, (Constant) 0, NULL, $2, NULL, NULL);
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
           if ($$->type == NULL && $$->ctype != NULL) {
               printf("%d: Fields %s cannot be accessed outside class %s\n", line, $3, $$->ctype->name);
               exit(1);
           }
           $$->ptr2 = TreeCreate(NULL, N_FIELD, $3, (Constant) 0, $$, NULL, NULL, NULL);
           
           if ( $$->ptr2->type != NULL ) {
               $$->type = $$->ptr2->type;
               $$->ctype = NULL;
           } else { 
               $$->ctype = $$->ptr2->ctype;
               $$->type = NULL;
           }

           $$->nodetype = N_FIELD;
       }
     | FIELD '.' ID {
           $$ = $1;
           if ($$->type == NULL && $$->ctype != NULL) {
               printf("%d: Field %s cannot be accessed outside class %s\n", line, $3, $$->ctype->name);
               exit(1);
           }

           ASTNode *x = TreeCreate(NULL, N_FIELD, $3, (Constant) 0, $$, NULL, NULL, NULL);

           ASTNode *t = $$->ptr2;
           for (; t->ptr2 != NULL; t = t->ptr2);
           t->ptr2 = x;

           $$->type = x->type;
           $$->ctype = NULL;
       }
     | SELF '.' ID {
           if (Cptr == NULL) {
               printf("%d: Use of self outside class methods is not allowed\n", line);
               exit(1);
           }

           $$ = TreeCreate(NULL, N_ID, $1, (Constant) 0, NULL, NULL, NULL, NULL);
           $$->ptr2 = TreeCreate(NULL, N_FIELD, $3, (Constant) 0, $$, NULL, NULL, NULL);
           
           if ( $$->ptr2->type != NULL ) {
               $$->type = $$->ptr2->type;
               $$->ctype = NULL;
           } else {
               $$->ctype = $$->ptr2->ctype;
               $$->type = NULL;
           }
           $$->nodetype = N_FIELD;
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
               if ($1->type == NULL || $1->type->fields == NULL) {
                   printf("%d: Cant use alloc() with non userdefined types\n", line);
                   exit(1);
               }
               $$ = TreeCreate(NULL, N_ALLOC, NULL, (Constant) 0, NULL, $1, NULL, NULL);
           }
         | Assignable '=' NEW '(' ID ')' ';' {
               Classtable *c = CLookup($5);
               if ($1->ctype == NULL) {
                   printf("%d: Cannot use new() with non-objects\n", line);
                   exit(1);
               } else if (c == NULL) {
                   printf("%d: Unknown class %s\n", line, $5);
                   exit(1);
               } else if (!isChild($1->ctype, c)) {
                   printf("%d: Class %s is not a descendant of class %s\n", line, 
                   c->name, $1->ctype->name);
                   exit(1);
               }
               $$ = TreeCreate(NULL, N_ALLOC, NULL, (Constant) 0, NULL, $1, NULL, NULL);
               $$->ctype = CLookup($5);
           } 
         ;

FreeStmt: FREE '(' Assignable ')' ';' {
              if ($3->type == NULL || $3->type->fields == NULL) {
                   printf("%d: Cant use free() with non userdefined types\n", line);
                   exit(1);
              }
              $$ = TreeCreate(NULL, N_FREE, NULL, (Constant) 0, NULL, $3, NULL, NULL);
              $3->value.intval = 1;
          }
        | DEL '(' Assignable ')' ';' {
              if ($3->ctype == NULL) {
                  printf("%d: Cannot use delete() with non-objects\n", line);
                  exit(1);
              }
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
    |   FieldFunction {$$ = $1;}
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

void stackSetup() {
    fprintf(fout, "L0:\n");
    Classtable *c = CT; Memberfunclist *m;

    while (c != NULL) {
        m = c->vfuncptr;
        while (m != NULL) {
            fprintf(fout, "MOV [%d], F%d\n", 
            4096 + c->classIndex*8 + m->funcPosition, m->flabel);
            m = m->next;
        }
        c = c->next;
    }
                   
    fprintf(fout, "MOV SP, %d\nPUSH R0\nCALL F0\nPOP R0\nMOV R0, \"Exit\"\nPUSH R0\nMOV R0, 10\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n", 4095 + gstSize);
}

int main(int argc, char **args) {
    if (argc > 1) 
        yyin = fopen(args[1], "r");
    fout = fopen("program.xsm", "w");
    TypeTableCreate();
    char *m = "main";
    GST = (Gsymbol *)malloc(sizeof(Gsymbol));
    *GST = (Gsymbol) {m, TLookup("int"), NULL, -1, -1, NULL, 0, NULL};

    fprintf(fout, "0\n2056\n0\n0\n0\n0\n0\n0\nJMP L0\n");
    yyparse();
    return 0;
}

