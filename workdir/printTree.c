char *nt[] = { 
    "CONST", "ID", "PTR",
    "PLUS", "MINUS", "MUL", "DIV", "MOD",
    "GT", "LT", "GE", "LE",  "EQ", "NE",
    "OR", "AND",
    "IF", "BREAK", "CONTINUE",
    "WHILE", "REPEAT", "DOWHILE",
    "READ", "WRITE", "ASGN",
    "SLIST", "BODY", "RET", "FUNC", "MAIN",
    "FIELD", "BRKP"
};

void printGST() {
    Gsymbol *temp = GST;
    Paramstruct *p;
    char sbuf[25];

    if(temp == NULL) {
        printf("No Global Declarations\n");
        return;
    }

    printf("Global Declarations:\nName Type Size Binding Paramlist      FLabel\n");
    while(temp != NULL) {
        printf("%4s %4d %4d %7d ", temp->name, temp->type, temp->size, temp->binding);

        sprintf(sbuf, "(");
        p = temp->paramlist;
        while(p != NULL) {
            sprintf(sbuf + strlen(sbuf), "%d %s, ", p->type, p->name);
            p = p->next;
        }
        sprintf(sbuf + strlen(sbuf), ")");

        if (strlen(sbuf) <= 9)
            printf("%9s %6d\n", sbuf, temp->flabel);
        else
            printf("%s %d\n", sbuf, temp->flabel);
        temp = temp->next;
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
            t = t->arglist;
        }
    } else if (root->nodetype == N_BODY || root->nodetype == N_MAIN) {
        printf("%s (", nt[root->nodetype]);
        Lsymbol *l = root->Lentry;
        while (l != NULL) {
            printf("%d %s %d, ", l->type, l->name, l->binding);
            l = l->next;
        }
        printf(")\n");
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
