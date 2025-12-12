
%{
#include "comp.tab.h"
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;
%}
%option noyywrap
%%


"int"      { return INT; }
"float"    { return FLOAT; }
"string"   { return STRING; }
"bool"     { return BOOL; }
"void"     { return VOID; }

"class"    { return CLASS; }
"main"     { return MAIN; }
"if"       { return IF; }
"while"    { return WHILE; }
"return"   { return RETURN; }
"Print"    { return PRINT; }

"true"     { return TRUE; }
"false"    { return FALSE; }

[0-9]+"."[0-9]+ { yylval.Float = atof(yytext); return FLOAT_LITERAL; }
\"[^\n"]*\"     { yylval.String = new std::string(yytext + 1, strlen(yytext) - 2); return STRING_LITERAL; }

[0-9]+ {
    yylval.Int = atoi(yytext);
    return INT_LITERAL;
}
[a-zA-Z_][a-zA-Z0-9_]* {
    std::string* s = new std::string(yytext);
    yylval.String = s;
    return ID;
}


[ \t\r\n]+  ;

. { return yytext[0]; }

%%

