//ma asigur ca daca fiserul value.h e inclus de mai multe ori intr un fisier nu am eroare
#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <iostream>

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
  std:: string s;
    Value();                     
    Value(int v);                
    Value(float v);              
    Value(bool v);               
    Value(const std::string& v); 
   std:: string toString() const;//folosesc la fisare(convertesc in string rezultattul) pt ca cout nu stie ce e value
};

#endif
