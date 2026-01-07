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

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void print(int level = 0) {
        for (int i = 0; i < level; i++) cout << "  ";
    }
    virtual Value eval(void* scope) {
        return Value();
    }
};

class ProgramNode : public ASTNode {
public:
    vector<ASTNode*> globals;
    ASTNode* mainBlock;

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "PROGRAM ROOT" << endl;
        for (auto n : globals) {
            if (n) n->print(level + 1);
        }
        if (mainBlock) mainBlock->print(level + 1);
    }

    Value eval(void* scope) override {
        for (auto g : globals) {
            if (g) g->eval(scope);
        }
        if (mainBlock) return mainBlock->eval(scope);
        return Value();
    }
};

class BlockNode : public ASTNode {
public:
    vector<ASTNode*> statements;

    void addStatement(ASTNode* stmt) {
        if (stmt) {
            statements.push_back(stmt);
        }
    }

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Block {" << endl;
        for (auto s : statements) {
            if (s) s->print(level + 1);
        }
        ASTNode::print(level);
        cout << "}" << endl;
    }

    Value eval(void* scope) override {
        Value last;
        for (auto s : statements) {
            if (!s) {
                continue;
            }
            last = s->eval(scope);
            if (last.hasReturn) {
                return last;
            }
        }
        return last;
    }
};
class MemberAssignNode : public ASTNode {
public:
    ASTNode* obj;
    string member;
    ASTNode* val;

    MemberAssignNode(ASTNode* o, string m, ASTNode* v) : obj(o), member(m), val(v) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "MemberAssign: ." << member << endl;
        if (obj) obj->print(level + 1);
        if (val) val->print(level + 1);
    }

    Value eval(void* scope) override {
        if (val) return val->eval(scope);
        return Value();
    }
};

class VarDeclNode : public ASTNode {
public:
    TypeInfo* type;
    string name;
    ASTNode* initVal;

    VarDeclNode(TypeInfo* t, string n, ASTNode* init = nullptr) 
        : type(t), name(n), initVal(init) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "VarDecl: " << name << endl;
    }

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        if (!st) {
            return Value();
        }

        Value v;
        if (initVal) {
            v = initVal->eval(scope);
        } else {
            // Valori default
            if (type && type->type == TYPE_INT)
                v = Value(0);
            else if (type && type->type == TYPE_FLOAT)
                v = Value(0.0f);
            else if (type && type->type == TYPE_BOOL)
                v = Value(false);
            else if (type && type->type == TYPE_STRING)
                v = Value(std::string(""));
            else
                v = Value();
        }
        st->setValue(name, v);
        return v;
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
        ASTNode::print(level);
        cout << "Function: " << name << endl;
        cout << string(level + 2, ' ') << "Params: " << params.size() << endl;
        for (auto p : params) {
            if (p) p->print(level + 2);
        }
        if (body) body->print(level + 1);
    }
};

class ClassDefNode : public ASTNode {
public:
    string name;
    vector<ASTNode*> members;

    ClassDefNode(string n) : name(n) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Class: " << name << endl;
        for (auto m : members) {
            if (m) m->print(level + 1);
        }
    }
};

class MainNode : public ASTNode {
public:
    BlockNode* body;

    MainNode(BlockNode* b) : body(b) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "MAIN BLOCK" << endl;
        if (body) body->print(level + 1);
    }

    Value eval(void* scope) override {
        if (body) {
            return body->eval(scope);
        }
        return Value();
    }
};

class StmtNode : public ASTNode {
public:
    string info;

    StmtNode(string s) : info(s) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Statement: " << info << endl;
    }
};

class IfNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenBlock;

    IfNode(ASTNode* c, ASTNode* t) : condition(c), thenBlock(t) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "If" << endl;
        if (condition) condition->print(level + 1);
        if (thenBlock) thenBlock->print(level + 1);
    }

    Value eval(void* scope) override {
        Value c = condition->eval(scope);
        if (c.type == VAL_BOOL && c.b) {
            return thenBlock ? thenBlock->eval(scope) : Value();
        }
        return Value();
    }
};

class WhileNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;

    WhileNode(ASTNode* c, ASTNode* b) : condition(c), body(b) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "While" << endl;
        if (condition) condition->print(level + 1);
        if (body) body->print(level + 1);
    }

    Value eval(void* scope) override {
        Value last;
        while (true) {
            Value c = condition->eval(scope);
            if (!(c.type == VAL_BOOL && c.b)) break;

            if (body) {
                last = body->eval(scope);
                if (last.hasReturn)
                    return last;
            }
        }
        return last;
    }
};

class PrintNode : public ASTNode {
public:
    ASTNode* expr;

    PrintNode(ASTNode* e) : expr(e) {}

    Value eval(void* scope) override {
        Value v = expr->eval(scope);
        cout << v.toString() << endl;
        return v;
    }

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Print" << endl;
        if (expr) expr->print(level + 1);
    }
};

class AssignNode : public ASTNode {
public:
    string name;
    ASTNode* val;

    AssignNode(string n, ASTNode* v) : name(n), val(v) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Assign: " << name << endl;
        if (val) val->print(level + 1);
    }

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        Value rhs = val->eval(scope);
        if (st) {
            st->setValue(name, rhs);
        }
        return rhs;
    }
};

class ReturnNode : public ASTNode {
public:
    ASTNode* expr;

    ReturnNode(ASTNode* e = nullptr) : expr(e) {}

