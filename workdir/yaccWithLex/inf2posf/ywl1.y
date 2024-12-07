%{
/*** Auxiliary declarations section ***/

#include<stdio.h>
#include<stdlib.h>
%}

 /*** YACC Declarations section ***/
%token NUMBER VAR

%union {
	char* string;
	int no;
}

%type <string> VAR
%type <no> NUMBER

%left '{' '}'
%left '='
%left '>' '<'
%left '+' '-'
%left '*' '/'
%%

/*** Rules Section ***/
start : expr '\n'  {printf("\n"); exit(1);}
      ;

expr: expr '+' expr     {printf("+ ");}
    | expr '-' expr	{printf("- ");}
    | expr '/' expr 	{printf("/ ");}
    | expr '*' expr     {printf("* ");}
    | '(' expr ')'
    | NUMBER		{printf("%d ", $1);}
    | VAR		{printf("%s ", $1);}
    ;
%%

/*** Auxiliary functions section ***/

yyerror(char const *s)
{
    printf("Error %s\n", s);
}

main()
{
 yyparse();
 return 1;
}

