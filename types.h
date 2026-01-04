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
    std::string className;

    TypeInfo(TipBaza t) : type(t), className("") {}
    TypeInfo(std::string name) : type(TYPE_CLASS), className(name) {}
    TypeInfo() : type(TYPE_UNKNOWN), className("") {}

    bool operator==(const TypeInfo& other) const {
        if (type != other.type) return false;
        if (type == TYPE_CLASS) return className == other.className;
        return true;
    }
    bool operator!=(const TypeInfo& other) const {
        return !(*this == other);
    }
};

#endif
