#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include "types.h"

using namespace std;

//clasa de baza pentru toate nodurile din arbore(abstracta)
class ASTNode {
public:
    //destructor virtual necesar pentru a sterge corect obiectele derivate
    virtual ~ASTNode() {}
    
    //functie de afisare recursiva
    //level=nivelul de indentare(0 pentru radacina)
    virtual void print(int level = 0) {
        //afisam spatii pentru a simula structura arborescenta
        for(int i=0; i<level; i++) cout << "  ";
    }
};

//radacina intregului program
class ProgramNode : public ASTNode {
public:
    vector<ASTNode*> globals; //vector pentru variabile globale/functii/clase
    ASTNode* mainBlock;       //pointer catre blocul principal main()
    
    void print(int level = 0) override {
        ASTNode::print(level); cout << "PROGRAM ROOT" << endl;
        //iteram prin globale si le afisam
        for (auto n : globals) n->print(level + 1);
        //daca exista main,il afisam
        if (mainBlock) mainBlock->print(level + 1);
    }
};

//bloc de cod delimitat de acolade{...}
class BlockNode : public ASTNode {
public:
    vector<ASTNode*> statements; //lista de instructiuni din bloc
    
    //adaugam instructiune doar daca pointerul e valid
    void addStatement(ASTNode* stmt) { if(stmt) statements.push_back(stmt); }
    
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Block {" << endl;
        for (auto s : statements) s->print(level + 1);
        ASTNode::print(level); cout << "}" << endl;
    }
};

//declaratie variabila:tip nume(=valoare)
class VarDeclNode : public ASTNode {
public:
    TypeInfo* type;   //ex:INT,FLOAT
    string name;      //ex:"myVar"
    ASTNode* initVal; //ex:5 in "int x=5;",poate fi nullptr
    
    //constructor cu valoare de initializare optionala(default null)
    VarDeclNode(TypeInfo* t, string n, ASTNode* init = nullptr) : type(t), name(n), initVal(init) {}
    
    void print(int level = 0) override {
        ASTNode::print(level); cout << "VarDecl: " << name << endl;
        //afisam si initializarea doar daca exista
        /*if(initVal)... logica de afisare copil...*/
    }
};

//definitie functie:tip nume(params){body}
class FuncDefNode : public ASTNode {
public:
    TypeInfo* returnType;
    string name;
    vector<VarDeclNode*> params; //lista de parametri formali
    BlockNode* body;             //corpul functiei
    
    FuncDefNode(TypeInfo* rt, string n, vector<VarDeclNode*> p, BlockNode* b) 
        : returnType(rt), name(n), params(p), body(b) {}

    void print(int level = 0) override {
        ASTNode::print(level); cout << "Function: " << name << endl;
        //afisam numarul de parametri pentru debug
        cout << string(level+2, ' ') << "Params: " << params.size() << endl;
        for(auto p : params) p->print(level + 2);
        //afisam corpul functiei indentat
        if(body) body->print(level + 1);
    }
};

//definitie clasa:class Nume{membri}
class ClassDefNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> members; //variabile sau metode
    ClassDefNode(string n) : name(n) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Class: " << name << endl;
        for(auto m : members) m->print(level + 1);
    }
};

//nod special pentru main()
class MainNode : public ASTNode {
public:
    BlockNode* body;
    MainNode(BlockNode* b) : body(b) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "MAIN BLOCK" << endl;
        if(body) body->print(level + 1);
    }
};

//statement generic(wrapper)
class StmtNode : public ASTNode {
public:
    string info;
    StmtNode(string s) : info(s) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Statement: " << info << endl;
    }
};

//structura de control if(cond){then}
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

//structura repetitiva while(cond){body}
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

//nod pentru Print(expr)
class PrintNode : public ASTNode {
public:
    ASTNode* expr;
    PrintNode(ASTNode* e) : expr(e) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Print" << endl;
        expr->print(level+1);
    }
};

//atribuire:id=val
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

//return expr;sau return;
class ReturnNode : public ASTNode {
public:
    ASTNode* expr; //poate fi null pentru void
    ReturnNode(ASTNode* e = nullptr) : expr(e) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Return" << endl;
        if(expr) expr->print(level+1);
    }
};

//operatii binare:stanga op dreapta(ex:1+2)
//sau unare:op stanga(ex:!ok,unde right e null)
class BinaryExprNode : public ASTNode {
public:
    string op;      //operatorul:"+","-","&&","!"
    ASTNode* left;
    ASTNode* right;
    BinaryExprNode(string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
    
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Op: " << op << endl;
        //verificam pointerii ca sa nu avem segfault la operatori unari(ex:NOT)
        if (left) left->print(level+1);   
        if (right) right->print(level+1); 
    }
};

//valori constante:10,3.14,"text",true
class LiteralNode : public ASTNode {
public:
    string val;
    LiteralNode(string v) : val(v) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "Literal: " << val << endl;
    }
};

//folosirea unei variabile(identifier)
class IdNode : public ASTNode {
public:
    string name;
    IdNode(string n) : name(n) {}
    void print(int level = 0) override {
        ASTNode::print(level); cout << "ID: " << name << endl;
    }
};

//apel functie:nume(arg1,arg2...)
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

//acces membru:obiect.membru
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