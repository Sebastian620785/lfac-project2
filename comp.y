%code requires {
  #include <string>
  #include <vector>
  #include "types.h"
  #include "ast.h"
}

%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "types.h"
#include "symbol_table.h" 
#include "ast.h"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

ScopeManager scopeManager; 

bool isLValue(ast_node* node) {
    if (dynamic_cast<id_node*>(node)) return true;
    if (dynamic_cast<dot_node*>(node)) return true;
    return false;
}

TypeInfo inferType(ast_node* node) {
    if (!node) return TypeInfo(TYPE_UNKNOWN);

    if (dynamic_cast<literal_node*>(node)) {
        literal_node* lit = dynamic_cast<literal_node*>(node);
        if (lit->val.find("\"") != string::npos) return TypeInfo(TYPE_STRING);
        if (lit->val == "true" || lit->val == "false") return TypeInfo(TYPE_BOOL);
        if (lit->val.find(".") != string::npos) return TypeInfo(TYPE_FLOAT);
        return TypeInfo(TYPE_INT); 
    }

    if (dynamic_cast<id_node*>(node)) {
        id_node* id = dynamic_cast<id_node*>(node);
        SymbolInfo* sym = scopeManager.currentScope->lookup(id->name);
        if (!sym) return TypeInfo(TYPE_UNKNOWN); 
        return sym->type;
    }

    if (dynamic_cast<assign_node*>(node)) {
        assign_node* asgn = dynamic_cast<assign_node*>(node);
        return inferType(asgn->left);
    }

    if (dynamic_cast<binary_expr_node*>(node)) {
        binary_expr_node* bin = dynamic_cast<binary_expr_node*>(node);
        TypeInfo leftT = inferType(bin->left);
        TypeInfo rightT = inferType(bin->right);
        string op = bin->op;
        
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" ||
            op == "&&" || op == "||" || op == "!") 
            return TypeInfo(TYPE_BOOL);
        
        if (leftT != rightT) return TypeInfo(TYPE_UNKNOWN);
        return leftT; 
    }
    
    if (dynamic_cast<call_node*>(node)) {
        call_node* call = dynamic_cast<call_node*>(node);
        SymbolInfo* sym = scopeManager.currentScope->lookup(call->func_name);
        if (sym) return sym->type; 
        return TypeInfo(TYPE_UNKNOWN);
    }
    
    if (dynamic_cast<dot_node*>(node)) {
        dot_node* dot = dynamic_cast<dot_node*>(node);
        if (dynamic_cast<id_node*>(dot->obj)) {
             id_node* idObj = dynamic_cast<id_node*>(dot->obj);
             SymbolInfo* symObj = scopeManager.currentScope->lookup(idObj->name);
             if (symObj && symObj->type.type == TYPE_CLASS) {
                 SymbolInfo* memberSym = scopeManager.lookupInClass(symObj->type.className, dot->member);
                 if (memberSym) return memberSym->type;
             }
             if (scopeManager.classScopes.count(idObj->name)) {
                 SymbolInfo* memberSym = scopeManager.lookupInClass(idObj->name, dot->member);
                 if (memberSym) return memberSym->type;
             }
        }
        return TypeInfo(TYPE_UNKNOWN); 
    }
    return TypeInfo(TYPE_UNKNOWN);
}
%}

%union {
  int           Int;
  float         Float;
  std::string   *String;
  TypeInfo      *TypeVal;
  ast_node      *Node;
  std::vector<ast_node*> *NodeList; 
  std::vector<TypeInfo>  *TypeList;
}

%token INT FLOAT STRING BOOL VOID CLASS MAIN IF WHILE RETURN PRINT TRUE FALSE
%token <String> ID STRING_LITERAL
%token <Int> INT_LITERAL
%token <Float> FLOAT_LITERAL
%token EQ NEQ LE GE AND OR NOT

%right '='
%left OR
%left AND
%left EQ NEQ
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right NOT
%left '.' '[' '('

%type <TypeVal> standard_type
%type <NodeList> arg_list arg_list_opt
%type <TypeList> param_list param_list_nonempty
%type <Node> expr bool_expr
%type <TypeVal> param_decl

%start program

%%

program : global_list main_block ;

global_list
  : /* empty */ 
  | global_list var_decl ';'
  | global_list func_def
  | global_list class_def
  ;

main_block : MAIN '(' ')' '{' stmt_list '}' ;

