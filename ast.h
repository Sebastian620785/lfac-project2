#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include "types.h"

using namespace std;

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void print(int level = 0) {
        for(int i=0; i<level; i++) cout << "  ";
    }
};

class ProgramNode : public ASTNode {
public:
    vector<ASTNode*> globals;
    ASTNode* mainBlock;
    void print(int level = 0) override {
        ASTNode::print(level); cout << "PROGRAM ROOT" << endl;
        for (auto n : globals) n->print(level + 1);
        if (mainBlock) mainBlock->print(level + 1);
    }
};

class BlockNode : public ASTNode {
public:
    vector<ASTNode*> statements;
    void addStatement(ASTNode* stmt) { if(stmt) statements.push_back(stmt); }
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Block {" << endl;
        for (auto s : statements) s->print(level + 1);
        ASTNode::print(level); cout << "}" << endl;
    }
};

class VarDeclNode : public ASTNode {
public:
    TypeInfo* type;
    string name;
    ASTNode* initVal;
    VarDeclNode(TypeInfo* t, string n, ASTNode* init = nullptr) : type(t), name(n), initVal(init) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "VarDecl: " << name << endl;
    }
};

class FuncDefNode : public ASTNode {
public:
    TypeInfo* returnType;
    string name;
    vector<VarDeclNode*> params;
    BlockNode* body;
    
    FuncDefNode(TypeInfo* rt, string n, vector<VarDeclNode*> p, BlockNode* b) 
        : returnType(rt), name(n), params(p), body(b) {}

    void print(int level = 0) override {
        ASTNode::print(level); cout << "Function: " << name << endl;
        cout << string(level+2, ' ') << "Params: " << params.size() << endl;
        for(auto p : params) p->print(level + 2);
        if(body) body->print(level + 1);
    }
};

class ClassDefNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> members;
    ClassDefNode(string n) : name(n) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Class: " << name << endl;
        for(auto m : members) m->print(level + 1);
    }
};

class MainNode : public ASTNode {
public:
    BlockNode* body;
    MainNode(BlockNode* b) : body(b) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "MAIN BLOCK" << endl;
        if(body) body->print(level + 1);
    }
};

class StmtNode : public ASTNode {
public:
    string info;
    StmtNode(string s) : info(s) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Statement: " << info << endl;
    }
};

class IfNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenBlock;
    IfNode(ASTNode* c, ASTNode* t) : condition(c), thenBlock(t) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "If" << endl;
        condition->print(level+1);
        thenBlock->print(level+1);
    }
};

class WhileNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;
    WhileNode(ASTNode* c, ASTNode* b) : condition(c), body(b) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "While" << endl;
        condition->print(level+1);
        body->print(level+1);
    }
};

class PrintNode : public ASTNode {
public:
    ASTNode* expr;
    PrintNode(ASTNode* e) : expr(e) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Print" << endl;
        expr->print(level+1);
    }
};

class AssignNode : public ASTNode {
public:
    string name;
    ASTNode* val;
    AssignNode(string n, ASTNode* v) : name(n), val(v) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Assign: " << name << endl;
        val->print(level+1);
    }
};

class ReturnNode : public ASTNode {
public:
    ASTNode* expr;
    ReturnNode(ASTNode* e = nullptr) : expr(e) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Return" << endl;
        if(expr) expr->print(level+1);
    }
};

class BinaryExprNode : public ASTNode {
public:
    string op;
    ASTNode* left;
    ASTNode* right;
    BinaryExprNode(string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
    
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Op: " << op << endl;
        if (left) left->print(level+1);   
        if (right) right->print(level+1); 
    }
};

class LiteralNode : public ASTNode {
public:
    string val;
    LiteralNode(string v) : val(v) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Literal: " << val << endl;
    }
};

class IdNode : public ASTNode {
public:
    string name;
    IdNode(string n) : name(n) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "ID: " << name << endl;
    }
};

class CallNode : public ASTNode {
public:
    string funcName;
    vector<ASTNode*> args;
    CallNode(string n, vector<ASTNode*> a = {}) : funcName(n), args(a) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Call: " << funcName << endl;
        for(auto a : args) a->print(level+1);
    }
};

class DotNode : public ASTNode {
public:
    ASTNode* obj;
    string member;
    DotNode(ASTNode* o, string m) : obj(o), member(m) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Access ." << member << endl;
        obj->print(level+1);
    }
};

#endif