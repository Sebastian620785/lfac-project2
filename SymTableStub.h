#ifndef SYMTABLESTUB_H
#define SYMTABLESTUB_H

#include <map>
#include <string>
#include <iostream>
#include "value.h"

class SymTableStub {
public:
    std::map<std::string, Value> vals;

    bool hasValue(const std::string& name) const {
        return vals.find(name) != vals.end();
    }

    Value getValue(const std::string& name) const {
        auto it = vals.find(name);
        if (it == vals.end()) {
            std::cerr << "Runtime error: variable '" << name << "' has no value\n";
            return Value(); // void
        }
        return it->second;
    }

    void setValue(const std::string& name, const Value& v) {
        vals[name] = v;
    }
};

#endif
