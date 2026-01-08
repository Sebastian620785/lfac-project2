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
    string name; //id
    TypeInfo type; //tip
    string category; //functie, clasa, varibila 
    string className; //nume clasa 
    string value; //valoare
    int size; int offset;
    vector<SymbolType> paramTypes; //(int, float, ...) 

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
//definierea unui singur scope - if, while
class SymbolTable {
    map<string, SymbolInfo> symbols; //{id, symbolInfo}
    SymbolTable* parent; 
    string scopeName; //"global", "func_main", etc. cand apelez dumpAllScopes, acesta este inclus in tables.txt
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

    //actulizam semnatura functiei, adaugand tipurile parametrilor, pentru ca apoi sa comparam cu variabilele pasate la apel de functie 
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
    
    //ne uitam daca exista variabila
    SymbolInfo* lookup(string name) {
        if (symbols.count(name)) return &symbols[name];
        return parent ? parent->lookup(name) : NULL;
    }

    SymbolInfo* lookupCurrent(string name) {
        return symbols.count(name) ? &symbols[name] : NULL;
    }

    SymbolTable* getParent() { 
        return parent; 
    }
    //inregistram stats despre scope-ul la care ne aflam
    void dump(ofstream& out) {
        if (!out.is_open()) return;

        out << "\n=== Scope: " << scopeName << " ===" << endl;
    
        if (parent != nullptr) 
            out << "Parinte: " << parent->getScopeName() << endl;
        else 
            out << "Parinte: Global" << endl;

        out << "Simboluri:" << endl;

        if (symbols.empty()) {
            out << "  (gol)" << endl;
            return;
        }

        for (auto& entry : symbols) {
            SymbolInfo& s = entry.second; 
            out << "  " << s.name << " : " << s.type.typeToString();
            out << " [" << s.category << "]";

            if (s.category == "variabila" && s.value != "") {
                out << " = " << s.value;
            }
            if (s.category == "functia") {
                out << " (argumente: " << s.paramTypes.size() << ")";
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
    //cauta daca avem o clasa definita cu numele clasei date. daca da, cautam membrul dorit
    SymbolInfo* lookupInClass(string className, string memberName) { 
        return classScopes.count(className) ? classScopes[className]->lookupCurrent(memberName) : NULL; 
    }

    void dumpAllScopes(const string& filename) {
        ofstream out(filename);
        if(!out.is_open()) return;
        for(auto s : allScopes) s->dump(out);
        out.close();
        cout << "[Info] tabel simbol a fost aruncat in " << filename << endl;
    }
};
#endif