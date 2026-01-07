#include "value.h"
#include <sstream>

Value::Value() {
    type = VAL_VOID;
    i = 0;
    f = 0.0f;
    b = false;
    s = "";
    hasReturn = false;
}

Value::Value(int v) {
    type = VAL_INT;
    i = v;
    f = 0.0f;
    b = false;
    s = "";
    hasReturn = false;
}

Value::Value(float v) {
    type = VAL_FLOAT;
    f = v;
    i = 0;
    b = false;
    s = "";
    hasReturn = false;
}

Value::Value(bool v) {
    type = VAL_BOOL;
    b = v;
    i = 0;
    f = 0.0f;
    s = "";
    hasReturn = false;
}

Value::Value(const std::string& v) {
    type = VAL_STRING;
    s = v;
    i = 0;
    f = 0.0f;
    b = false;
    hasReturn = false;
}

std::string Value::toString() const {
    if (type == VAL_INT) return std::to_string(i);

    if (type == VAL_FLOAT) {
        std::ostringstream oss;
        oss << f;
        return oss.str();
    }

    if (type == VAL_BOOL) return b ? "true" : "false";

    if (type == VAL_STRING) return s;

    if (type == VAL_VOID) return "void";

    return "";
}