var_decl
  : standard_type ID { 
      if (scopeManager.currentScope->lookupCurrent(*$2)) {
          yyerror(("Semantic Error: Variable '" + *$2 + "' redeclared.").c_str()); exit(1);
      }
      scopeManager.currentScope->addSymbol(SymbolInfo(*$2, *$1, "variable"));
      delete $2; 
    }
  | standard_type ID '=' expr { 
      if (scopeManager.currentScope->lookupCurrent(*$2)) {
          yyerror(("Semantic Error: Variable '" + *$2 + "' redeclared.").c_str()); exit(1);
      }
      scopeManager.currentScope->addSymbol(SymbolInfo(*$2, *$1, "variable"));
      TypeInfo exprT = inferType($4);
      if (*$1 != exprT && exprT.type != TYPE_UNKNOWN) {
           yyerror(("Semantic Error: Type mismatch init '" + *$2 + "'.").c_str()); exit(1);
      }
      delete $2; 
    }
  | ID ID {
      if (scopeManager.currentScope->lookupCurrent(*$2)) {
          yyerror(("Semantic Error: Variable '" + *$2 + "' redeclared.").c_str()); exit(1);
      }
      if (!scopeManager.classScopes.count(*$1)) {
           yyerror(("Semantic Error: Class '" + *$1 + "' undefined.").c_str()); exit(1);
      }
      TypeInfo t(*$1);
      scopeManager.currentScope->addSymbol(SymbolInfo(*$2, t, "variable"));
      delete $1; delete $2;
    }
  | ID ID '=' expr {
      if (scopeManager.currentScope->lookupCurrent(*$2)) {
          yyerror(("Semantic Error: Variable '" + *$2 + "' redeclared.").c_str()); exit(1);
      }
      if (!scopeManager.classScopes.count(*$1)) {
           yyerror(("Semantic Error: Class '" + *$1 + "' undefined.").c_str()); exit(1);
      }
      TypeInfo t(*$1);
      scopeManager.currentScope->addSymbol(SymbolInfo(*$2, t, "variable"));
      delete $1; delete $2;
    }
  ;

func_def
  : standard_type ID '(' { 
        if (scopeManager.currentScope->lookupCurrent(*$2)) {
            yyerror(("Semantic Error: Function '" + *$2 + "' redeclared.").c_str()); exit(1);
        }
        SymbolInfo funcSym(*$2, *$1, "function");
        scopeManager.currentScope->addSymbol(funcSym); 
        scopeManager.enterScope("func_" + *$2); 
    }
    param_list ')' {
        SymbolInfo* s = scopeManager.currentScope->getParent()->lookup(*$2);
        if(s) {
            //convertim manual din vector<TypeInfo> in vector<SymbolType>
            for(const auto& tInfo : *$5) {
                SymbolType st = SYM_UNKNOWN;
                if(tInfo.type == TYPE_INT) st = SYM_INT;
                else if(tInfo.type == TYPE_FLOAT) st = SYM_FLOAT;
                else if(tInfo.type == TYPE_BOOL) st = SYM_BOOL;
                else if(tInfo.type == TYPE_STRING) st = SYM_STRING;
                else if(tInfo.type == TYPE_CLASS) st = SYM_CLASS;
                
                s->paramTypes.push_back(st);
            }
        }
    }
    '{' block_items '}' {
       scopeManager.exitScope();
       delete $2;
    }
  ;

block_items
  : /* empty */
  | block_items var_decl ';'
  | block_items statement
  ;

param_list
  : /* empty */         { $$ = new std::vector<TypeInfo>(); }
  | param_list_nonempty { $$ = $1; }
  ;

param_list_nonempty
  : param_decl { $$ = new std::vector<TypeInfo>(); $$->push_back(*$1); }
  | param_list_nonempty ',' param_decl { $1->push_back(*$3); $$ = $1; }
  ;

param_decl 
  : standard_type ID { 
        if (scopeManager.currentScope->lookupCurrent(*$2)) {
             yyerror("Semantic Error: Parameter redeclared."); exit(1);
        }
        scopeManager.currentScope->addSymbol(SymbolInfo(*$2, *$1, "parameter"));
        $$ = $1; delete $2; 
    }
  ;

class_def
  : CLASS ID '{' { 
      if (scopeManager.currentScope->lookupCurrent(*$2)) {
             yyerror("Semantic Error: Class redeclared."); exit(1);
      }
      scopeManager.currentScope->addSymbol(SymbolInfo(*$2, TypeInfo(*$2), "class"));
      scopeManager.enterScope("class_" + *$2);
      scopeManager.saveClassScope(*$2);
    }
    class_member_list '}' ';' { 
      scopeManager.exitScope();
      delete $2;
    }
  ;

class_member_list
  : /* empty */
  | class_member_list var_decl ';'
  | class_member_list func_def
  ;

stmt_list
  : /* empty */
  | stmt_list statement
  ;

statement
  : expr ';'
  | PRINT '(' expr ')' ';' 
  | IF '(' bool_expr ')' '{' stmt_list '}' 
  | WHILE '(' bool_expr ')' '{' stmt_list '}' 
  | RETURN expr ';'
  | RETURN ';'
  ;

bool_expr
  : expr {
       TypeInfo t = inferType($1);
       if (t.type != TYPE_BOOL && t.type != TYPE_UNKNOWN) {
           yyerror("Semantic Error: Condition must be boolean."); exit(1);
       }
       $$ = $1;
    }
  ;

