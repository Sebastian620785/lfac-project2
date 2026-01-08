#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include "types.h"
#include "value.h"
#include "symbol_table.h"
#include "SymTableStub.h"

using namespace std;

class ast_node {
public:
    virtual ~ast_node() {}
    
    virtual Value eval(void* scope) {
        return Value();
    }
};

class program_node : public ast_node {
public:
    vector<ast_node*> globals;
    ast_node* main_block;

    program_node() : main_block(nullptr) {}

    Value eval(void* scope) override {
        for (auto g : globals) {
            if (g) g->eval(scope);
        }
        if (main_block) return main_block->eval(scope);
        return Value();
    }
};

class block_node : public ast_node {
public:
    vector<ast_node*> statements;

    void addStatement(ast_node* stmt) {
        if (stmt) {
            statements.push_back(stmt);
        }
    }

    Value eval(void* scope) override {
        Value last;
        for (auto s : statements) {
            if (!s) continue;
            
            last = s->eval(scope);
            if (last.hasReturn) {
                return last;
            }
        }
        return last;
    }
};

class main_node : public ast_node {
public:
    block_node* body;

    main_node(block_node* b) : body(b) {}

    Value eval(void* scope) override {
        if (body) {
            return body->eval(scope);
        }
        return Value();
    }
};

class var_decl_node : public ast_node {
public:
    TypeInfo* type;
    string name;
    ast_node* init_val;

    var_decl_node(TypeInfo* t, string n, ast_node* init = nullptr) 
        : type(t), name(n), init_val(init) {}

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        if (!st) return Value();

        Value v;
        if (init_val) {
            v = init_val->eval(scope);
        } else {
            if (type && type->type == TYPE_INT) v = Value(0);
            else if (type && type->type == TYPE_FLOAT) v = Value(0.0f);
            else if (type && type->type == TYPE_BOOL) v = Value(false);
            else if (type && type->type == TYPE_STRING) v = Value(std::string(""));
            else v = Value();
        }
        st->setValue(name, v);
        return v;
    }
};

class func_def_node : public ast_node {
public:
    TypeInfo* return_type;
    string name;
    std::vector<std::string> param_names; 
    block_node* body;

    func_def_node(TypeInfo* type, std::string n, std::vector<std::string> params, block_node* b)
        : return_type(type), name(n), param_names(params), body(b) {}

    Value eval(void* scope) override { return Value(); }
};

class class_def_node : public ast_node {
public:
    string name;
    vector<ast_node*> members;

    class_def_node(string n) : name(n) {}
    
    Value eval(void* scope) override { return Value(); }
};

class if_node : public ast_node {
public:
    ast_node* condition;
    ast_node* then_block;

    if_node(ast_node* c, ast_node* t) : condition(c), then_block(t) {}

    Value eval(void* scope) override {
        Value c = condition->eval(scope);
        if (c.type == VAL_BOOL && c.b) {
            return then_block ? then_block->eval(scope) : Value();
        }
        return Value();
    }
};

class while_node : public ast_node {
public:
    ast_node* condition;
    ast_node* body;

    while_node(ast_node* c, ast_node* b) : condition(c), body(b) {}

    Value eval(void* scope) override {
        Value last;
        while (true) {
            Value c = condition->eval(scope);
            if (!(c.type == VAL_BOOL && c.b)) break;

            if (body) {
                last = body->eval(scope);
                if (last.hasReturn) return last;
            }
        }
        return last;
    }
};

class return_node : public ast_node {
public:
    ast_node* expr;

    return_node(ast_node* e = nullptr) : expr(e) {}

    Value eval(void* scope) override {
        Value v;
        if (expr) {
            v = expr->eval(scope);
        }
        v.hasReturn = true;
        return v;
    }
};

class assign_node : public ast_node {
public:
    string name;
    ast_node* val;

    assign_node(string n, ast_node* v) : name(n), val(v) {}

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        Value rhs = val->eval(scope);
        if (st) {
            st->setValue(name, rhs);
        }
        return rhs;
    }
};

class member_assign_node : public ast_node {
public:
    ast_node* obj;
    string member;
    ast_node* val;

    member_assign_node(ast_node* o, string m, ast_node* v) : obj(o), member(m), val(v) {}

    Value eval(void* scope) override {
        if (val) return val->eval(scope);
        return Value();
    }
};

