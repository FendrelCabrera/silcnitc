int line = 1;

Typetable *TT = NULL;
Typetable *TInstall (char *name, int size, Fieldlist *fields) {
    Typetable *x = TT, *y;
    while (x != NULL) {
        if (strcmp(x->name, name) == 0) {
            printf("%d: Type %s is already defined\n", line, name);
            exit(1);
        }

        if (x->next == NULL) break;
        x = x->next;
    }

    y = (Typetable *)malloc(sizeof(Typetable));
    *y = (Typetable) {
        name, size, fields, NULL
    };

    if (x == NULL) TT = y;
    else x->next = y;

    return y;
}

void TypeTableCreate() {
    TInstall("int", 1, NULL);
    TInstall("str", 1, NULL);
    TInstall("bool", 0, NULL);
    TInstall("void", 0, NULL);
}

Typetable *TLookup (char *name) {
    Typetable *t = TT;
    while (t != NULL) {
        if (strcmp(t->name, name) == 0)
            return t;
        t = t->next;
    } 
    return NULL;
}

Fieldlist *FLookup (Typetable *type, char *name) {
    Fieldlist *f = type->fields;
    while (f != NULL) {
        if (strcmp(f->name, name) == 0)
            return f;
        f = f->next;
    }
    return NULL;
}

int GetSize (Typetable *type) {
    int i = 0;
    for ( Fieldlist *f = type->fields; f != NULL; f = f->next) i++;
    return (i == 0)?(1):(i);
}

Gsymbol *GST = NULL;
int gstSize = 0;
int fcount = 0;

Gsymbol *GInstall (char *name, Typetable *type, int size, Paramstruct *paramlist) {
    Gsymbol *temp = GST;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            printf("Redeclared Id: %s\n", name);
            exit(1);
        } else if(temp->next == NULL)
            break;
        temp = temp->next;
    }
    
    Gsymbol *x = (Gsymbol *)malloc(sizeof(Gsymbol));
    // name, type, size, binding, paramlist, flabel, next
    *x = (Gsymbol) {
        name, type, size, -1, paramlist, -1, NULL
    };

    if (size == 0) { 
        x->flabel = ++fcount;
        x->size = -1;
    }
    
    if (size > 0) {
        x->binding = 4096 + gstSize;
        gstSize += size;
    }

    if (GST == NULL) GST = x;
    else temp->next = x;

    return x;
}
    
