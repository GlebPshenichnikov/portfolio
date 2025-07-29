#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED
#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <vector>
#include <stack>
#include <algorithm>

enum type_of_lex {
    LEX_NULL, 
    LEX_FOR,
    LEX_TO, 
    POLIZ_POP,          //1
    LEX_AND,            //2
    LEX_BEGIN,          //3
    LEX_BOOL,           //4
    LEX_DO,             //5
    LEX_ELSE,           //6
    LEX_END,            //7
    LEX_IF,             //8
    LEX_FALSE,          //9
    LEX_INT,            //10
    LEX_NOT,            //11
    LEX_OR,             //12
    LEX_PROGRAM,        //13
    LEX_READ,           //14
    LEX_THEN,           //15
    LEX_TRUE,           //16
    LEX_VAR,            //17
    LEX_WHILE,          //18
    LEX_WRITE,          //19
    LEX_REPEAT,         //20
    LEX_UNTIL,          //21
    LEX_CONTINUE,       //22
    LEX_BREAK,          //23
    LEX_REAL,           //24
    LEX_CASE,           //25
    LEX_OF,             //26
    LEX_STRING,         //27
    LEX_ARRAY,          //28
    LEX_FIN,            //29
    LEX_SEMICOLON,      //30
    LEX_COMMA,          //31
    LEX_COLON,          //32
    LEX_ASSIGN,         //33
    LEX_LPAREN,         //34
    LEX_RPAREN,         //35
    LEX_EQ,             //36
    LEX_LSS,            //37
    LEX_GTR,            //38
    LEX_PLUS,           //39
    LEX_MINUS,          //40
    LEX_TIMES,          //41
    LEX_PROCENT,        //42
    LEX_SLASH,          //43
    LEX_LEQ,            //44
    LEX_NEQ,            //45
    LEX_GEQ,            //46
    LEX_LBRACKET,       //47
    LEX_RBRACKET,       //48
    LEX_NUM,            //49
    LEX_STR,            //50
    LEX_ID,             //51
    POLIZ_LABEL,        //52
    POLIZ_ADDRESS,      //53
    POLIZ_GO,           //54
    POLIZ_FGO           //55
};

class Value {
public:
    int i;
    double d;
    std::string* s;
    type_of_lex type;
    Value() {i = 0; type = LEX_INT;}
    Value(int it){i = it; type = LEX_INT;}
    Value(double dt){d = dt; type = LEX_REAL;}
    Value(const std::string& sd) {s = new std::string(sd); type = LEX_STRING;}
    Value(type_of_lex t) : type(t) {
        switch (t) {
            case LEX_INT: case LEX_BOOL:
                i = 0;
                break;
            case LEX_REAL:
                d = 0.0;
                break;
            case LEX_STRING:
                s = new std::string();
                break;
            
            default:
                throw "Unknown type";
        }
    }
    Value(const Value& other) {
        type = other.type;
        switch (type) {
            case LEX_INT: case LEX_BOOL:
                i = other.i;
                break;
            case LEX_REAL:
                d = other.d;
                break;
            case LEX_STRING:
                s = new std::string(*(other.s));
                break;
            default:
                throw "Unknown type";
        }
    }
    ~Value() {
        if (type == LEX_STRING)
            delete s;
    }

    type_of_lex get_type() const { return type; }


};

using namespace std;




class Lex {
    type_of_lex t_lex;
    double v_real; 
    int v_lex;
    string str_lex;
    int line;
    int row;

public:
    Lex(type_of_lex t = LEX_NULL, int v = 0, int l = 0, int r = 0) : t_lex(t), v_lex(v), line(l), row(r) {}
    Lex(type_of_lex t, double v, int l = 0, int r = 0) : t_lex(t), v_real(v), line(l), row(r) {}
    Lex(type_of_lex t, string v, int l = 0, int r = 0) : t_lex(t), str_lex(v), line(l), row(r) {}

    int get_line() const { return line; }
    int get_row() const { return row; }
    type_of_lex get_type() const { return t_lex; }
    int get_value() const { return v_lex; }
    double get_real() const { return v_real; }
    string get_str() const { return str_lex; }

    friend ostream &operator<<(ostream &s, Lex l);
};

class Ident {
    string name;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
    double real_value;
    string str_value;
    int array_size;
private:
    type_of_lex array_type;
    std::vector<Value> array;

public:
    Ident() {
        declare = false;
        assign = false;
    }

    bool operator==(const string &s) const {
        return name == s;
    }

    Ident(const string n) {
        name = n;
        declare = false;
        assign = false;
    }

    string get_name() const { return name; }
    bool get_declare() const { return declare; }
    void put_declare() { declare = true; }
    type_of_lex get_type() const { return type; }
    void put_type(type_of_lex t) { type = t; }
    bool get_assign() const { return assign; }
    void put_assign() { assign = true; }
    int get_value() const { return value; }
    void put_value(int v) { value = v; }
    double get_real_value() const { return real_value; }
    void put_real_value(double v) { real_value = v; }
    string get_str_value() const { return str_value; }
    void put_str_value(string v) { str_value = v; }
    int get_array_size() const { return array_size; }
    void put_array_size(int s) { array_size = s; }
    void put_array_type(type_of_lex t) { array_type = t; }
    type_of_lex get_array_type() { return array_type; }


    void set_array_element(int index, const Value& value) {
        if (index >= 0 && index < array.size()) {
            array[index] = value;
        } else {
            throw "Array index out of bounds";
        }
    }

    Value get_array_element(int index) const {
        if (index >= 0 && index < array.size()) {
            return array[index];
        } else {
            throw "Array index out of bounds";
        }
    }
    
    void init_array(int size) {
    array.resize(size);
}
};

class Scanner {
    FILE *fd;
    char c;
    int row = 1;
    int line = 1;
    int prev_line_len;


    int look(const string buf, const char **list);
    void gc();

public:
    static const char *TW[], *TD[];

    Scanner(const char *program);
    Lex get_lex();
    ~Scanner() { fclose(fd); }
};

class Except {
    Lex l;
    int line;
    int row;
    char c;
    const char *err_name;
    bool is_char;

public:
    Except(Lex c);
    Except(char c, int line, int row);
    Except(const char *err_name, int line, int row);

    Lex get_lex() const { return l; }
    int get_line() const { return line; }
    int get_row() const { return row; }
    int get_is_char() const { return is_char; }
    char get_char() const { return c; }
    const char *get_err_name() const { return err_name; }
};

#endif