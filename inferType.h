#ifndef INFER_TYPE_H
#define INFER_TYPE_H

#include <iostream>
#include <string>
#include <vector>
#include "types.h"
#include "ast.h"
#include "symbol_table.h"

using namespace std;

extern int semantic_errors;

extern int yylineno;

inline void yyerror(const char* s) {
    std::cerr << "Error: " << s << " at line " << yylineno << std::endl;
    semantic_errors++;
}
extern ScopeManager scopeManager;

inline bool isLValue(ast_node* node) {
    if (dynamic_cast<id_node*>(node)) return true;
    if (dynamic_cast<dot_node*>(node)) return true;
    return false;
}

inline TypeInfo inferType(ast_node* node) {
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
        
        if (!sym) { 
            string err = "Semantic Error: Variable '" + id->name + "' undefined.";
            yyerror(err.c_str()); 
            return TypeInfo(TYPE_UNKNOWN); 
        } 
        return sym->type;
    }

    if (dynamic_cast<assign_node*>(node)) {
        assign_node* asgn = dynamic_cast<assign_node*>(node);
        SymbolInfo* sym = scopeManager.currentScope->lookup(asgn->name);
        if (!sym) return TypeInfo(TYPE_UNKNOWN);
        return sym->type;
    }

    if (dynamic_cast<binary_expr_node*>(node)) {
        binary_expr_node* bin = dynamic_cast<binary_expr_node*>(node);
        TypeInfo leftT = inferType(bin->left);
        TypeInfo rightT = inferType(bin->right);
        string op = bin->op;
        
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" ||
            op == "&&" || op == "||" || op == "!") 
            return TypeInfo(TYPE_BOOL);
        
        // Propagare eroare
        if (leftT.type == TYPE_UNKNOWN || rightT.type == TYPE_UNKNOWN) return TypeInfo(TYPE_UNKNOWN);

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
            
            if (!symObj) {
                return TypeInfo(TYPE_UNKNOWN);
            }

            if (symObj->type.type == TYPE_CLASS) {
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
    
    if (dynamic_cast<member_assign_node*>(node)) {
        member_assign_node* ma = dynamic_cast<member_assign_node*>(node);
        return inferType(ma->val);
    }

    if (dynamic_cast<method_call_node*>(node)) {
        auto* mc = dynamic_cast<method_call_node*>(node);
        if (auto* idObj = dynamic_cast<id_node*>(mc->obj)) {
            SymbolInfo* symObj = scopeManager.currentScope->lookup(idObj->name);
            if (!symObj || symObj->type.type != TYPE_CLASS) return TypeInfo(TYPE_UNKNOWN);

            SymbolInfo* m = scopeManager.lookupInClass(symObj->type.className, mc->method);
            if (!m || m->category != "function") return TypeInfo(TYPE_UNKNOWN);

            return m->type; 
        }
        return TypeInfo(TYPE_UNKNOWN);
    }

    return TypeInfo(TYPE_UNKNOWN);
}

#endif