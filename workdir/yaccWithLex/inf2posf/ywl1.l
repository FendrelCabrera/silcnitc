%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "y.tab.h"
%}

integer -?[0-9]+
float -?[0-9]+\.[0-9]+
whitespace [ \t]+
newline [\n\r]
var [_A-Za-z][_A-Za-z0-9]*

%%
{integer} {yylval.no=atoi(yytext); return NUMBER;}
{var} {yylval.string=strdup(yytext); return VAR;}
{whitespace} {}
{newline} {return *yytext;}
[\+\-\*/\(\)] {return *yytext;}
%%

int yywrap() {
	return 1;
}
