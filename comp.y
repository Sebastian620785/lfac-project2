/* ====== HEADERE PENTRU .h ȘI .c ====== */
%code requires {
  #include <string>
  using std::string;
}

%{
#include <iostream>

extern int yylex();
void yyerror(const char* s);
%}

/* ====== VALORI SEMANTICE (yylval) ====== */
%union {
  int    Int;
  float  Float;
  string *String;
}

/* ====== TOKENURI FĂRĂ VALOARE ====== */
%token INT FLOAT STRING BOOL
%token CLASS MAIN IF WHILE RETURN PRINT
%token TRUE FALSE
%token AND OR NOT 

/* ====== TOKENURI CU VALOARE DIN %union ====== */

%token <String> ID
%token <String> STRING_LITERAL
%token <Int>    INT_LITERAL
%token <Float>  FLOAT_LITERAL

%token E NE LE RE

/* ====== PRECEDENȚĂ (pentru mai târziu) ====== */
%left OR
%left AND
%right NOT
%left E NE LE RE '<' '>'
%left '+' '-'
%left '*' '/'

/* ====== SIMBOL DE START ====== */
%start program

%%

/* ====== REGULI DE GRAMATICĂ ====== */

/* programul = (0+ declarații globale) + main_block obligatoriu */
program
  : global_list main_block
  ;

/* listă de declarații globale (variabile, funcții, clase) */
global_list
  : /* empty */
  | global_list global_decl
  ;

/* deocamdată, doar variabile globale */
global_decl
  : var_decl ';'
  ;

/* blocul main */
main_block
  : MAIN '(' ')' '{' stmt_list '}'
  ;

/* listă de statement-uri din main (sau alt bloc) */
stmt_list
  : /* empty */
  | stmt_list statement
  ;

/* un statement – acum doar Print(expr);, ulterior mai adăugăm */
statement
  : print_stmt ';'
  | ID '=' expr ';'
  | IF '(' expr ')' '{' stmt_list '}'
  | WHILE '(' expr ')' '{' stmt_list '}' 
  | fnc_call ';'
  |method_call ';'
  ;

/* Print(expr) */
print_stmt
  : PRINT '(' expr ')'
  ;

fnc_call
  : ID '(' arg_list_opt ')'
  ;
arg_list_opt
  :/*empty*/
  | arg_list
  ;
arg_list
  : expr
  | arg_list ',' expr
  ;
/* declarație de variabilă (globală sau locală) */
var_decl
  : type ID                   /* ex: int x; */
  | type ID '=' expr          /* ex: int x = 3; */
  ;

/* tipuri de bază */
type
  : INT
  | FLOAT
  | STRING
  | BOOL
  ;

/* expresii – deocamdată literali simpli (integer, float, string, bool) */
expr
  : INT_LITERAL
  | FLOAT_LITERAL
  | STRING_LITERAL
  | TRUE
  | FALSE
  | ID
  | expr '+' expr 
  | expr '-' expr
  | expr '*' expr
  | expr '/' expr
  | '(' expr ')'
  | expr E  expr
  | expr NE expr
  | expr LE expr
  | expr RE expr
  | expr '<' expr
  | expr '>' expr
  | expr AND expr
  | expr OR expr
  | NOT expr
  |fnc_call
  |field_access
  |method_call
  ;

field_access
  :ID '.' ID
  ;
method_call
  : ID '.' fnc_call
  ;
%%

/* ====== IMPLEMENTARE yyerror + main ====== */

void yyerror(const char* s) {
  std::cout << "error:" << s;
}

int main() {
  yyparse();
  return 0;
}
