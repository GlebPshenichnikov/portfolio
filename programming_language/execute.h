#ifndef EXECUTE_H_INCLUDED
#define EXECUTE_H_INCLUDED

#include "parser.h"
#include "scanner.h"
#include <variant>
#include <variant>
#include <cstring>
#include <string>


class Executer {
public:
    void execute(vector<Lex> &poliz);
};

class Interpretator {
    Parser pars;
    Executer E;
public:
    Interpretator(const char *program) : pars(program) {}
    void interpretation();
};



#endif 