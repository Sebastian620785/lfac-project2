#ifndef TYPES_H
#define TYPES_H

#include <string>

enum TipBaza {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_CLASS,    
    TYPE_UNKNOWN
};


struct TypeInfo {
    TipBaza type;
    std::string className; //folosit cand declaram o clasa

    TypeInfo(TipBaza t) : type(t), className("") {}
    TypeInfo(std::string name) : type(TYPE_CLASS), className(name) {}
    TypeInfo() : type(TYPE_UNKNOWN), className("") {}
};

#endif
