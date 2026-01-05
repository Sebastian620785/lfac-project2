%{
#include "comp.tab.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;
%}

%option noyywrap
%option yylineno

%x COMMENT

%%

"int"           { return INT; }
"float"         { return FLOAT; }
"string"        { return STRING; }
"bool"          { return BOOL; }
"void"          { return VOID; }
"class"         { return CLASS; }
"main"          { return MAIN; }
"return"        { return RETURN; }
"if"            { return IF; }
"while"         { return WHILE; }
"print"|"Print" { return PRINT; }
"true"          { return TRUE; }
"false"         { return FALSE; }

"=="            { return EQ; }
"!="            { return NEQ; }
"<="            { return LE; }
">="            { return GE; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }

[0-9]+\.[0-9]+  { yylval.Float = atof(yytext); return FLOAT_LITERAL; }
[0-9]+          { yylval.Int = atoi(yytext); return INT_LITERAL; }
\"[^\"]*\"      { yylval.String = new string(yytext + 1, strlen(yytext) - 2); return STRING_LITERAL; }

[a-zA-Z][a-zA-Z0-9_]* { yylval.String = new string(yytext); return ID; }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>(.|\n) { /* Ignora */ }
"//"[^\n]* { /* Ignora linie */ }

[ \t\n\r]+      ; 
.               { return yytext[0]; }

%%