int line = 1;
Typetable *Tptr = NULL;
Classtable *Cptr = NULL;

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

Typetable *TCheck(char *name) {
    if (strcmp(name, "void") == 0 || strcmp(name, "bool") == 0) {
        printf("%d: Field/var of type %s not allowed\n", line, name);
        exit(1);
    }
    Typetable *t = TLookup(name);
    if (t == NULL) {
        printf("%d: Type %s not defined\n", line, name);
        exit(1);
    }
    return t;
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

void Type_Finstall(Typetable *tptr, char *type, char *name) {
    Typetable *tp = TLookup(type);
    if (tp == NULL) {
        printf("%d: Type %s not defined\n", line, type);
        exit(1);
    } else if (strcmp(type, "void") == 0 || strcmp(type, "bool") == 0) {
        printf("%d: Type %s cannot be used for a field\n", line, type);
        exit(1);
    }

    int fi = 0; Fieldlist *f = tptr->fields;
    for (; f != NULL; f = f->next) {
        fi++;
        if (strcmp (f->name, name) == 0) {
            printf("%d: Field %s repeated for type %s\n", line, name, tptr->name);
            exit(1);
        } else if (f->next == NULL) break;
    }

    if (fi == 8) {
        printf("%d: User defined types cannot have more than 8 fields\n", line);
        exit(1);
    }

    Fieldlist *x = (Fieldlist *)malloc(sizeof(Fieldlist));
    *x = (Fieldlist) { name, fi, tp, NULL, NULL };

    if (f == NULL) tptr->fields = x;
    else f->next = x;
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

Classtable *CT = NULL;
int gstSize = 0;
Classtable *CInstall (char *name, char *parent_class_name) {
    if (TLookup(name) != NULL) {
        printf("Class %s conflicts with type %s\n", name, name);
        exit(1);
    }

    Classtable *t = CT;
    int classIndex = 0;
    while (t != NULL) {
        classIndex++;
        if (strcmp(t->name, name) == 0) {
            printf("%d: Class %s already exists\n", line, name);
            exit(1);
        }

        if (t->next == NULL) break;
        t = t->next;
    }

    Classtable *x = (Classtable *)malloc(sizeof(Classtable));
    Classtable *parentPtr = NULL;
    Memberfunclist *m = NULL;
    Fieldlist *f = NULL;
    int fieldCount = 0, methodCount = 0;

    if (parent_class_name != NULL) {
        parentPtr = CLookup(parent_class_name);
        if (parentPtr == NULL) {
            printf("%d: Class %s not defined\n", line, parent_class_name);
            exit(1);
        }
        
        f = FCopy(parentPtr);
        m = MCopy(parentPtr);
        fieldCount = parentPtr->fieldCount;
        methodCount = parentPtr->methodCount;
    }

    *x = (Classtable) {
        name, f, m, parentPtr, classIndex, fieldCount, methodCount, NULL
    };

    if (t == NULL) CT = x;
    else t->next = x;
    gstSize += 8;
    return x;
}

Classtable *CLookup(char *name) {
    Classtable *t = CT;
    for (; t != NULL; t = t->next) {
        if (strcmp(name, t->name) == 0)
            break;
    }

    return t;
}

void Class_Finstall(Classtable *cptr, char *typename, char *name) {
    if (cptr->fieldCount == 8) {
        printf("Class fields can occupy atmost 8 words: %s\n", cptr->name);
        exit(1);
    }

    Fieldlist *f = cptr->memberfield;
    for (; f != NULL; f = f->next) {
        if (strcmp (f->name, name) == 0) {
            printf("%d: Field %s already declared\n", line, name);
            exit(1);
        }

        if (f->next == NULL) break;
    }

    Typetable *t = TLookup(typename);
    Classtable *c = CLookup(typename);
    if (t == NULL && c == NULL) {
        printf("%d: Type/class %s not defined\n", line, typename);
        exit(1);
    } else if (c != NULL && cptr->fieldCount == 7) {
        printf("Class fields can occupy atmost 8 words: %s\n", cptr->name);
        exit(1);
    }

    Fieldlist *x = (Fieldlist *)malloc(sizeof(Fieldlist));
    *x = (Fieldlist) {name, cptr->fieldCount, t, c, NULL};
    
    if (f == NULL) cptr->memberfield = x;
    else f->next = x;

    if (c == NULL) cptr->fieldCount++;
    else cptr->fieldCount += 2;
}

int fcount = 0;
void Class_Minstall(Classtable *cptr, char *name, Typetable *type, Paramstruct *Paramlist) {
    Fieldlist *f = Class_Flookup(cptr, name);
    if (f != NULL) {
        printf("%d: Method %s conflicts with field %s\n", line, name, name);
        exit(1);
    }

    Memberfunclist *m = cptr->vfuncptr;
    for (; m != NULL; m = m->next) {
        if (strcmp(m->name, name) == 0) {
            if (m->isDefined == 0) {
                printf("%d: Method %s already declared\n", line, name);
                exit(1);
            } else {
                // inherited
                // param check
                Paramstruct *p1 = m->paramlist, *p2 = Paramlist;
                while (p1 != NULL && p2 != NULL) {
                    if (p1->type != p2->type || strcmp(p1->name, p2->name) != 0)
                        break;
                    p1 = p1->next; p2 = p2->next;
                }

                if (p1 || p2 || type != m->type) {
                    printf("%d: Overridden method should have same signature as in parent\n", line);
                    exit(1);
                }
                
                m->flabel = ++fcount;
                m->isDefined = 0;
                return;
            }
        }

        if (m->next == NULL) break;
    }

    if (cptr->methodCount == 8) {
        printf("Class can have atmost 8 methods: %s\n", cptr->name);
        exit(1);
    }

    Memberfunclist *x = (Memberfunclist *) malloc(sizeof(Memberfunclist));
    *x = (Memberfunclist) {
        name, type, Paramlist, cptr->methodCount, 
        ++fcount, NULL, 0
    };

    if (m == NULL) cptr->vfuncptr = x;
    else m->next = x;
    cptr->methodCount++;
}

Memberfunclist *Class_Mlookup(Classtable *Ctype, char *Name) {
    Memberfunclist *m = Ctype->vfuncptr;
    for (; m != NULL; m = m->next) {
        if (strcmp(m->name, Name) == 0)
            break;
    }
    return m;
}

Fieldlist *Class_Flookup(Classtable *Ctype, char *Name) {
    Fieldlist *f = Ctype->memberfield;
    for (; f != NULL; f = f->next) {
        if (strcmp(f->name, Name) == 0)
            break;
    }
    return f;
}

Fieldlist *FCopy(Classtable *Ctype) {
    Fieldlist *ret = NULL, *tail = NULL;

    for(Fieldlist *f = Ctype->memberfield; f != NULL; f = f->next) {
        Fieldlist *x = (Fieldlist *)malloc(sizeof(Fieldlist));
        memcpy(x, f, sizeof(Fieldlist));

        if (ret == NULL) ret = x;
        else tail->next = x;
        tail = x;
    }

    return ret;
}

Memberfunclist *MCopy(Classtable *Ctype) {
    Memberfunclist *ret = NULL, *tail = NULL;

    for(Memberfunclist *m = Ctype->vfuncptr; m != NULL; m = m->next) {
        Memberfunclist *x = (Memberfunclist *)malloc(sizeof(Memberfunclist));
        memcpy(x, m, sizeof(Memberfunclist));

        if (ret == NULL) ret = x;
        else tail->next = x;
        tail = x;
    }

    return ret;
}

int isChild(Classtable *c1, Classtable *c2) {
    while(c2 != NULL) {
        if (c1 == c2)
            return 1;
        else
            c2 = c2->parentPtr;
    }
    return 0;
}


Gsymbol *GST = NULL;

Gsymbol *GInstall (char *name, Typetable *type, Classtable *ctype, int size, Paramstruct *paramlist) {
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
        name, type, ctype, size, -1, paramlist, -1, NULL
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
            printf("%d: Redeclared variable: %s\n", line, name);
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
        // x->binding = 0;
        LST = x;
    } else {
        // x->binding = len;
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
    Gsymbol *Gentry = NULL;
    Lsymbol *Lentry = NULL;
    Classtable *ctype = NULL;

    // ID lookup
    if (nodetype == N_ID || nodetype == N_PTR) {
        Lentry = LLookup(name); Gentry = GLookup(name);

        if (ptr1 == NULL) {
            if (Lentry != NULL) {
                if (strcmp(name, "self") == 0) ctype = Cptr;
                else type = Lentry->type;
            } else if (Gentry == NULL) {
                printf("%d: Undeclared variable: '%s'\n", line, name);
                exit(1);
            } /* else if (Gentry->size > 1) {
                printf("%d: Invalid var access for array: %s\n", line, name);
                exit(1);
            } */ else if (Gentry->type != NULL)
                type = Gentry->type;
            else 
                ctype = Gentry->ctype;
        } else if (Gentry == NULL && Lentry != NULL) {
            printf("%d: Invalid array access for local var: %s\n", line, name);
            exit(1);
        } else if (Gentry != NULL) {
            if (Gentry->size == 1) {
                printf("%d: Invalid array access for global var: %s\n", line, name);
                exit(1);
            } else if (Gentry->type == NULL) {
                printf("%d: Array access of class %s not allowed\n", line, Gentry->ctype->name);
                exit(1);
            } else
                type = Gentry->type;
        } else {
            printf("%d: Undeclared array: %s\n", line, name);
            exit(1);
        }

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
        Fieldlist *f;
        if ( arglist->type != NULL ) {
            f = FLookup(arglist->type, name);
            if ( f == NULL ) {
                printf("%d: %s is not a field of type %s\n", line, name, arglist->type->name);
                exit(1);
            }
        } else {
            f = Class_Flookup(arglist->ctype, name);
            if ( f == NULL ) {
                printf("%d: %s is not a field of class %s\n", line, name, arglist->ctype->name);
                exit(1);
            }
        }

        arglist = NULL;
        if ( f->type != NULL ) type = f->type;
        else ctype = f->ctype;
    } else if (nodetype == N_METHOD) {
        Memberfunclist *m = Class_Mlookup(ptr1->ctype, name);
        if (m == NULL) {
            printf("%d: %s is not a method of class %s\n", line, name, ptr1->ctype->name);
            exit(1);
        }
        type = m->type;
    }

    // type checking
    if (nodetype == N_WRITE && (ptr1->type != TLookup("int") && ptr1->type != TLookup("str"))) {
        printf("%d: Type mismatch: Writing %s\n", line, ptr1->type->name);
        exit(1);
    } else if (nodetype == N_ASGN && 
               (
                   ptr1->type != ptr2->type || 
                   ptr1->type == NULL && !isChild(ptr1->ctype, ptr2->ctype)
               ) && 
               !(
                   ptr2->type == TLookup("void") && 
                   (ptr1->ctype != NULL || ptr1->type->fields != NULL)
               )
              ) {
        if (ptr1->type != ptr2->type) printf("Types not equal\n");
        else if (ptr1->ctype != ptr2->ctype) printf("Ctypes not equal\n");

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
            (
                ptr1->type != ptr2->type || 
                ptr1->type == NULL && !isChild(ptr1->ctype, ptr2->ctype)
            ) && 
            ((ptr1->type != NULL && ptr1->type->fields == NULL) || ptr2->type != TLookup("void")) &&
            (ptr1->type != TLookup("void") || (ptr2->type != NULL && ptr2->type->fields == NULL))
            ){
        printf("%d: Type mismatch in comparision\n", line);
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
    } else if (nodetype == N_METHOD) {
        Memberfunclist *m = Class_Mlookup(ptr1->ctype, name);
        ASTNode *t = arglist;
        Paramstruct *p = m->paramlist;

        while (t != NULL && p != NULL) {
            if (t->type != p->type) {
                printf("%d: Type mismatch: Expected %s but received %s for argument '%s' of method '%s'\n", line, p->type->name, t->type->name, p->name, name);
                exit(1);
            }

            t = t->ptr3;
            p = p->next;
        }

        if (t != NULL) {
            printf("%d: Extra parameter supplied for method '%s'\n", line, name);
            exit(1);
        } else if (p != NULL) {
            printf("%d: Missing parameter '%s' for method '%s'\n", line, p->name, name);
            exit(1);
        }
    }

    ASTNode *x = (ASTNode *)malloc(sizeof(ASTNode));
    *x = (ASTNode) {
        type, ctype, nodetype, name, value, arglist, ptr1, ptr2, ptr3, Gentry, Lentry
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

int lastUsedLabel = 0;
int getLabel() {
    return ++lastUsedLabel;
}

int loopStack[100][2];
int stackSize = 0;

int funcRegStack[20];
int frsSize = 0;


int codeGen (ASTNode *t, FILE* fp) {
    if (t == NULL)
        return 0;
    else if (t->nodetype == N_CONST) {
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
        
        if (t->ptr1->ctype != NULL && isChild(t->ptr1->ctype, t->ptr2->ctype)) {
            t->ptr2->value.intval = 0;
            codeGen(t->ptr2, fp);
            fprintf(fp, "INR R0\nINR R1\nMOV [R0], [R1]\n");
            freeReg();
        }

        freeReg();
    } else if (t->nodetype == N_SLIST) {
        codeGen(t->ptr1, fp);
        codeGen(t->ptr2, fp);
    } else if (t->nodetype == N_BODY) {
        if (Cptr == NULL)
            fprintf(fp, "F%d:\n", t->Gentry->flabel);
        else
            fprintf(fp, "F%d:\n", Class_Mlookup(Cptr, t->name)->flabel);


        fprintf(fp, "PUSH BP\nMOV BP, SP\n");

        for (Lsymbol *l = t->Lentry; l != NULL; l = l->next) {
            if (l->binding > 0)
                fprintf(fp, "PUSH R0\n");
        }
        // LST = t->Lentry;

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
        Typetable *tp = NULL;
        if (t->Lentry != NULL) {
            int j = getReg();
            fprintf(fp, "MOV R%d, BP\n", i);
            fprintf(fp, "MOV R%d, %d\n", j, t->Lentry->binding);
            fprintf(fp, "ADD R%d, R%d\n", i, j);
            freeReg();

            if (t->Lentry->type != NULL)
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

        ASTNode *y = t->ptr2;
        do {
            fprintf(fp, "MOV R%d, [R%d]\n", i, i);

            Fieldlist *f;
            if (tp != NULL) f = FLookup(tp, y->name);
            else f = Class_Flookup(Cptr, y->name);
            fprintf(fp, "ADD R%d, %d\n", i, f->fieldIndex);

            tp = y->type;
            y = y->ptr2;
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
        
        if (t->ctype != NULL) 
            fprintf(fp, "INR R1\nMOV [R1], %d\n", 4096 + t->ctype->classIndex*8);

        freeReg();
        freeReg();
    } else if (t->nodetype == N_FREE) {
        int i = codeGen(t->ptr1, fp);
        fprintf(fp, "MOV R1, \"Free\"\nPUSH R1\nPUSH R0\nPUSH R1\nPUSH R1\nPUSH R1\nCALL 0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\nPOP R0\n");
        freeReg();
    } else if (t->nodetype == N_METHOD) {
        // Pushing registers in use
        funcRegStack[frsSize++] = lastUsedReg;
        for (int i = 0; i <= lastUsedReg; i++)
            fprintf(fp, "PUSH R%d\n", i);
        lastUsedReg = -1;

        // Address of self
        /*
        t->ptr1->value.intval = 1;
        codeGen(t->ptr1, fp);
        fprintf(fp, "PUSH R0\n");
        freeReg();
        */
        codeGen(t->ptr1, fp);
        if (t->ptr1->nodetype == N_FIELD || strcmp(t->ptr1->name, "self") != 0)
        fprintf(fp, "INR R0\nMOV R1, [R0]\nPUSH R1\nDCR R0\nMOV R1, [R0]\nPUSH R1\n");
        else
        fprintf(fp, "DCR R0\nMOV R1, [R0]\nPUSH R1\nINR R0\nMOV R1, [R0]\nPUSH R1\n");
        freeReg();

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
        Memberfunclist *m = Class_Mlookup(t->ptr1->ctype, t->name);
        
        codeGen(t->ptr1, fp);
        if (t->ptr1->nodetype == N_FIELD || strcmp(t->ptr1->name, "self") != 0)
        fprintf(fp, "INR R0\nMOV R0, [R0]\nADD R0, %d\nMOV R0, [R0]\n", m->funcPosition);
        else
        fprintf(fp, "DCR R0\nMOV R0, [R0]\nADD R0, %d\nMOV R0, [R0]\n", m->funcPosition);
        freeReg();

        // fprintf(fp, "CALL F%d\n", m->flabel);
        fprintf(fp, "CALL R0\n");
        
        lastUsedReg = funcRegStack[--frsSize];
        int j = getReg();
        fprintf(fp, "POP R%d\n", j); // store return value

        // Pop and discard arguments
        int popReg = (j == 0)?(1):(0);
        for (
            ASTNode *arg = t->arglist;
            arg != NULL; arg = arg->ptr3
        ) fprintf(fp, "POP R%d\n", popReg);
        fprintf(fp, "POP R%d\nPOP R%d\n", popReg, popReg); // popping self

        // Pop and restore registers in use
        for (int i = j - 1; i >= 0; i--)
            fprintf(fp, "POP R%d\n", i);
        return j;
    }
}

