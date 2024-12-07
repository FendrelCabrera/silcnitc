%{
/*** Auxiliary declarations section ***/

#include<stdio.h>
#include<stdlib.h>
#include <string.h>

char *mergeStrings(const char* str1, const char*str2, char token);
%}

 /*** YACC Declarations section ***/
%token NUMBER VAR

%union {
	char* string;
	int no;
}

%type <string> VAR
%type <string> NUMBER

%left '{' '}'
%left '='
%left '>' '<'
%left '+' '-'
%left '*' '/'
%%

/*** Rules Section ***/
start : expr '\n'  {printf("%s\n", $<string>1); exit(1);}
      ;

expr: expr '+' expr     {$<string>$=mergeStrings($<string>1, $<string>3, '+');}
    | expr '-' expr	{$<string>$=mergeStrings($<string>1, $<string>3, '-');}
    | expr '/' expr 	{$<string>$=mergeStrings($<string>1, $<string>3, '/');}
    | expr '*' expr     {$<string>$=mergeStrings($<string>1, $<string>3, '*');}
    | '(' expr ')'	{$<string>$=$<string>2;}
    | NUMBER		{$<string>$=$<string>1;}
    | VAR		{$<string>$=$<string>1;}
    ;
%%

/*** Auxiliary functions section ***/

yyerror(char const *s)
{
    printf("Error %s\n", s);
}

char *mergeStrings(const char *str1, const char *str2, char op) {
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	size_t total = len1 + len2 + 4;

	char *result = (char *)malloc(total);
	if (result == NULL) {
		printf("malloc error\n");
		exit(1);
	}
	
	result[0] = op;
	result[1] = ' ';
	strcpy(result + 2, str1);
	result[len1 + 2] = ' ';
	strcpy(result + (3 + len1), str2);
	return result;
}

int main()
{
 yyparse();
 return 1;
}

