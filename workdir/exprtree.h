typedef struct tnode{
 int val; //value of the expression tree
 int type;
 // char *varname;
 char varname;
 // int nodetype;
 char nodetype;
 struct tnode *left,*right; //left and right branches
} tnode;

struct tnode* createTree(int val, int type, char name, char nodetype, struct tnode *l, struct tnode *r);

/*
 * val -> value of NUM (leaf node)
 * varname -> [a-z]
 * nodetype -> read, write, connector, +, -, *, /, =, n, v
 *
 */

/*To evaluate an expression tree*/
int evaluate(struct tnode *t);

void prefix(struct tnode *t);
void postfix(struct tnode *t);
int codeGen(struct tnode *t, FILE* fp);
