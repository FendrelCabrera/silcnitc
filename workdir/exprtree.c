struct tnode* createTree(int val, int type, char* varname, int nodetype, struct tnode *l, struct tnode *r, struct tnode *t){
    // Check if variable exists
    struct Gsymbol *y = NULL;
    if (nodetype == 8 || nodetype == 25 || nodetype == 26 || nodetype == 27) {
        y = Lookup(varname);
        if (y == NULL) {
            printf("Variable not declared\n");
            exit(1);
        } else if(nodetype == 8 && y->rowLength != -1) {
            printf("Invalid var access for array\n");
            exit(1);
        } else if(nodetype == 25 && y->rowLength != y->size) {
            printf("Invalid 1d access for variable\n");
            exit(1);
        } else if(nodetype == 26 && y->rowLength >= y->size) {
            printf("Invalid 2d access for variable\n");
            exit(1);
        }

        if (nodetype == 27) {
            if(y->type == 3)
                type = 1;
            else if(y->type == 4)
                type = 2;
            else {
                printf("Reference for non-pointer not allowed\n");
                exit(1);
            }
        } else if(y->type == 3 || y->type == 4)
            type = 1;
        else
            type = y->type;
    }

    // Expression Type Checking
	if(
            (
             (type == 0 || type == 1) && 
              l != NULL && r != NULL && 
             (l->type != 1 || r->type != 1)
            ) || 
            ((16 <= nodetype && nodetype <= 18) && l->type != 0) || 
            (nodetype == 2 && l->type != 1 && l->type != 2) ||
            (nodetype == 15 && l->type != r->type) ||
            ((21 <= nodetype && nodetype <= 22) && r->type != 0) ||
            (nodetype == 25 && l->type != 1) ||
            (nodetype == 26 && (l->type != 1 || r->type != 1))
    ) {
        printf("Type mismatch\n");
        exit(1);
    }

	struct tnode *x = (struct tnode *)malloc(sizeof(struct tnode));
	x->val = val;
	x->type = type;
	x->varname = varname;
	x->nodetype = nodetype;
	x->left = l;
	x->right = r;
	x->third = t;
    x->Gentry = y;
	return x;
}

struct Gsymbol *ST = NULL;
int stSize = 0;

struct Gsymbol *Lookup(char *name) {
    struct Gsymbol *temp = ST;
    
