#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include "types.h"

using namespace std;

class ast_node {
public:
    virtual ~ast_node() {}
};

class expr_node : public ast_node {
public:
    virtual ~expr_node() {}
};

class literal_node : public expr_node {
public:
    string val;
    literal_node(string v) : val(v) {}
};

class id_node : public expr_node {
public:
    string name;
    id_node(string n) : name(n) {}
};

class binary_expr_node : public expr_node {
public:
    string op;
    ast_node* left;
    ast_node* right;
    binary_expr_node(string o, ast_node* l, ast_node* r) : op(o), left(l), right(r) {}
};

class assign_node : public expr_node {
public:
    ast_node* left;
    ast_node* right;
    assign_node(ast_node* l, ast_node* r) : left(l), right(r) {}
};

class call_node : public expr_node {
public:
    string func_name;
    vector<ast_node*> args;
    call_node(string n, vector<ast_node*> a) : func_name(n), args(a) {}
};

class dot_node : public expr_node {
public:
    ast_node* obj;
    string member;
    dot_node(ast_node* o, string m) : obj(o), member(m) {}
};

#endif