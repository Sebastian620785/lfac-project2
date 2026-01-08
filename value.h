#ifndef VALUE_H
#define VALUE_H

#include <string>

enum ValueType {
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STRING,
    VAL_VOID
};

class Value {
public:
    ValueType type;

    int i;
    float f;
    bool b;
    std::string s;

    bool hasReturn; 

    Value();
    Value(int v);
    Value(float v);
    Value(bool v);
    Value(const std::string& v);

    std::string toString() const;
};

#endif
