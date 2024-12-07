%{
/*** Auxiliary declarations section ***/

#include<stdio.h>
#include<stdlib.h>

/* Custom function to print an operator*/
void print_operator(char op);

/* Variable to keep track of the position of the number in the input */
int pos=0, lvl=0;

%}

 /*** YACC Declarations section ***/
%token DIGIT LETTER IF 
%left '{' '}'
%left '='
%left '>' '<'
%left '+' '-'
%left '*' '/'
%%

/*** Rules Section ***/
start : stmt '\n'  {exit(1);}
      ;

stmt : var '=' expr ';' {printf("Decl with valid variable\n");}
     | ifcontrol '{' stmt '}' {printf("if end\n"); lvl--;}
     | stmt stmt
     ;

boolean : expr '>' expr {printf("Reduced greater boolean\n");}
	| expr '<' expr {printf("Reduced lesser boolean\n");}
	;

ifcontrol : IF '(' boolean ')' {printf("if start at lvl %d\n", lvl++);} 

expr: expr '+' expr     {print_operator('+');}
    | expr '-' expr	{print_operator('-');}
    | expr '/' expr 	{print_operator('/');}
    | expr '*' expr     {print_operator('*');}
    | '(' expr ')'
    | number		{printf("Reduced num to expr\n");}
    | var		{printf("Reduced var to expr\n");}
    ;

number: DIGIT
      | number DIGIT
      ;

var: LETTER 
   | var DIGIT 
   | var LETTER 
   ;
%%

/*** Auxiliary functions section ***/

void print_operator(char c){
    switch(c){
        case '+'  : printf("PLUS ");
                    break;
        case '*'  : printf("MUL ");
                    break;
	case '-'  : printf("MIN ");
		    break;
	case '/'  : printf("DIV ");
		    break;
    }
    return;
}

yyerror(char const *s)
{
    printf("yyerror random %s\n\n",s);
}

yylex(){
    char c;
    c = getchar();
    if(isdigit(c)){
        pos++;
	yylval = c - '0';
        return DIGIT;
    }
    else if (c == '_') {
	return LETTER;
    }
    else if (isalpha(c)) {
	if (c == 'i') {
		c = getchar();
		if (c == 'f') {
			c = getchar();
			ungetc(c, stdin);
			if (c == '(') 
				return IF;
			else {
				c = 'f';
				ungetc(c, stdin);
				return LETTER;
			}
		}
		ungetc(c, stdin);
		return LETTER;
	}

	return LETTER;
    }							
    else if(c == ' '){
        yylex();         /*This is to ignore whitespaces in the input*/
    }
    else {
        return c;
    }
}

main()
{
 yyparse();
 return 1;
}

