char *nt[] = { 
    "CONST", "ID", "PTR",
    "PLUS", "MINUS", "MUL", "DIV", "MOD",
    "GT", "LT", "GE", "LE",  "EQ", "NE",
    "OR", "AND",
    "IF", "BREAK", "CONTINUE",
    "WHILE", "REPEAT", "DOWHILE",
    "READ", "WRITE", "ASGN",
    "SLIST", "BODY", "RET", "FUNC", 
    "FIELD", "METHOD", 
    "INIT", "ALLOC", "FREE",
    "BRKP"
};

void printGST() {
    Gsymbol *temp = GST;
    Paramstruct *p;
    char sbuf[25];

    if(temp == NULL) {
        printf("No Global Declarations\n");
        return;
    }

    printf("Global Declarations:\nName      Type Size Binding Paramlist      FLabel\n");
    while(temp != NULL) {
        printf("%9s %4s %4d %7d ", temp->name, 
                (temp->type != NULL)?(temp->type->name):(temp->ctype->name), 
                temp->size, temp->binding);

        sprintf(sbuf, "(");
        p = temp->paramlist;
        while(p != NULL) {
            sprintf(sbuf + strlen(sbuf), "%s %s, ", p->type->name, p->name);
            p = p->next;
        }
        sprintf(sbuf + strlen(sbuf), ")");

        if (strlen(sbuf) <= 9)
            printf("%9s %6d\n", sbuf, temp->flabel);
        else
            printf("%s %d\n", sbuf, temp->flabel);
        temp = temp->next;
    }

    printf("\n");
}

void printTT() {
    if (TT == NULL) {
        printf("No type declarations\n");
        return;
    }

    printf("Type Declarations:\n");
    for (Typetable *type = TT; type != NULL; type = type->next) {
        printf("%s %d ", type->name, type->size);

        if ( type->fields != NULL ) {
            printf("{ ");
            for (Fieldlist *f = type->fields; f != NULL; f = f->next)
                printf("%s %s, ", f->type->name, f->name);
            printf("}");
        }

        printf("\n");
    }

    printf("\n");
}

void printCT() {
    if (CT == NULL) {
        printf("No class declarations\n");
        return;
    }

    printf("Class declarations:\n");
    for (Classtable *ctype = CT; ctype != NULL; ctype = ctype->next) {
        printf("%s\n(", ctype->name);
        for (Fieldlist *f = ctype->memberfield; f != NULL; f = f->next) {
            if (f->type) printf("%s %s", f->type->name, f->name);
            else printf("%s %s", f->ctype->name, f->name);
            if (f->next != NULL) printf(", ");
        }
        printf(")\n");
        for (Memberfunclist *m = ctype->vfuncptr; m != NULL; m = m->next) {
            printf("%s %s(", m->type->name, m->name);
            for (Paramstruct *p = m->paramlist; p != NULL; p = p->next) {
                printf("%s %s", p->type->name, p->name);
                if (p->next != NULL) printf(", ");
            }
            printf(")\n");
        }
        printf("\n");
    }
}

void printT (ASTNode *root, int level) {
    if (root == NULL)
        return;
    
    for(int i = 0; i < level; i++)
        printf("| ");
    
    if (root->nodetype == N_FUNC) {
        printf("%s\n", nt[root->nodetype]);
        ASTNode *t = root->arglist;
        while (t != NULL) {
            printT(t, level + 1);
            t = t->ptr3;
        }
    } else if (root->nodetype == N_BODY) {
        printf("%s (", nt[root->nodetype]);
        Lsymbol *l = root->Lentry;
        while (l != NULL) {
            if (l->type != NULL)
                printf("%s ", l->type->name);
            
            printf("%s %d, ", l->name, l->binding);
            l = l->next;
        }
        printf(")\n");
    } else if (root->nodetype == N_FIELD) {
        printf("%s (%s", nt[root->nodetype], root->name);
        ASTNode *t = root->ptr3;

        while (t != NULL) {
            printf(".%s", t->name);
            t = t->ptr3;
        }
        printf(")\n");
        return;
    } else 
        printf("%s\n", nt[root->nodetype]);

    printT(root->ptr1, level + 1);
    printT(root->ptr2, level + 1);
    printT(root->ptr3, level + 1);
}

ASTNode *revArgList(ASTNode *root) {
    if (root == NULL)
        return NULL;
    else if (root->ptr3 == NULL)
        return root;

    ASTNode *head = revArgList(root->ptr3);
    root->ptr3->ptr3 = root;
    root->ptr3 = NULL;

    return head;
}