    while(temp != NULL) {
        if(strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

void Install(char *name, int type, int size, int rowLength) {
    struct Gsymbol *temp = ST;

    while(temp != NULL) {
        if(strcmp(temp->name, name) == 0) {
            printf("Error: Redeclared variable\n");
            exit(1);
        }

        if(temp->next == NULL)
            break;
        temp = temp->next;
    }

    struct Gsymbol *x = (struct Gsymbol *)malloc(sizeof(struct Gsymbol));
    x->name = name;
    x->type = type;
    x->size = size;
    x->binding = 4096 + stSize;
    x->next = NULL;
    x->rowLength = rowLength;

    if(temp == NULL)
        ST = x;
    else
        temp->next = x;
    stSize += size;
}

void printStEntries() {
    struct Gsymbol *temp = ST;
    if(temp == NULL)
        return;

    printf("ST Entries:\n");
    while(temp != NULL) {
        printf("%s %d %d %d %d\n", temp->name, temp->type, temp->size, temp->binding, temp->rowLength);
        temp = temp->next;
    }
}

void prefix(struct tnode *t) {
	if (t == NULL)
		return;
	prefix(t->left);
	prefix(t->right);
	prefix(t->third);
	printf("%d ", t->nodetype);
}

int lastUsedReg = -1;

int getReg() {
	return ++lastUsedReg;
}

void freeReg() {
	lastUsedReg--;
}

int lastUsedLabel = -1;
int getLabel() {
    return ++lastUsedLabel;
}

int loopStack[100][2];
int stackSize = 0;

int codeGen(struct tnode *t, FILE* fp) {
	if (t->nodetype == 0) { // connector
		codeGen(t->left, fp);
		codeGen(t->right, fp);
	} else if(t->nodetype == 1) { // read
        codeGen(t->left, fp);
        fprintf(fp, "MOV R1, \"Read\"\nPUSH R1\nMOV R1, -1\nPUSH R1\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
        freeReg();
    } else if(t->nodetype == 2) { // write
		codeGen(t->left, fp);
		fprintf(fp, "MOV R1, \"Write\"\nPUSH R1\nMOV R1, -2\nPUSH R1\nPUSH R0\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
		freeReg();
	} else if(t->nodetype == 15) { // assg
		codeGen(t->left, fp);
        codeGen(t->right, fp);
        fprintf(fp, "MOV [R0], R1\n");
        freeReg();
        freeReg();
	} else if(t->nodetype == 8) { // var
        int i = getReg();
        if(t->val == 0)
            fprintf(fp, "MOV R%d, %d\n", i, t->Gentry->binding);
        else
            fprintf(fp, "MOV R%d, [%d]\n", i, t->Gentry->binding);
        return i;
    } else if(t->nodetype == 25) { // 1d array
        int i = codeGen(t->left, fp);
        fprintf(fp, "ADD R%d, %d\n", i, t->Gentry->binding);
        if(t->val == 1)
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        return i;
    } else if(t->nodetype == 26) { // 2d array
        int i = codeGen(t->left, fp);
        int j = codeGen(t->right, fp);
        fprintf(fp, "MUL R%d, %d\n", i, t->Gentry->rowLength);
        fprintf(fp, "ADD R%d, R%d\n", i, j);
        freeReg();
        fprintf(fp, "ADD R%d, %d\n", i, t->Gentry->binding);
        if(t->val == 1)
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        return i;
    } else if(t->nodetype == 27) { // pointer
        int i = getReg();
        fprintf(fp, "MOV R%d, [%d]\n", i, t->Gentry->binding);
        if(t->val == 1) 
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        return i;
	} else if(t->nodetype == 7) { // num
		int i = getReg();
		fprintf(fp, "MOV R%d, %d\n", i, t->val);
		return i;
    } else if(t->nodetype == 23) { // strc
        int i = getReg();
        fprintf(fp, "MOV R%d, %s\n", i, t->varname);
        return i;
	} else if(3 <= t->nodetype && t->nodetype <= 6 || t->nodetype == 24) { // arithmetic
		int i = codeGen(t->left, fp);
		int j = codeGen(t->right, fp);

		switch(t->nodetype) {
			case 3:
				fprintf(fp, "ADD ");
				break;

			case 4:
				fprintf(fp, "SUB ");
				break;

			case 5:
				fprintf(fp, "MUL ");
				break;

			case 6:
				fprintf(fp, "DIV ");
				break;

            case 24:
                fprintf(fp, "MOD ");
                break;
		}
		fprintf(fp, "R%d, R%d\n", i, j);

		freeReg();
		return i;
	} else if(9 <= t->nodetype && t->nodetype <= 14) { // logical
        int i = codeGen(t->left, fp);
        int j = codeGen(t->right, fp);

        switch(t->nodetype) {
            case 9:
                fprintf(fp, "LT ");
                break;

            case 10:
                fprintf(fp, "LE ");
                break;

            case 11:
                fprintf(fp, "GT ");
                break;

            case 12:
                fprintf(fp, "GE ");
                break;

            case 13:
                fprintf(fp, "EQ ");
                break;

            case 14:
                fprintf(fp, "NE ");
                break;
        }
        fprintf(fp, "R%d, R%d\n", i, j);
        freeReg();
        return i;
    } else if(t->nodetype == 18) { // while
        int l1 = getLabel();
        int l2 = getLabel();

        fprintf(fp, "L%d:\n", l1);
        codeGen(t->left, fp);
        fprintf(fp, "JZ R0, L%d\n", l2);
        freeReg();

        loopStack[stackSize][0] = l1;
        loopStack[stackSize][1] = l2;
        stackSize++;

        codeGen(t->right, fp);

        stackSize--;

        fprintf(fp, "JMP L%d\nL%d:\n", l1, l2);
    } else if(t->nodetype == 16) { // if
        int l = getLabel();
        codeGen(t->left, fp);
        fprintf(fp, "JZ R0, L%d\n", l);
        freeReg();
        codeGen(t->right, fp);
        fprintf(fp, "L%d:\n", l);
    } else if(t->nodetype == 17) { // ifelse
        int l1 = getLabel();
        int l2 = getLabel();

        codeGen(t->left, fp);
        fprintf(fp, "JZ R0, L%d\n", l1);
        freeReg();
        codeGen(t->right, fp);
        fprintf(fp, "JMP L%d\nL%d:\n", l2, l1);
        codeGen(t->third, fp);
        fprintf(fp, "L%d:\n", l2);
    } else if(t->nodetype == 19) { // break
        if (stackSize > 0)
            fprintf(fp, "JMP L%d\n", loopStack[stackSize - 1][1]);
    } else if(t->nodetype == 20) { // continue 
        if (stackSize > 0)
            fprintf(fp, "JMP L%d\n", loopStack[stackSize - 1][0]);
    } else if(t->nodetype == 21) { // repeat
        int l1 = getLabel(), l2 = getLabel(), l3 = getLabel();
        fprintf(fp, "L%d:\n", l1);
        loopStack[stackSize][0] = l2;
        loopStack[stackSize][1] = l3;
        stackSize++;
        codeGen(t->left, fp);
        stackSize--;
        fprintf(fp, "L%d:\n", l2);
        codeGen(t->right, fp);
        fprintf(fp, "JZ R0, L%d\nL%d:\n", l1, l3);
        freeReg();
    } else if(t->nodetype == 22) { // dowhile
        int l1 = getLabel(), l2 = getLabel(), l3 = getLabel();
        fprintf(fp, "L%d:\n", l1);
        loopStack[stackSize][0] = l2;
        loopStack[stackSize][1] = l3;
        stackSize++;
        codeGen(t->left, fp);
        stackSize--;
        fprintf(fp, "L%d:\n", l2);
        codeGen(t->right, fp);
        fprintf(fp, "JNZ R0, L%d\nL%d:\n", l1, l3);
        freeReg();
    }
}

int arr[26];
int brk = 0, cnt = 0;

int evaluate(struct tnode *t) {
	if (t->nodetype == 0) { // connector
        evaluate(t->left);
        if (brk == 0 && cnt == 0)
		    evaluate(t->right);
	} else if(t->nodetype == 1) { // read
		int adr = (*(t->left->varname) - 'a');
		scanf("%d", arr + adr);
	} else if(t->nodetype == 2) { // write
		int op = evaluate(t->left);
		printf("%d\n", op);
	} else if(t->nodetype == 15) { // assg
		int op = evaluate(t->right);
		int adr = (*(t->left->varname) - 'a');
		arr[adr] = op;
	} else if(t->nodetype == 8) { // var
		return arr[*(t->varname) - 'a'];
	} else if(t->nodetype == 7) { // num
		return t->val;
	} else if (3 <= t->nodetype && t->nodetype <= 6 || t->nodetype == 24){ // arithmetic
		int i = evaluate(t->left);
		int j = evaluate(t->right);

		switch(t->nodetype) {
			case 3:
				return i + j;

			case 4:
				return i - j;

			case 5:
				return i * j;

			case 6:
				return i/j;

            case 24: 
                return i % j;
		}
	} else if (9 <= t->nodetype && t->nodetype <= 14) { // logical
        int i = evaluate(t->left);
        int j = evaluate(t->right);

        switch(t->nodetype) {
            case 9:
                return i < j;

            case 10:
                return i <= j;

            case 11:
                return i > j;

            case 12:
                return i >= j;

            case 13:
                return i == j;

            case 14:
                return i != j;
        }
    } else if (t->nodetype == 16) { // if
        if (evaluate(t->left) == 1)
            evaluate(t->right);
    } else if (t->nodetype == 17) { // ifelse
        if (evaluate(t->left) == 1)
            evaluate(t->right);
        else
            evaluate(t->third);
    } else if (t->nodetype == 18) { // while
        stackSize++;
        while(evaluate(t->left) == 1 && brk == 0) {
            cnt = 0;
            evaluate(t->right);
        }
        cnt = 0; brk = 0;
        stackSize--;
    } else if(t->nodetype == 19 && stackSize > 0) // break
        brk = 1;
    else if(t->nodetype == 20 && stackSize > 0) // continue
        cnt = 1;
    else if(t->nodetype == 21) { // repeat
        stackSize++;
        do {
            cnt = 0;
            evaluate(t->left);
        } while(evaluate(t->right) == 0 && brk == 0);
        cnt = 0; brk = 0;
        stackSize--;
    } else if(t->nodetype == 22) { // dowhile
        stackSize++;
        do {
            cnt = 0;
            evaluate(t->left);
        } while(evaluate(t->right) == 1 && brk == 0);
        cnt = 0; brk = 0;
        stackSize--;
    }
}

