Gsymbol *GST = NULL;
int gstSize = 0;
int fcount = 0;

Gsymbol *GInstall (char *name, int type, int size, Paramstruct *paramlist) {
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
        name, type, size, (size > 0)?(4096 + gstSize):(-1), paramlist, -1, NULL
    };
    
    gstSize += size;
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
Lsymbol *LInstall(char *name, int type) {
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

ASTNode *TreeCreate (int type, int nodetype, char *name, Constant value, ASTNode *arglist, ASTNode *ptr1, ASTNode *ptr2, ASTNode *ptr3) {
    Gsymbol *Gentry = NULL;
    Lsymbol *Lentry = NULL;

    // ID lookup
    if (nodetype == N_ID || nodetype == N_PTR) {
        Lentry = LLookup(name); Gentry = GLookup(name);

        if (ptr1 == NULL) {
            if (Lentry != NULL)
                type = Lentry->type;
            else if (Gentry == NULL) {
                printf("Undeclared variable: '%s'\n", name);
                exit(1);
            } else if (Gentry->size > 1) {
                printf("Invalid var access for array: %s\n", name);
                exit(1);
            } else
                type = Gentry->type;
        } else if (Gentry == NULL && Lentry != NULL) {
            printf("Invalid array access for local var: %s\n", name);
            exit(1);
        } else if (Gentry != NULL) {
            if (Gentry->size == 1) {
                printf("Invalid array access for global var: %s\n", name);
                exit(1);
            } else
                type = Gentry->type;
        } else {
            printf("Undeclared array: %s\n", name);
            exit(1);
        }

        if (type == T_INTP || type == T_STRP) {
            if (nodetype == N_ID || type == T_INTP)
                type = T_INT;
            else if (type == T_STRP)
                type = T_STR;
        } else if (nodetype == N_PTR) {
            printf("Invalid pointer operation for var %s\n", name);
            exit(1);
        }
    } else if (nodetype == N_FUNC) {
        Gentry = GLookup(name);
        if (Gentry == NULL) {
            printf("Undeclared variable: '%s'\n", name);
            exit(1);
        } else if (Gentry->size != 0) {
            printf("Invalid func call for var: '%s'\n", name);
            exit(1);
        } 
        type = Gentry->type;
    }

    // type checking
    if (nodetype == N_WRITE && (ptr1->type != T_INT && ptr1->type != T_STR)) {
        printf("Type mismatch: Writing %d\n", ptr1->type);
        exit(1);
    } else if (nodetype == N_ASGN && ptr1->type != ptr2->type) {
        printf("Type mismatch: Assigning %d to %d\n", ptr2->type, ptr1->type);
        exit(1);
    } else if ((nodetype == N_IF || nodetype == N_WHILE) && ptr1->type != T_BOOL) {
        printf("Type mismatch: IF/WHILE condition not bool\n");
        exit(1);
    } else if ((nodetype == N_REPEAT || nodetype == N_DOWHILE) && ptr2->type != T_BOOL) {
        printf("Type mismatch: REPEAT/DOWHILE condition not bool\n");
        exit(1);
    } else if (N_PLUS <= nodetype && nodetype <= N_MOD) {
        if (ptr1->type != T_INT || ptr2->type != T_INT) {
            printf("Type mismatch: Arithmetic operation b/w %d and %d\n", ptr1->type, ptr2->type);
            exit(1);
        }
    } else if (N_GT <= nodetype && nodetype <= N_NE && ptr1->type != ptr2->type) {
        printf("Type mismatch: Comparision b/w %d and %d\n", ptr1->type, ptr2->type);
        exit(1);
    } else if (N_OR <= nodetype && nodetype <= N_AND) {
        if (ptr1->type != T_BOOL || ptr2->type != T_BOOL) {
            printf("Type mismatch: Logical operation b/w %d and %d\n", ptr1->type, ptr2->type);
            exit(1);
        }
    } else if (nodetype == N_ID && ptr1 != NULL && ptr1->type != T_INT) {
        printf("Type mismatch: Array index not integer\n");
        exit(1);
    } else if (nodetype == N_FUNC) {
        ASTNode *t = arglist;
        Paramstruct *p = Gentry->paramlist;

        while (t != NULL && p != NULL) {
            if (t->type != p->type) {
                printf("Type mismatch: Expected %d but received %d for argument '%s' of function '%s'\n", p->type, t->type, p->name, name);
                exit(1);
            }

            t = t->ptr3;
            p = p->next;
        }

        if (t != NULL) {
            printf("Extra parameter supplied for function '%s'\n", name);
            exit(1);
        } else if (p != NULL) {
            printf("Missing parameter '%s' for function '%s'\n", p->name, name);
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
        if (t->type == T_STR)
            fprintf(fp, "MOV R%d, %s\n", i, t->value.strval);
        else
            fprintf(fp, "MOV R%d, %d\n", i, t->value.intval);
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
    } else if (t->nodetype == N_BRKP) 
        fprintf(fp, "BRKP\n");
}
