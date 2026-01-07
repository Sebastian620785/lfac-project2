#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include "types.h"

using namespace std;

enum SymbolType { SYM_INT, SYM_FLOAT, SYM_STRING, SYM_BOOL, SYM_CHAR, SYM_VOID, SYM_CLASS, SYM_UNKNOWN };

static string typeToString(SymbolType t) {
    switch(t) {
        case SYM_INT: return "int";
        case SYM_FLOAT: return "float";
        case SYM_STRING: return "string";
        case SYM_BOOL: return "bool";
        case SYM_VOID: return "void";
        case SYM_CLASS: return "class";
        default: return "unknown";
    }
}

static int getTypeSize(SymbolType t) {
    switch(t) {
        case SYM_INT: return 4;
        case SYM_FLOAT: return 8;
        case SYM_BOOL: return 1;
        case SYM_STRING: return 256; 
        default: return 0;
    }
}

class SymbolInfo {
public:
    string name;
    TypeInfo type; 
    string category; 
    string className;
    string value; 
    int size; int offset;
    vector<SymbolType> paramTypes; 

    SymbolInfo() : size(0), offset(0) {}
    SymbolInfo(string n, TypeInfo t, string cat) : name(n), type(t), category(cat) {
        SymbolType st = SYM_UNKNOWN;
        if(t.type == TYPE_INT) st = SYM_INT;
        else if(t.type == TYPE_FLOAT) st = SYM_FLOAT;
        else if(t.type == TYPE_BOOL) st = SYM_BOOL;
        else if(t.type == TYPE_STRING) st = SYM_STRING;
        else if(t.type == TYPE_CLASS) st = SYM_CLASS;
        size = getTypeSize(st);
        offset = 0;
    }
};

class SymbolTable {
    map<string, SymbolInfo> symbols;
    SymbolTable* parent;
    string scopeName;
    int currentMemoryOffset; 
public:

    SymbolTable(SymbolTable* p, string name) : parent(p), scopeName(name), currentMemoryOffset(0) {}

    bool addSymbol(SymbolInfo sym) {
        if (symbols.count(sym.name)) return false;
        sym.offset = currentMemoryOffset;
        currentMemoryOffset += sym.size;
        symbols[sym.name] = sym;
        return true;
    }

    bool updateFunctionParams(string name, vector<TypeInfo> params) {
        if (symbols.count(name)) {
            for(auto p : params) {
                if(p.type == TYPE_INT) symbols[name].paramTypes.push_back(SYM_INT);
                else if(p.type == TYPE_FLOAT) symbols[name].paramTypes.push_back(SYM_FLOAT);
                else if(p.type == TYPE_BOOL) symbols[name].paramTypes.push_back(SYM_BOOL);
                else if(p.type == TYPE_STRING) symbols[name].paramTypes.push_back(SYM_STRING);
            }
            return true;
        }
        return false;
    }

    string getScopeName() { return scopeName; }
    
    SymbolInfo* lookup(string name) {
        if (symbols.count(name)) return &symbols[name];
        return parent ? parent->lookup(name) : NULL;
    }

    SymbolInfo* lookupCurrent(string name) {
        return symbols.count(name) ? &symbols[name] : NULL;
    }

    SymbolTable* getParent() { return parent; }
    
    void dump(ofstream& out) {
        if (!out.is_open()) return;

        out << endl << "===== SCOPE: " << scopeName << " =====" << endl;
        if (parent) out << "Parent: " << parent->getScopeName() << endl;
        else out << "Parent: none" << endl;
        
        out << "Symbols:" << endl;
        if (symbols.empty()) out << "  (none)" << endl;

        for (auto const& [key, val] : symbols) {
            out << "  " << val.name << " : " << val.type.typeToString();
            out << " (" << val.category << ")";
            
            if (val.category == "variable" && !val.value.empty() && val.value != "?") {
                out << " [Val: " << val.value << "]";
            }
            if (val.category == "function") {
                out << " [Params: " << val.paramTypes.size() << "]";
            }
            out << endl;
        }
    }
};

class ScopeManager {
public:
    SymbolTable *currentScope, *globalScope;

    map<string, SymbolTable*> classScopes;

    vector<SymbolTable*> allScopes;
    

    ScopeManager() { 
        globalScope = new SymbolTable(NULL, "Global"); 
        currentScope = globalScope;
        allScopes.push_back(globalScope);
    }

    ~ScopeManager() { 
        for(auto s : allScopes) delete s;
    }
    
    void enterScope(string name) { 
        currentScope = new SymbolTable(currentScope, name); 
        allScopes.push_back(currentScope);
    }

    void exitScope() { 
        if(currentScope->getParent()) currentScope = currentScope->getParent(); 
    }

    void saveClassScope(string className) { 
        classScopes[className] = currentScope; 
    }

    SymbolInfo* lookupInClass(string className, string memberName) { 
        return classScopes.count(className) ? classScopes[className]->lookupCurrent(memberName) : NULL; 
    }

    void dumpAllScopes(const string& filename) {
        ofstream out(filename);
        if(!out.is_open()) return;
        for(auto s : allScopes) s->dump(out);
        out.close();
        cout << "[Info] Symbol tables dumped to " << filename << endl;
    }
};
#endif