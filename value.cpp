#include "value.h"
#include <sstream>//convertesc float in string

Value::Value() {
    type = VAL_VOID;
    i = 0;
    f = 0.0f;
    b = false;
    s = "";
}

Value::Value(int v) {
    type = VAL_INT;
    i = v;
    f = 0.0f;
    b = false;
    s = "";
}

Value::Value(float v) {
    type = VAL_FLOAT;
    f = v;
    i = 0;
    b = false;
    s = "";
}

Value::Value(bool v) {
    type = VAL_BOOL;
    b = v;
    i = 0;
    f = 0.0f;
    s = "";
}

Value::Value(const std::string& v) {
    type = VAL_STRING;
    s = v;
    i = 0;
    f = 0.0f;
    b = false;
}



std::string Value::toString() const{
    if(type==VAL_INT)
     return std::to_string(i);
    else 
    if(type==VAL_FLOAT) {
        std::ostringstream oss;//ca un cout dar in loc sa scrie pe ecran scrie intr un string intern
        oss << f;//ia float si l scrie in string(oss)
        return oss.str();
    }
    else 
    if(type==VAL_BOOL) {
    if(b) 
    return "true";
    else 
    return "false";
    }
    else 
    if(type==VAL_STRING) 
    return s;
    else
     if(type==VAL_VOID) 
     return "void";
     return "";//pt a evita situatiua in care e de oricea lt tip
}