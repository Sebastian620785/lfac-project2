
/*ce apare in .h*/
%code requires {
  #include <string>
  using namespace std;
}

%{
#include <iostream>



extern int yylex();
 void yyerror(const char* s);
%}
/*ajunge  in .c*/


%union {
  int Int;
  float Float;

  string * String;
}
/*toate tipurile posibile pe care le pot avea simbolurile*/

/*definesc tokeni*/
%token INT FLOAT STRING BOOL
%token CLASS MAIN IF WHILE RETURN PRINT
/*definesc identificatorii si literalii*/
%token <String> STRING_LITERAL
%token <String> ID
%token <Int> INT_LITERAL
%token <Float> FLOAT_LITERAL
%token TRUE FALSE

%start program

%%
program:
global_list main_block
;
global_list:
   | global_list global_decl
   ;

   global_decl:
   var_decl ';'
   ;
   main_block:
   MAIN '{' stm_list '}'
   ;
   stm_list:
   |stm_list statement
   ;
   statement:
   print_stmt ';'
   ;
   print_stmt:
   PRINT '(' expr ')'
   ;
   var_decl:
   type ID
   |type ID '=' expr
   ;
   type :
   INT
   |FLOAT
   |STRING
    |BOOL
    ;
    expr:
    INT_LITERAL
    |FLOAT_LITERAL
    |STRING_LITERAL
    |TRUE
    |FALSE
    ;
%%

void yyerror(const char* s) {
  cout << "error:" << s;
}

int main(){
 yyparse();
}

