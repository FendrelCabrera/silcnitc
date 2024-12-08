#define inttype 1
#define booltype 0
#define strtype 2
#define intptype 3
#define strptype 4

typedef struct tnode{
	int val; //value of the expression tree
	int type;
	char *varname;
	int nodetype;
	struct tnode *left, *right, *third; //left and right branches
    struct Gsymbol *Gentry;
} tnode;

struct Gsymbol {
    char *name;
    int type;
    int size;
    int binding;
    struct Gsymbol *next;
    int rowLength;
};

struct tnode* createTree(int val, int type, char* varname, int nodetype, struct tnode *l, struct tnode *r, struct tnode *t);

struct Gsymbol *Lookup(char *name); // Returns pointer to ST entry
void Install(char *name, int type, int size, int rowLength); // Creates ST entry

/*
 * val -> value of NUM (leaf node)
 * type -> expr type (int/bool)
 * varname -> [a-z]
 */

/* nodetype
 * connector - 0
 * read - 1
 * write - 2
 * plus - 3
 * minus - 4
 * mul - 5
 * div - 6
 * num - 7
 * var - 8
 * LT - 9
 * LE - 10
 * GT - 11
 * GE - 12
 * EQ (==) - 13
 * NE - 14
 * = (assignment) - 15
 * if - 16
 * ifelse - 17
 * while - 18
 * break - 19
 * continue - 20
 * repeat - 21
 * dowhile - 22
 * stringConstant - 23
 * % - 24
 * 1d array - 25
 * 2d array - 26
 * pointer - 27
*/

/*To evaluate an expression tree*/
int evaluate(struct tnode *t);
void prefix(struct tnode *t);
int codeGen(struct tnode *t, FILE* fp);
