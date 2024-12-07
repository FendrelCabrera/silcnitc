%{
	#include <stdio.h>
	#include <stdlib.h>

	int lc = 0, wc = 0, cc = 0, ic = 0, fc = 0;
%}

integer -?[0-9]+
float -?[0-9]+\.[0-9]+
wordA3 "A"[a-zA-Z]{2}
word [a-zA-Z]+
whitespace [ \t]+
newline [\n\r]+
comment ("/*"(.|\n)*"*/"|"//".*)

%%
{integer} {ic++;}
{float}	{fc++;}
{wordA3} {wc++; printf("A3: %s\n", yytext);}
{word} {wc++; cc += yyleng;}
{whitespace} {}
{newline} {lc++;} 
{comment} {printf("Comment: %s\n", yytext);}
%%

int yywrap() {
	printf("Line Count: %d\n", lc);
	printf("Word count: %d\n", wc);
	printf("Character count: %d\n", cc);
	printf("Integer count: %d\n", ic);
	printf("Float count: %d\n", fc);
	return 1;
}

int main() {
	yyin = fopen("input.txt", "r");
	yylex();
	return 0;
}