Gsymbol *GLookup(char *name) {
    Gsymbol *temp = GST;
    
    while(temp != NULL) {
        if(strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

Lsymbol *LST = NULL;
Lsymbol *LInstall(char *name, Typetable *type) {
    Lsymbol *temp = LST;
    int len = 1;
    while(temp != NULL) {
        if(strcmp(temp->name, name) == 0) {
            printf("Redeclared variable: %s\n", name);
            exit(1);
        } else if (temp->next == NULL)
            break;

        temp = temp->next;
        len++;
    }

    Lsymbol *x = (Lsymbol *)malloc(sizeof(Lsymbol));
    // name, type, binding, next
    *x = (Lsymbol) { name, type, 0, NULL };
    
    if (temp == NULL) {
        x->binding = 0;
        LST = x;
    } else {
        x->binding = len;
        temp->next = x;
    }

    return x;
}

Lsymbol *LLookup(char *name) {
    Lsymbol *temp = LST;
    
    while(temp != NULL) {
        if(strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

ASTNode *TreeCreate (Typetable *type, int nodetype, char *name, Constant value, ASTNode *arglist, ASTNode *ptr1, ASTNode *ptr2, ASTNode *ptr3) {
    if (type == NULL) type = TLookup("void");
    Gsymbol *Gentry = NULL;
    Lsymbol *Lentry = NULL;

    // ID lookup
    if (nodetype == N_ID || nodetype == N_PTR) {
        Lentry = LLookup(name); Gentry = GLookup(name);

        if (ptr1 == NULL) {
            if (Lentry != NULL)
                type = Lentry->type;
            else if (Gentry == NULL) {
                printf("%d: Undeclared variable: '%s'\n", line, name);
                exit(1);
            } else if (Gentry->size > 1) {
                printf("%d: Invalid var access for array: %s\n", line, name);
                exit(1);
            } else
                type = Gentry->type;
        } else if (Gentry == NULL && Lentry != NULL) {
            printf("%d: Invalid array access for local var: %s\n", line, name);
            exit(1);
        } else if (Gentry != NULL) {
            if (Gentry->size == 1) {
                printf("%d: Invalid array access for global var: %s\n", line, name);
                exit(1);
            } else
                type = Gentry->type;
        } else {
            printf("%d: Undeclared array: %s\n", line, name);
            exit(1);
        }

        /*
        if (type == T_INTP || type == T_STRP) {
            if (nodetype == N_ID || type == T_INTP)
                type = T_INT;
            else if (type == T_STRP)
                type = T_STR;
        } else if (nodetype == N_PTR) {
            printf("Invalid pointer operation for var %s\n", name);
            exit(1);
        }
        */
    } else if (nodetype == N_FUNC) {
        Gentry = GLookup(name);
        if (Gentry == NULL) {
            printf("%d: Undeclared variable: '%s'\n", line, name);
            exit(1);
        } else if (Gentry->size > 0) {
            printf("%d: Invalid func call for var: '%s'\n", line, name);
            exit(1);
        } 

        type = Gentry->type;
    } else if (nodetype == N_FIELD) {
        Fieldlist *f = FLookup(type, name);
        if (f == NULL) {
            printf("%d: %s is not a field of type %s\n", line, name, type->name);
            exit(1);
        }
        type = f->type;
    }

    // type checking
    if (nodetype == N_WRITE && (ptr1->type != TLookup("int") && ptr1->type != TLookup("str"))) {
        printf("%d: Type mismatch: Writing %s\n", line, ptr1->type->name);
        exit(1);
    } else if (nodetype == N_ASGN && 
               ptr1->type != ptr2->type && 
               (ptr1->type->fields == NULL || ptr2->type != TLookup("void"))
              ) {
        printf("%d: Type mismatch: Assigning %s to %s\n", line, ptr2->type->name, ptr1->type->name);
        exit(1);
    } else if ((nodetype == N_IF || nodetype == N_WHILE) && ptr1->type != TLookup("bool")) {
        printf("%d: Type mismatch: IF/WHILE condition not bool\n", line);
        exit(1);
    } else if ((nodetype == N_REPEAT || nodetype == N_DOWHILE) && ptr2->type != TLookup("bool")) {
        printf("%d: Type mismatch: REPEAT/DOWHILE condition not bool\n", line);
        exit(1);
    } else if (N_PLUS <= nodetype && nodetype <= N_MOD) {
        if (ptr1->type != TLookup("int") || ptr2->type != TLookup("int")) {
            printf("%d: Type mismatch: Arithmetic operation b/w %s and %s\n", line, ptr1->type->name, ptr2->type->name);
            exit(1);
        }
    } else if (
            (nodetype == N_EQ || nodetype == N_NE) && 
             ptr1->type != ptr2->type && 
            (ptr1->type->fields == NULL || ptr2->type != TLookup("void")) &&
            (ptr1->type != TLookup("void") || ptr2->type->fields == NULL)
            ){
        printf("%d: Type mismatch: Comparision b/w %s and %s\n", line, ptr1->type->name, ptr2->type->name);
        exit(1);
    } else if (N_GT <= nodetype && nodetype <= N_LE && ptr1->type != ptr2->type) {
        printf("%d: Type mismatch: Comparision b/w %s and %s\n", line, ptr1->type->name, ptr2->type->name);
        exit(1);
    } else if (N_OR <= nodetype && nodetype <= N_AND) {
        if (ptr1->type != TLookup("bool") || ptr2->type != TLookup("bool")) {
            printf("%d: Type mismatch: Logical operation b/w %s and %s\n", line, ptr1->type->name, ptr2->type->name);
            exit(1);
        }
    } else if (nodetype == N_ID && ptr1 != NULL && ptr1->type != TLookup("int")) {
        printf("%d: Type mismatch: Array index not integer\n", line);
        exit(1);
    } else if (nodetype == N_FUNC) {
        ASTNode *t = arglist;
        Paramstruct *p = Gentry->paramlist;

        while (t != NULL && p != NULL) {
            if (t->type != p->type) {
                printf("%d: Type mismatch: Expected %s but received %s for argument '%s' of function '%s'\n", line, p->type->name, t->type->name, p->name, name);
                exit(1);
            }

            t = t->ptr3;
            p = p->next;
        }

        if (t != NULL) {
            printf("%d: Extra parameter supplied for function '%s'\n", line, name);
            exit(1);
        } else if (p != NULL) {
            printf("%d: Missing parameter '%s' for function '%s'\n", line, p->name, name);
            exit(1);
        }
    } else if (nodetype == N_ALLOC || nodetype == N_FREE) {
        if (ptr1->type->fields == NULL) {
            printf("%d: Cannot use memAlloc funcs for non-user defined type\n", line);
            exit(1);
        }
    }

    ASTNode *x = (ASTNode *)malloc(sizeof(ASTNode));
    *x = (ASTNode) {
        type, nodetype, name, value, arglist, ptr1, ptr2, ptr3, Gentry, Lentry
    };
    return x;
}

void prefix(ASTNode *t, int d) {
	if (t == NULL)
		return;

    prefix(t->ptr1, d + 1);
	prefix(t->ptr2, d + 1);
	prefix(t->ptr3, d + 1);

    for(int i = 0; i < d; i++)
        printf(" ");
	printf("%d\n", t->nodetype);
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

int funcRegStack[20];
int frsSize = 0;


int codeGen (ASTNode *t, FILE* fp) {
    if (t->nodetype == N_CONST) {
        int i = getReg();
        if (t->type == TLookup("str"))
            fprintf(fp, "MOV R%d, %s\n", i, t->value.strval);
        else if (t->type == TLookup("int"))
            fprintf(fp, "MOV R%d, %d\n", i, t->value.intval);
        else 
            fprintf(fp, "MOV R%d, \"null\"\n", i);
        return i;
    } else if (t->nodetype == N_ID) {
        int i = getReg();
        if (t->Lentry != NULL) {
            int j = getReg();
            fprintf(fp, "MOV R%d, BP\n", i);
            fprintf(fp, "MOV R%d, %d\n", j, t->Lentry->binding);
            fprintf(fp, "ADD R%d, R%d\n", i, j);
            freeReg();
        } else {
            fprintf(fp, "MOV R%d, %d\n", i, t->Gentry->binding);
            
            if (t->ptr1 != NULL) {
                int j = codeGen(t->ptr1, fp);
                if (t->ptr2 == NULL)
                    fprintf(fp, "ADD R%d, R%d\n", i, j);
                else {

                }
                freeReg();
            }
        }
        
        if (t->value.intval == 1)
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        return i;
    } else if (t->nodetype == N_PTR) {
        int i = getReg();
        if (t->Lentry != NULL) {
            int j = getReg();
            fprintf(fp, "MOV R%d, BP\n", i); 
            fprintf(fp, "MOV R%d, %d\n", j, t->Lentry->binding);
            fprintf(fp, "ADD R%d, R%d\n", i, j); 
            freeReg();
        } else {
            fprintf(fp, "MOV R%d, %d\n", i, t->Gentry->binding);
    
            if (t->ptr1 != NULL) {
                int j = codeGen(t->ptr1, fp);
                if (t->ptr2 == NULL)
                    fprintf(fp, "ADD R%d, R%d\n", i, j); 
                else {

                }
                freeReg();
            }
        }
    
        fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        if (t->value.intval == 1)
            fprintf(fp, "MOV R%d, [R%d]\n", i, i); 
        return i;
    } else if (N_PLUS <= t->nodetype && t->nodetype <= N_MOD) {
        int i = codeGen(t->ptr1, fp);
        int j = codeGen(t->ptr2, fp);

        if (t->nodetype == N_PLUS)
            fprintf(fp, "ADD ");
        else if (t->nodetype == N_MINUS)
            fprintf(fp, "SUB ");
        else if (t->nodetype == N_MUL)
            fprintf(fp, "MUL ");
        else if (t->nodetype == N_DIV)
            fprintf(fp, "DIV ");
        else if (t->nodetype == N_MOD)
            fprintf(fp, "MOD ");
        
        fprintf(fp, "R%d, R%d\n", i, j);
        freeReg();
        return i;
    } else if (N_GT <= t->nodetype && t->nodetype <= N_NE) {
        int i = codeGen(t->ptr1, fp);
        int j = codeGen(t->ptr2, fp);

        if (t->nodetype == N_GT)
            fprintf(fp, "GT ");
        else if (t->nodetype == N_LT)
            fprintf(fp, "LT ");
        else if (t->nodetype == N_GE)
            fprintf(fp, "GE ");
        else if (t->nodetype == N_LE)
            fprintf(fp, "LE ");
        else if (t->nodetype == N_EQ)
            fprintf(fp, "EQ ");
        else if (t->nodetype == N_NE)
            fprintf(fp, "NE ");
        
        fprintf(fp, "R%d, R%d\n", i, j);
        freeReg();
        return i;
    } else if (t->nodetype == N_OR) {
        int i = codeGen(t->ptr1, fp);
        int j = codeGen(t->ptr2, fp);

        fprintf(fp, "ADD R%d, R%d\nMOV R%d, 0\nNE R%d, R%d\n", i, j, j, i, j);
        freeReg();
        return i;
    } else if (t->nodetype == N_AND) {
        int i = codeGen(t->ptr1, fp);
        int j = codeGen(t->ptr2, fp);

        fprintf(fp, "MUL R%d, R%d\nMOV R%d, 0\nNE R%d, R%d\n", i, j, j, i, j);
        freeReg();
        return i;
    } else if (t->nodetype == N_IF) {
        int l1 = getLabel();
        codeGen(t->ptr1, fp);
        fprintf(fp, "JZ R0, L%d\n", l1);
        freeReg();
        codeGen(t->ptr2, fp);

        if (t->ptr3 == NULL)
            fprintf(fp, "L%d:\n", l1);
        else {
            int l2 = getLabel();
            fprintf(fp, "JMP L%d\nL%d:\n", l2, l1);
            codeGen(t->ptr3, fp);
            fprintf(fp, "L%d:\n", l2);
        }
    } else if (t->nodetype == N_BREAK) {
        if (stackSize > 0)
            fprintf(fp, "JMP L%d\n", loopStack[stackSize - 1][1]);
    } else if(t->nodetype == N_CONTINUE) {  
        if (stackSize > 0)
            fprintf(fp, "JMP L%d\n", loopStack[stackSize - 1][0]);
    } else if (t->nodetype == N_WHILE) {
        int l1 = getLabel();
        int l2 = getLabel();

        fprintf(fp, "L%d:\n", l1);
        codeGen(t->ptr1, fp);
        fprintf(fp, "JZ R0, L%d\n", l2);
        freeReg();

        loopStack[stackSize][0] = l1;
        loopStack[stackSize][1] = l2;
        stackSize++;

        codeGen(t->ptr2, fp);

        stackSize--;

        fprintf(fp, "JMP L%d\nL%d:\n", l1, l2);
    } else if (t->nodetype == N_REPEAT) { 
        int l1 = getLabel(), l2 = getLabel(), l3 = getLabel();
        fprintf(fp, "L%d:\n", l1);
        loopStack[stackSize][0] = l2;
        loopStack[stackSize][1] = l3;
        stackSize++;
        codeGen(t->ptr1, fp);
        stackSize--;
        fprintf(fp, "L%d:\n", l2);
        codeGen(t->ptr2, fp);
        fprintf(fp, "JZ R0, L%d\nL%d:\n", l1, l3);
        freeReg();
    } else if (t->nodetype == N_DOWHILE) { 
        int l1 = getLabel(), l2 = getLabel(), l3 = getLabel();
        fprintf(fp, "L%d:\n", l1);
        loopStack[stackSize][0] = l2;
        loopStack[stackSize][1] = l3;
        stackSize++;
        codeGen(t->ptr1, fp);
        stackSize--;
        fprintf(fp, "L%d:\n", l2);
        codeGen(t->ptr2, fp);
        fprintf(fp, "JNZ R0, L%d\nL%d:\n", l1, l3);
        freeReg();
    } else if (t->nodetype == N_READ) {
        codeGen(t->ptr1, fp);
        fprintf(fp, "MOV R1, \"Read\"\nPUSH R1\nMOV R1, -1\nPUSH R1\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
        freeReg();
    } else if (t->nodetype == N_WRITE) {
        codeGen(t->ptr1, fp);
		fprintf(fp, "MOV R1, \"Write\"\nPUSH R1\nMOV R1, -2\nPUSH R1\nPUSH R0\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
		freeReg();
    } else if (t->nodetype == N_ASGN) {
        codeGen(t->ptr1, fp);
        codeGen(t->ptr2, fp);
        fprintf(fp, "MOV [R0], R1\n");
        freeReg();
        freeReg();
    } else if (t->nodetype == N_SLIST) {
        codeGen(t->ptr1, fp);
        codeGen(t->ptr2, fp);
    } else if (t->nodetype == N_BODY || t->nodetype == N_MAIN) {
        if (t->nodetype == N_MAIN)
            fprintf(fp, "F0:\n");
        else
            fprintf(fp, "F%d:\n", t->Gentry->flabel);

        fprintf(fp, "PUSH BP\nMOV BP, SP\n");

        for (Lsymbol *l = t->Lentry; l != NULL; l = l->next) {
            if (l->binding > 0)
                fprintf(fp, "PUSH R0\n");
        }
        LST = t->Lentry;

        codeGen(t->ptr1, fp);
        codeGen(t->ptr2, fp);
    } else if (t->nodetype == N_RET) {
        codeGen(t->ptr1, fp);
        for (Lsymbol *l = LST; l != NULL; l = l->next) {
            if (l->binding > 0)
                fprintf(fp, "POP R1\n");
        }
        fprintf(fp, "MOV R1, BP\nMOV R2, -2\nADD R1, R2\nMOV [R1], R0\nPOP BP\nRET\n");
        freeReg();
    } else if (t->nodetype == N_FUNC) {
        // Pushing registers in use
        funcRegStack[frsSize++] = lastUsedReg;
        for (int i = 0; i <= lastUsedReg; i++)
            fprintf(fp, "PUSH R%d\n", i);
        lastUsedReg = -1;
       
        // Evaluating and pushing arguments
        t->arglist = revArgList(t->arglist);
        for (
            ASTNode *arg = t->arglist;
            arg != NULL; arg = arg->ptr3
        ) {
            codeGen(arg, fp);
            fprintf(fp, "PUSH R0\n");
            freeReg();
        }

        fprintf(fp, "PUSH R0\n"); // space for return value
        fprintf(fp, "CALL F%d\n", t->Gentry->flabel);
        
        lastUsedReg = funcRegStack[--frsSize];
        int j = getReg();
        fprintf(fp, "POP R%d\n", j); // store return value

        // Pop and discard arguments
        int popReg = (j == 0)?(1):(0);
        for (
            ASTNode *arg = t->arglist;
            arg != NULL; arg = arg->ptr3
        ) fprintf(fp, "POP R%d\n", popReg);

        // Pop and restore registers in use
        for (int i = j - 1; i >= 0; i--)
            fprintf(fp, "POP R%d\n", i);
        return j;
    } else if (t->nodetype == N_FIELD) {
        int i = getReg();
        Typetable *tp;
        if (t->Lentry != NULL) {
            int j = getReg();
            fprintf(fp, "MOV R%d, BP\n", i);
            fprintf(fp, "MOV R%d, %d\n", j, t->Lentry->binding);
            fprintf(fp, "ADD R%d, R%d\n", i, j);
            freeReg();
            tp = t->Lentry->type;
        } else {
            fprintf(fp, "MOV R%d, %d\n", i, t->Gentry->binding);
            
            if (t->ptr1 != NULL) {
                int j = codeGen(t->ptr1, fp);
                if (t->ptr2 == NULL)
                    fprintf(fp, "ADD R%d, R%d\n", i, j);
                else {

                }
                freeReg();
            }

            tp = t->Gentry->type;
        }

        ASTNode *y = t->ptr3;
        do {
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
            Fieldlist *f = FLookup(tp, y->name);
            fprintf(fp, "ADD R%d, %d\n", i, f->fieldIndex);

            tp = y->type;
            y = y->ptr3;
        } while (y != NULL);
        
        if (t->value.intval == 1)
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);
        return i;
    } else if (t->nodetype == N_BRKP) 
        fprintf(fp, "BRKP\n");
    else if (t->nodetype == N_INIT)
        fprintf(fp, "MOV R0, \"Heapset\"\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
    else if (t->nodetype == N_ALLOC) {
        getReg();
        fprintf(fp, "MOV R1, \"Alloc\"\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\n");
        codeGen(t->ptr1, fp);
        fprintf(fp, "MOV [R1], R0\n");
        freeReg();
        freeReg();
    } else if (t->nodetype == N_FREE) {
        int i = codeGen(t->ptr1, fp);
        fprintf(fp, "MOV R1, \"Free\"\nPUSH R1\nPUSH R0\nPUSH R1\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
        freeReg();
    }
}

/*
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
*/
int arr[26];
int brk = 0, cnt = 0;

/*
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
*/
