#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
#include "scanner.h"

using namespace std;

template <class T, class T_EL>
void from_st(T & st, T_EL & i) {
    i = st.top();
    st.pop();
}

class Parser {
private:

    
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    double c_real;
    string c_str;
    Scanner scan;
    stack<int> st_int;
    stack<type_of_lex> st_lex;
    void P();
    void D1();
    void D();
    void D_A();
    void B();
    void S();
    void E_A();
    void E();
    void E1();
    void T();
    void F();
    void dec(type_of_lex type);
    void dec_array(type_of_lex type, int array_size);
    void check_id();
    void check_case();
    void check_op();
    void check_not();
    void eq_type();
    void eq_bool();
    void check_id_in_read();
    void getlex();
    void stack_clean();

public:
    vector<Lex> poliz;
    Parser(const char *program) : scan(program), c_val(0), c_real(0.0) {}
    void analyze();
};
#endif