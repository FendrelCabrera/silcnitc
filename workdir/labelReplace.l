%{
    #include <stdlib.h>
    #include <stdio.h>
    int pass = 1, line = 0;
    FILE* fout;
    // int labels[100];

    char labels[100][5], lname[5], start[10];
    int lcount = 0, labelAdr[100];

    void set(char lname[5], int adr);
    int get(char lname[5]);
%}

letter [a-zA-Z]
digit [0-9]
label {letter}({letter}|{digit})*

%%

{label}:\n {
    if (pass == 1) {
        /*
        int index;
        sscanf(yytext, "%*[^0-9]%d", &index);
        labels[index] = 2040 + 2*line;
        printf("L%d @ %d\n", index, labels[index]);
        */

        sscanf(yytext, "%[^:]", lname);
        set(lname, 2040 + 2*line);
        printf("%s @ %d\n", lname, 2040 + 2*line);
    }
}
(CALL|JMP).*{label}\n {
    if (pass == 1)
        line++;
    else {
        /*
        int lno;
        char s[10];
        sscanf(yytext, "%[^0-9 ] %*[^0-9]%d", s, &lno);
        fprintf(fout, "%s %d\n", s, labels[lno]);
        */

        sscanf(yytext, "%[^ ] %s", start, lname);
        fprintf(fout, "%s %d\n", start, get(lname));
    }
}
(JZ|JNZ).*{label}\n {
    if (pass == 1)
        line++;
    else {
        /*
        int lno;
        char s[10];
        sscanf(yytext, "%[^,], %*[^0-9]%d", s, &lno);
        fprintf(fout, "%s, %d\n", s, labels[lno]);
        */
        sscanf(yytext, "%[^,], %s", start, lname);
        fprintf(fout, "%s, %d\n", start, get(lname));
    }
}
.*\n {
    if (pass == 1)
        line++;
    else
        fprintf(fout, "%s", yytext);
}

%%

int yywrap(void) {
    if(pass == 1) {
        pass = 2;
        rewind(yyin);
        fout = fopen("test.xsm", "w");
        return 0;
    } 

    fclose(yyin);
    fclose(fout);
    return 1;
}

void set(char lname[5], int adr) {
    strcpy(labels[lcount], lname);
    labelAdr[lcount] = adr;
    lcount++;
}

int get(char lname[5]) {
    for (int i = 0; i < lcount; i++) {
        if (strcmp(lname, labels[i]) == 0)
            return labelAdr[i];
    }

    return -1;
}

int main() {
    yyin = fopen("program.xsm", "r");
    yylex();
    return 0;
}
