int line;
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
    N_SLIST, N_BODY, N_RET, N_FUNC, 
    N_FIELD, N_METHOD, 
    N_INIT, N_ALLOC, N_FREE,
    N_BRKP
};

typedef struct Fieldlist {
    char *name;
    int fieldIndex;
    struct Typetable *type;
    struct Classtable *ctype;
    struct Fieldlist *next;
} Fieldlist;

typedef struct Typetable {
    char *name;
    int size;
    struct Fieldlist *fields;
    struct Typetable *next;
} Typetable;

void TypeTableCreate();
Typetable *TLookup (char *name);
Typetable *TInstall (char *name, int size, Fieldlist *fields);
Fieldlist *FLookup (Typetable *type, char *name);
int GetSize (Typetable *type);

typedef struct Paramstruct {
    char *name;
    Typetable *type;
    struct Paramstruct *next;
} Paramstruct;

typedef struct Memberfunclist {
    char *name;
    Typetable * type;
    Paramstruct *paramlist;
    int funcPosition;
    int flabel;
    struct Memberfunclist *next;
    int isDefined;
} Memberfunclist;

typedef struct Classtable {
    char *name;
    Fieldlist *memberfield;
    Memberfunclist *vfuncptr;
    struct Classtable *parentPtr;
    int classIndex;
    int fieldCount;
    int methodCount;
    struct Classtable *next;
} Classtable;

Classtable* CInstall(char *name,char *parent_class_name);
Classtable* CLookup(char *name);
void Class_Finstall(Classtable *cptr, char *typename, char *name);
void Class_Minstall(Classtable *cptr, char *name, Typetable *type, Paramstruct *Paramlist);
Memberfunclist* Class_Mlookup(Classtable* Ctype,char* Name);
Fieldlist* Class_Flookup(Classtable* Ctype,char* Name);

typedef struct Gsymbol {
    char *name; 
    Typetable *type; 
    Classtable *ctype;
    int size; 
    int binding; 
    Paramstruct *paramlist; 
    int flabel;
    struct Gsymbol *next; 
} Gsymbol;

Gsymbol* GInstall(char *name, Typetable *type, Classtable *ctype, int size, Paramstruct *paramlist);
Gsymbol* GLookup(char *name);

typedef struct Lsymbol {
    char *name; 
    Typetable *type; 
    int binding;
    struct Lsymbol *next; 
} Lsymbol;

Lsymbol* LInstall(char *name, Typetable *type); 
Lsymbol* LLookup(char *name); 

typedef union {
    int intval;
    char *strval;
} Constant;

typedef struct ASTNode {
    Typetable *type;
    Classtable *ctype;
    int nodetype;
    char *name;
    Constant value; 
    struct ASTNode *arglist;
    struct ASTNode *ptr1, *ptr2, *ptr3;
    Gsymbol *Gentry;
    Lsymbol *Lentry;
} ASTNode;

ASTNode* TreeCreate (
    Typetable *type,
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
