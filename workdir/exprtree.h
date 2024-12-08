enum {
    T_BOOL, T_INT, T_STR, T_INTP, T_STRP
};

enum {
    N_CONST, N_ID, N_PTR,
    N_PLUS, N_MINUS, N_MUL, N_DIV, N_MOD,
    N_GT, N_LT, N_GE, N_LE, N_EQ, N_NE,
    N_OR, N_AND,
    N_IF, N_BREAK, N_CONTINUE, 
    N_WHILE, N_REPEAT, N_DOWHILE, 
    N_READ, N_WRITE, N_ASGN,
    N_SLIST, N_BODY, N_RET, N_FUNC, N_MAIN,
    N_FIELD, N_BRKP
};

typedef struct Paramstruct {
    char *name;
    int type;
    struct Paramstruct *next;
} Paramstruct;

typedef struct Gsymbol {
    char *name; 
    int type; 
    int size; 
    int binding; 
    Paramstruct *paramlist; 
    int flabel;
    struct Gsymbol *next; 
} Gsymbol;

Gsymbol* GInstall(char *name, int type, int size, Paramstruct *paramlist);
Gsymbol* GLookup(char *name);

typedef struct Lsymbol {
    char *name; 
    int type; 
    int binding;
    struct Lsymbol *next; 
} Lsymbol;

Lsymbol* LInstall(char *name, int type); 
Lsymbol* LLookup(char *name); 

typedef union {
    int intval;
    char *strval;
} Constant;

typedef struct ASTNode {
    int type;
    int nodetype;
    char *name;
    Constant value; 
    struct ASTNode *arglist;
    struct ASTNode *ptr1, *ptr2, *ptr3;
    Gsymbol *Gentry;
    Lsymbol *Lentry;
} ASTNode;

ASTNode* TreeCreate (
    int type,
    int nodetype,
    char *name,
    Constant value,
    ASTNode *arglist,
    ASTNode *ptr1,
    ASTNode *ptr2,
    ASTNode *ptr3
);

int evaluate(ASTNode *t);
void prefix(ASTNode *t, int d);
int codeGen(ASTNode *t, FILE* fp);

void printT(ASTNode *root, int level);
ASTNode *revArgList (ASTNode *root);
