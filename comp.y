%code requires {
  #include "ast.h"
}

%{
#include <iostream>
#include <string>
#include <vector>
#include "types.h"
#include "ast.h"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

ASTNode* root = nullptr;
%}

%union {
  int       Int;
  float     Float;
  std::string *String;
  TypeInfo  *TypeVal;
  
  ASTNode* Node;
  BlockNode* Block;
  VarDeclNode* VarDecl;
  
  std::vector<ASTNode*>* NodeList; 
  std::vector<VarDeclNode*>* VarList; 
}

%token INT FLOAT STRING BOOL VOID CLASS MAIN IF WHILE RETURN PRINT TRUE FALSE
%token <String> ID STRING_LITERAL
%token <Int> INT_LITERAL
%token <Float> FLOAT_LITERAL
%token EQ NEQ LE GE AND OR NOT

%left OR
%left AND
%left EQ NEQ
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right NOT
%right '(' '[' '.'

%type <TypeVal> type
%type <NodeList> global_list class_member_list arg_list arg_list_opt
%type <VarList> param_list param_list_nonempty
%type <Node> global_decl class_def func_def class_member statement main_block assignment
%type <Node> expr
%type <VarDecl> var_decl param_decl
%type <Block> stmt_list block_items

%start program

%%

program
  : global_list main_block 
    { 
        ProgramNode* prog = new ProgramNode();
        prog->globals = *$1;
        prog->mainBlock = $2;
        root = prog;
    }
  ;

global_list
  : /* empty */ 
    { $$ = new std::vector<ASTNode*>(); }
  | global_list global_decl
    { 
      $1->push_back($2);
      $$ = $1;
    }
  ;

global_decl
  : var_decl ';' { $$ = $1; }
  | func_def     { $$ = $1; }
  | class_def    { $$ = $1; }
  ;

main_block
  : MAIN '(' ')' '{' stmt_list '}' 
    { $$ = new MainNode($5); }
  ;

var_decl
  : type ID             
    { $$ = new VarDeclNode($1, *$2); delete $2; }
  | type ID '=' expr
    { $$ = new VarDeclNode($1, *$2, $4); delete $2; }
  ;

func_def
  : type ID '(' param_list ')' '{' block_items '}'
    {
       $$ = new FuncDefNode($1, *$2, *$4, $7); 
       delete $2;
    }
  ;

/* O listă mixtă de declarații și statement-uri pentru a evita conflictul */
block_items
  : /* empty */ { $$ = new BlockNode(); }
  | block_items var_decl ';' { $1->addStatement($2); $$ = $1; }
  | block_items statement    { $1->addStatement($2); $$ = $1; }
  ;

param_list
  : /* empty */ 
    { $$ = new std::vector<VarDeclNode*>(); }
  | param_list_nonempty 
    { $$ = $1; }
  ;

param_list_nonempty
  : param_decl 
    { 
      $$ = new std::vector<VarDeclNode*>(); 
      $$->push_back($1); 
    }
  | param_list_nonempty ',' param_decl
    {
      $1->push_back($3);
      $$ = $1;
    }
  ;

param_decl 
  : type ID { $$ = new VarDeclNode($1, *$2); delete $2; }
  ;

class_def
  : CLASS ID '{' class_member_list '}' ';' 
    { 
      ClassDefNode* cls = new ClassDefNode(*$2);
      cls->members = *$4;
      $$ = cls;
      delete $2;
    }
  ;

class_member_list
  : /* empty */ { $$ = new std::vector<ASTNode*>(); }
  | class_member_list class_member { $1->push_back($2); $$ = $1; }
  ;

class_member
  : var_decl ';' { $$ = $1; }
  | func_def     { $$ = $1; }
  ;

/* stmt_list ramane doar pentru if/while/main unde nu vrem declaratii (optional) */
stmt_list
  : /* empty */ { $$ = new BlockNode(); }
  | stmt_list statement { $1->addStatement($2); $$ = $1; }
  ;

