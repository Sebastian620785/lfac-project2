
%{
#include "comp.tab.h"
#include <string>
#include <cstdlib>
#include <cstring>

using namespace std;
%}
%option noyywrap
%%


"int"      { return INT; }
"float"    { return FLOAT; }
"string"   { return STRING; }
"bool"     { return BOOL; }

"class"    { return CLASS; }
"main"     { return MAIN; }
"if"       { return IF; }
"while"    { return WHILE; }
"return"   { return RETURN; }
"Print"    { return PRINT; }

"true"     { return TRUE; }
"false"    { return FALSE; }

"==" { return E; }
"!=" { return NE; }
"<=" { return LE; }
">=" { return RE; }

"&&" { return AND; }
"||" { return OR; }
"!"  { return NOT; }

[0-9]+"."[0-9]+ {
    yylval.Float=atof(yytext);
    return FLOAT_LITERAL;
}
[0-9]+ {
    yylval.Int = atoi(yytext);
    return INT_LITERAL;
}
\"[^\n"]*\" {
    std::string* s = new std::string(yytext + 1, strlen(yytext) - 2);
    yylval.String = s;
    return STRING_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    std::string* s = new std::string(yytext);
    yylval.String = s;
    return ID;
}


[ \t\r\n]+  ;

. { return yytext[0]; }

%%

