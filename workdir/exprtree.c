struct tnode* makeLeafNode(int n)
{
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->op = NULL;
    temp->val = n;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode* makeOperatorNode(char c,struct tnode *l,struct tnode *r){
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->op = malloc(sizeof(char));
    *(temp->op) = c;
    temp->left = l;
    temp->right = r;
    return temp;
}

int evaluate(struct tnode *t){
    if(t->op == NULL)
    {
        return t->val;
    }
    else{
        switch(*(t->op)){
            case '+' : return evaluate(t->left) + evaluate(t->right);
                       break;
            case '-' : return evaluate(t->left) - evaluate(t->right);
                       break;
            case '*' : return evaluate(t->left) * evaluate(t->right);
                       break;
            case '/' : return evaluate(t->left) / evaluate(t->right);
                       break;
        }
    }
}

void prefix(struct tnode *t) {
	if (t == NULL)
		return;
	else if (t->op == NULL)
		printf("%d ", t->val);
	else {
		printf("%c ", *(t->op));
		prefix(t->left);
		prefix(t->right);
	}
}

void postfix(struct tnode *t) {
	if (t == NULL)
		return;
	else if (t->op == NULL)
		printf("%d ", t->val);
	else {
		postfix(t->left);
		postfix(t->right);
		printf("%c ", *(t->op));
	}
}

int lastUsed = -1;

int getReg() {
	return ++lastUsed;
}

void freeReg() {
	lastUsed--;
}

int codeGen(struct tnode *t, FILE* fp) {
	if (t->op == NULL) {
		int i = getReg();
		fprintf(fp, "MOV R%d, %d\n", i, t->val);
		return i;
	} else {
		int i = codeGen(t->left, fp);
		int j = codeGen(t->right, fp);

		switch(*(t->op)) {
			case '+':
				fprintf(fp, "ADD ");
				break;

			case '-':
				fprintf(fp, "SUB ");
				break;

			case '*':
				fprintf(fp, "MUL ");
				break;

			case '/':
				fprintf(fp, "DIV ");
				break;
		}
		fprintf(fp, "R%d, R%d\n", i, j);

		freeReg();
		return i;
	}
}