statement
  : assignment ';'      { $$ = $1; }
  | expr ';'            { $$ = $1; }
  | PRINT '(' expr ')' ';' { $$ = new PrintNode($3); }
  | IF '(' expr ')' '{' stmt_list '}' { $$ = new IfNode($3, $6); }
  | WHILE '(' expr ')' '{' stmt_list '}' { $$ = new WhileNode($3, $6); }
  | RETURN expr ';'     { $$ = new ReturnNode($2); }
  | RETURN ';'          { $$ = new ReturnNode(nullptr); }
  ;

assignment
  : ID '=' expr 
    { $$ = new AssignNode(*$1, $3); delete $1; }
  ;

expr
  : INT_LITERAL       { $$ = new LiteralNode(to_string($1)); }
  | FLOAT_LITERAL     { $$ = new LiteralNode(to_string($1)); }
  | STRING_LITERAL    { $$ = new LiteralNode(*$1); delete $1; }
  | TRUE              { $$ = new LiteralNode("true"); }
  | FALSE             { $$ = new LiteralNode("false"); }
  | ID                { $$ = new IdNode(*$1); delete $1; }
  | expr '+' expr     { $$ = new BinaryExprNode("+", $1, $3); }
  | expr '-' expr     { $$ = new BinaryExprNode("-", $1, $3); }
  | expr '*' expr     { $$ = new BinaryExprNode("*", $1, $3); }
  | expr '/' expr     { $$ = new BinaryExprNode("/", $1, $3); }
  | expr AND expr     { $$ = new BinaryExprNode("&&", $1, $3); }
  | expr OR expr      { $$ = new BinaryExprNode("||", $1, $3); }
  | expr EQ expr      { $$ = new BinaryExprNode("==", $1, $3); }
  | expr NEQ expr     { $$ = new BinaryExprNode("!=", $1, $3); }
  | expr '<' expr     { $$ = new BinaryExprNode("<", $1, $3); }
  | expr '>' expr     { $$ = new BinaryExprNode(">", $1, $3); }
  | expr LE expr      { $$ = new BinaryExprNode("<=", $1, $3); }
  | expr GE expr      { $$ = new BinaryExprNode(">=", $1, $3); }
  | NOT expr          { $$ = new BinaryExprNode("!", $2, nullptr); }
  | '(' expr ')'      { $$ = $2; }
  | ID '(' arg_list_opt ')' 
    { 
      $$ = new CallNode(*$1, *$3); 
      delete $1; delete $3;
    }
  | expr '.' ID       { $$ = new DotNode($1, *$3); delete $3; }
  | expr '.' ID '(' arg_list_opt ')' 
    { 
       $$ = new CallNode("Method:" + *$3, *$5); 
       delete $3; delete $5;
    }
  ;

arg_list_opt
  : /* empty */ { $$ = new std::vector<ASTNode*>(); }
  | arg_list    { $$ = $1; }
  ;

arg_list
  : expr 
    { 
      $$ = new std::vector<ASTNode*>(); 
      $$->push_back($1); 
    }
  | arg_list ',' expr
    {
      $1->push_back($3);
      $$ = $1;
    }
  ;

type
  : INT     { $$ = new TypeInfo(TYPE_INT); }
  | FLOAT   { $$ = new TypeInfo(TYPE_FLOAT); }
  | VOID    { $$ = new TypeInfo(TYPE_VOID); }
  | BOOL    { $$ = new TypeInfo(TYPE_BOOL); }
  | STRING  { $$ = new TypeInfo(TYPE_STRING); }
  | ID      { $$ = new TypeInfo(*$1); delete $1; }
  ;

%%

void yyerror(const char* s) {
  std::cerr << "Error: " << s << " at line " << yylineno << std::endl;
}

int main() {
  if (yyparse() == 0) {
      if (root) {
          root->print();
      }
  }
  return 0;
}