class binary_expr_node : public ast_node {
public:
    string op;
    ast_node* left;
    ast_node* right;

    binary_expr_node(string o, ast_node* l, ast_node* r) : op(o), left(l), right(r) {}

    Value eval(void* scope) override {
        Value leftVal = left->eval(scope);

        if (right == nullptr) {
            if (op == "!") {
                if (leftVal.type == VAL_BOOL) return Value(!leftVal.b);
            }
            return Value();
        }

        Value rightVal = right->eval(scope);

        if (op == "+") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i + rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f + rightVal.f);
            if (leftVal.type == VAL_STRING) return Value(leftVal.s + rightVal.s);
        }
        if (op == "-") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i - rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f - rightVal.f);
        }
        if (op == "*") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i * rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f * rightVal.f);
        }
        if (op == "/") {
            if (leftVal.type == VAL_INT) {
                if (rightVal.i == 0) return Value();
                return Value(leftVal.i / rightVal.i);
            }
            if (leftVal.type == VAL_FLOAT) {
                if (rightVal.f == 0.0f) return Value();
                return Value(leftVal.f / rightVal.f);
            }
        }
        if (op == "<") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i < rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f < rightVal.f);
        }
        if (op == ">") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i > rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f > rightVal.f);
        }
        if (op == "<=") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i <= rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f <= rightVal.f);
        }
        if (op == ">=") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i >= rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f >= rightVal.f);
        }
        if (op == "==") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i == rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f == rightVal.f);
            if (leftVal.type == VAL_BOOL) return Value(leftVal.b == rightVal.b);
            if (leftVal.type == VAL_STRING) return Value(leftVal.s == rightVal.s);
        }
        if (op == "!=") {
            if (leftVal.type == VAL_INT) return Value(leftVal.i != rightVal.i);
            if (leftVal.type == VAL_FLOAT) return Value(leftVal.f != rightVal.f);
            if (leftVal.type == VAL_BOOL) return Value(leftVal.b != rightVal.b);
            if (leftVal.type == VAL_STRING) return Value(leftVal.s != rightVal.s);
        }
        if (op == "&&" && leftVal.type == VAL_BOOL) return Value(leftVal.b && rightVal.b);
        if (op == "||" && leftVal.type == VAL_BOOL) return Value(leftVal.b || rightVal.b);

        return Value();
    }
};

class literal_node : public ast_node {
public:
    string val;

    literal_node(string v) : val(v) {}

    Value eval(void* scope) override {
        if (val == "true") return Value(true);
        if (val == "false") return Value(false);
        if (val.find("\"") != string::npos) {
            string s = val;
            if(s.size() >= 2) s = s.substr(1, s.size()-2);
            return Value(s);
        }
        bool isFloat = (val.find('.') != string::npos);
        if (isFloat) return Value(std::stof(val));
        return Value(std::stoi(val));
    }
};

class id_node : public ast_node {
public:
    string name;

    id_node(string n) : name(n) {}

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        if (!st) return Value();
        return st->getValue(name);
    }
};

class call_node : public ast_node {
public:
    string func_name;
    vector<ast_node*> args;
    TypeInfo ret_type;

    call_node(string n, vector<ast_node*> a, TypeInfo rt)
        : func_name(n), args(a), ret_type(rt) {}

    Value eval(void* scope) override {
        if (ret_type.type == TYPE_INT) return Value(0);
        if (ret_type.type == TYPE_FLOAT) return Value(0.0f);
        if (ret_type.type == TYPE_BOOL) return Value(false);
        if (ret_type.type == TYPE_STRING) return Value(std::string(""));
        return Value();
    }
};

class dot_node : public ast_node {
public:
    ast_node* obj;
    string member;

    dot_node(ast_node* o, string m) : obj(o), member(m) {}

    Value eval(void* scope) override {
        return Value();
    }
};

class method_call_node : public ast_node {
public:
    ast_node* obj;
    std::string method;
    std::vector<ast_node*> args;

    method_call_node(ast_node* o, const std::string& m, std::vector<ast_node*> a)
        : obj(o), method(m), args(std::move(a)) {}

    Value eval(void* scope) override {
        return Value();
    }
};

class print_node : public ast_node {
public:
    ast_node* expr;

    print_node(ast_node* e) : expr(e) {}

    Value eval(void* scope) override {
        Value v = expr->eval(scope);
        cout << v.toString() << endl;
        return v;
    }
};

#endif