    Value eval(void* scope) override {
        Value v;
        if (expr) {
            v = expr->eval(scope);
        }
        v.hasReturn = true;
        return v;
    }

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Return" << endl;
        if (expr) expr->print(level + 1);
    }
};

class BinaryExprNode : public ASTNode {
public:
    string op;
    ASTNode* left;
    ASTNode* right;

    BinaryExprNode(string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Op: " << op << endl;
        if (left) left->print(level + 1);
        if (right) right->print(level + 1);
    }

    Value eval(void* scope) override {
        Value leftVal = left->eval(scope);

        // operator unar
        if (right == nullptr) {
            if (op == "!") {
                if (leftVal.type == VAL_BOOL)
                    return Value(!leftVal.b);
            }
            return Value();
        }

        Value rightVal = right->eval(scope);

        // aritmetici
        if (op == "+") {
            if (leftVal.type == VAL_INT)
                return Value(leftVal.i + rightVal.i);
            if (leftVal.type == VAL_FLOAT)
                return Value(leftVal.f + rightVal.f);
            if (leftVal.type == VAL_STRING)
                return Value(leftVal.s + rightVal.s);
        }

        if (op == "-") {
            if (leftVal.type == VAL_INT)
                return Value(leftVal.i - rightVal.i);
            if (leftVal.type == VAL_FLOAT)
                return Value(leftVal.f - rightVal.f);
        }

        if (op == "*") {
            if (leftVal.type == VAL_INT)
                return Value(leftVal.i * rightVal.i);
            if (leftVal.type == VAL_FLOAT)
                return Value(leftVal.f * rightVal.f);
        }

        if (op == "/") {
            if (leftVal.type == VAL_INT) {
                if (rightVal.i == 0) {
                    cout << " nu se poate impartii la 0" << endl;
                    return Value();
                }
                return Value(leftVal.i / rightVal.i);
            }

            if (leftVal.type == VAL_FLOAT) {
                if (rightVal.f == 0.0f) {
                    cout << " nu se poate impartii la 0" << endl;
                    return Value();
                }
                return Value(leftVal.f / rightVal.f);
            }
        }

        // comparatii
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

        // logici
        if (op == "&&") {
            if (leftVal.type == VAL_BOOL) return Value(leftVal.b && rightVal.b);
        }

        if (op == "||") {
            if (leftVal.type == VAL_BOOL) return Value(leftVal.b || rightVal.b);
        }

        return Value();
    }
};

class LiteralNode : public ASTNode {
public:
    string val;

    LiteralNode(string v) : val(v) {}

    Value eval(void* scope) override {
        if (val == "true") return Value(true);
        if (val == "false") return Value(false);
        
        bool isInt = true;
        bool isFloat = false;
        
        for (char c : val) {
            if (c == '.') {
                isFloat = true;
                isInt = false;
                break;
            }
            if (c < '0' || c > '9') {
                isInt = false;
                break;
            }
        }

        if (isInt) {
            return Value(std::stoi(val));
        }
        if (isFloat) {
            return Value(std::stof(val));
        }
        return Value(val);
    }

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Literal: " << val << endl;
    }
};

class IdNode : public ASTNode {
public:
    string name;

    IdNode(string n) : name(n) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "ID: " << name << endl;
    }

    Value eval(void* scope) override {
        SymTableStub* st = (SymTableStub*)scope;
        if (!st) return Value();
        return st->getValue(name);
    }
};

class CallNode : public ASTNode {
public:
    string funcName;
    vector<ASTNode*> args;
    TypeInfo retType;

    CallNode(string n, vector<ASTNode*> a, TypeInfo rt)
        : funcName(n), args(a), retType(rt) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Call: " << funcName << endl;
        for (auto a : args) {
            if (a) a->print(level + 1);
        }
    }

    Value eval(void* scope) override {
        if (retType.type == TYPE_INT) return Value(0);
        if (retType.type == TYPE_FLOAT) return Value(0.0f);
        if (retType.type == TYPE_BOOL) return Value(false);
        if (retType.type == TYPE_STRING) return Value(std::string(""));
        return Value();
    }
};

class DotNode : public ASTNode {
public:
    ASTNode* obj;
    string member;

    DotNode(ASTNode* o, string m) : obj(o), member(m) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "Access ." << member << endl;
        if (obj) obj->print(level + 1);
    }

    Value eval(void* scope) override {
        return Value();
    }
};

class MethodCallNode : public ASTNode {
public:
    ASTNode* obj;
    std::string method;
    std::vector<ASTNode*> args;

    MethodCallNode(ASTNode* o, const std::string& m, std::vector<ASTNode*> a)
        : obj(o), method(m), args(std::move(a)) {}

    void print(int level = 0) override {
        ASTNode::print(level);
        cout << "MethodCall: ." << method << endl;
        if (obj) obj->print(level + 1);
        for (auto a : args) {
            if (a) a->print(level + 1);
        }
    }

    Value eval(void* scope) override {
        return Value();
    }
};

// ==== ALIAS-uri pentru parser (comp.y) ====
typedef ASTNode        ast_node;
typedef LiteralNode    literal_node;
typedef IdNode         id_node;
typedef AssignNode     assign_node;
typedef BinaryExprNode binary_expr_node;
typedef CallNode       call_node;
typedef DotNode        dot_node;
typedef MethodCallNode method_call_node;
typedef MemberAssignNode member_assign_node;

#endif