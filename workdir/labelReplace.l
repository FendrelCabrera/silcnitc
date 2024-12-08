%{
    #include <stdlib.h>
    #include <stdio.h>
    int pass = 1, line = 0;
    FILE* fout;
    int labels[100];
%}

letter [a-zA-Z]
digit [0-9]
label {letter}({letter}|{digit})*

%%

{label}:\n {
    if (pass == 1) {
        int index;
        sscanf(yytext, "%*[^0-9]%d", &index);
        labels[index] = 2040 + 2*line;
        printf("L%d @ %d\n", index, labels[index]);
    }
}
(CALL|JMP).*{label}\n {
    if (pass == 1)
        line++;
    else {
        int lno;
        char s[10];
        sscanf(yytext, "%[^0-9 ] %*[^0-9]%d", s, &lno);
        fprintf(fout, "%s %d\n", s, labels[lno]);
    }
}
(JZ|JNZ).*{label}\n {
    if (pass == 1)
        line++;
    else {
        int lno;
        char s[10];
        sscanf(yytext, "%[^,], %*[^0-9]%d", s, &lno);
        fprintf(fout, "%s, %d\n", s, labels[lno]);
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

int main() {
    yyin = fopen("program.xsm", "r");
    yylex();
    return 0;
}