expr
  : INT_LITERAL       { $$ = new literal_node(to_string($1)); }
  | FLOAT_LITERAL     { $$ = new literal_node(to_string($1)); }
  | STRING_LITERAL    { $$ = new literal_node(*$1); delete $1; }
  | TRUE              { $$ = new literal_node("true"); }
  | FALSE             { $$ = new literal_node("false"); }
  | ID                { $$ = new id_node(*$1); delete $1; }
  | expr '=' expr {
        if (!isLValue($1)) { yyerror("Semantic Error: Left side must be variable."); exit(1); }
        TypeInfo l = inferType($1);
        TypeInfo r = inferType($3);
        if (l != r && l.type != TYPE_UNKNOWN && r.type != TYPE_UNKNOWN) {
            yyerror("Semantic Error: Type mismatch in assignment."); exit(1);
        }
        $$ = new assign_node($1, $3);
    }
  | expr '+' expr     { 
       TypeInfo t1 = inferType($1); TypeInfo t2 = inferType($3);
       if (t1 != t2) { yyerror("Semantic Error: Type mismatch (+)."); exit(1); }
       $$ = new binary_expr_node("+", $1, $3); 
    }
  | expr '-' expr     { 
       TypeInfo t1 = inferType($1); TypeInfo t2 = inferType($3);
       if (t1 != t2) { yyerror("Semantic Error: Type mismatch (+)."); exit(1); }
       $$ = new binary_expr_node("-", $1, $3); 
    }
  | expr '*' expr     { 
       TypeInfo t1 = inferType($1); TypeInfo t2 = inferType($3);
       if (t1 != t2) { yyerror("Semantic Error: Type mismatch (+)."); exit(1); }
       $$ = new binary_expr_node("*", $1, $3); 
    }
  | expr '/' expr     { 
       TypeInfo t1 = inferType($1); TypeInfo t2 = inferType($3);
       if (t1 != t2) { yyerror("Semantic Error: Type mismatch (+)."); exit(1); }
       $$ = new binary_expr_node("/", $1, $3); 
    }
  | expr AND expr     { $$ = new binary_expr_node("&&", $1, $3); }
  | expr OR expr      { $$ = new binary_expr_node("||", $1, $3); }
  | expr EQ expr      { $$ = new binary_expr_node("==", $1, $3); }
  | expr NEQ expr     { $$ = new binary_expr_node("!=", $1, $3); }
  | expr '<' expr     { $$ = new binary_expr_node("<", $1, $3); }
  | expr '>' expr     { $$ = new binary_expr_node(">", $1, $3); }
  | expr LE expr      { $$ = new binary_expr_node("<=", $1, $3); }
  | expr GE expr      { $$ = new binary_expr_node(">=", $1, $3); }
  | NOT expr          { $$ = new binary_expr_node("!", $2, nullptr); }
  | '(' expr ')'      { $$ = $2; }
  | ID '(' arg_list_opt ')' { 
      SymbolInfo* f = scopeManager.currentScope->lookup(*$1);
      if (!f) { yyerror("Semantic Error: Function undefined."); exit(1); }
      if (f->paramTypes.size() != $3->size()) { yyerror("Semantic Error: Arg count mismatch."); exit(1); }
      for(size_t i=0; i<$3->size(); ++i) {
          TypeInfo argT = inferType((*$3)[i]);
          if (argT.type != TYPE_UNKNOWN) {
              bool match = false;
              if (argT.type == TYPE_INT && f->paramTypes[i] == SYM_INT) match = true;
              else if (argT.type == TYPE_FLOAT && f->paramTypes[i] == SYM_FLOAT) match = true;
              else if (argT.type == TYPE_BOOL && f->paramTypes[i] == SYM_BOOL) match = true;
              else if (argT.type == TYPE_STRING && f->paramTypes[i] == SYM_STRING) match = true;
              else if (argT.type == TYPE_CLASS && f->paramTypes[i] == SYM_CLASS) match = true;
              if(!match) { yyerror("Semantic Error: Arg type mismatch."); exit(1); }
          }
      }
      $$ = new call_node(*$1, *$3); delete $1; delete $3;
    }
  | expr '.' ID { 
       dot_node* node = new dot_node($1, *$3);
       TypeInfo t = inferType(node);
       if (t.type == TYPE_UNKNOWN) { yyerror("Semantic Error: Invalid member access."); exit(1); }
       $$ = node; delete $3; 
    }
  | expr '.' ID '(' arg_list_opt ')' { $$ = new call_node("method", *$5); }
  ;

arg_list_opt
  : /* empty */ { $$ = new std::vector<ast_node*>(); }
  | arg_list    { $$ = $1; }
  ;

arg_list
  : expr { $$ = new std::vector<ast_node*>(); $$->push_back($1); }
  | arg_list ',' expr { $1->push_back($3); $$ = $1; }
  ;

standard_type
  : INT     { $$ = new TypeInfo(TYPE_INT); }
  | FLOAT   { $$ = new TypeInfo(TYPE_FLOAT); }
  | VOID    { $$ = new TypeInfo(TYPE_VOID); }
  | BOOL    { $$ = new TypeInfo(TYPE_BOOL); }
  | STRING  { $$ = new TypeInfo(TYPE_STRING); }
  ;

%%

void yyerror(const char* s) {
  std::cerr << "Error: " << s << " at line " << yylineno << std::endl;
}

int main() {
  yyparse();
  return 0;
}