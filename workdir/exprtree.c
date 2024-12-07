struct tnode* createTree(int val, int type, char name, char nodetype, struct tnode *l, struct tnode *r){
	struct tnode *x = (struct tnode *)malloc(sizeof(struct tnode));
	x->val = val;
	x->type = type;
	x->varname = name;
	x->nodetype = nodetype;
	x->left = l;
	x->right = r;
	return x;
}

void prefix(struct tnode *t) {
	if (t == NULL)
		return;
	prefix(t->left);
	prefix(t->right);
	printf("%c ", t->nodetype);
}

int lastUsed = -1;

int getReg() {
	return ++lastUsed;
}

void freeReg() {
	lastUsed--;
}

int codeGen(struct tnode *t, FILE* fp) {
	if (t->nodetype == 'c') {
		codeGen(t->left, fp);
		codeGen(t->right, fp);
	} else if(t->nodetype == 'r') {
		int adr = (t->left->varname - 'a') + 4096;
		fprintf(fp, "MOV R0, \"Read\"\nPUSH R0\nMOV R0, -1\nPUSH R0\nMOV R0, %d\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n", adr);
	} else if(t->nodetype == 'w') {
		codeGen(t->left, fp);
		fprintf(fp, "MOV R1, \"Write\"\nPUSH R1\nMOV R1, -2\nPUSH R1\nPUSH R0\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
		freeReg();
	} else if(t->nodetype == '=') {
		codeGen(t->right, fp);
		int adr = (t->left->varname - 'a') + 4096;
		fprintf(fp, "MOV [%d], R0\n", adr);
		freeReg();
	} else if(t->nodetype == 'v') {
		int i = getReg();
		int adr = (t->varname - 'a') + 4096;
		fprintf(fp, "MOV R%d, [%d]\n", i, adr);
		return i;
	} else if(t->nodetype == 'n') {
		int i = getReg();
		fprintf(fp, "MOV R%d, %d\n", i, t->val);
		return i;
	} else {
		int i = codeGen(t->left, fp);
		int j = codeGen(t->right, fp);

		switch(t->nodetype) {
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

int arr[26];

int evaluate(struct tnode *t) {
	if (t->nodetype == 'c') {
		evaluate(t->left);
		evaluate(t->right);
	} else if(t->nodetype == 'r') {
		int adr = (t->left->varname - 'a');
		scanf("%d", arr + adr);
	} else if(t->nodetype == 'w') {
		int op = evaluate(t->left);
		printf("%d\n", op);
	} else if(t->nodetype == '=') {
		int op = evaluate(t->right);
		int adr = (t->left->varname - 'a');
		arr[adr] = op;
	} else if(t->nodetype == 'v') {
		return arr[t->varname - 'a'];
	} else if(t->nodetype == 'n') {
		return t->val;
	} else {
		int i = evaluate(t->left);
		int j = evaluate(t->right);

		switch(t->nodetype) {
			case '+':
				return i + j;
				break;

			case '-':
				return i - j;
				break;

			case '*':
				return i * j;
				break;

			case '/':
				return i/j;
				break;
		}
	}
}

