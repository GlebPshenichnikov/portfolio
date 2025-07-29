#include "execute.h"
#include <iostream>
#include <string>

extern std::vector<Ident> TID;
extern bool is_case;
extern bool is_new_for;
extern int is_for;

std::stack<Value> args;
void Executer::execute(std::vector<Lex>& poliz) {
    try {
    std::stack<Value> args;
    int ind = 0;
    int size = poliz.size();
    while (ind < size) {
        Lex lex = poliz[ind];

        switch (lex.get_type()) {
            case LEX_TRUE:
            case LEX_FALSE:
            case LEX_NUM:
            case POLIZ_ADDRESS:
            case POLIZ_LABEL:
                args.push(Value(lex.get_value()));
                break;

            case POLIZ_POP:
                is_case = false;
                is_new_for = false;
                is_for = 0;
                while (!args.empty()) args.pop();
                break;

            case LEX_REAL:
                args.push(Value(lex.get_real()));
                break;

            case LEX_STRING:
                args.push(Value(lex.get_str()));
                break;

            case LEX_ID:
                {
                    int index = lex.get_value();

                    if (index < 0 || index >= TID.size())
                        throw "IDInvalid identifier index";
                    Ident ident = TID[index];
                    if (!ident.get_assign())
                        throw "Uninitialized identifier";
                    switch (ident.get_type()) {
                        case LEX_INT:
                            args.push(Value(ident.get_value()));
                            break;
                        case LEX_REAL:
                            args.push(Value(ident.get_real_value()));
                            break;
                        case LEX_STRING:
                            args.push(Value(ident.get_str_value()));
                            break;
                        case LEX_BOOL:
                        {
                            Value temp(ident.get_value());
                            temp.type = LEX_BOOL;
                            args.push(temp);
                        }
                            break;
                        default:
                            throw "Unknown identifier type";
                    }
                }
                break;

            case LEX_NOT:
                {
                    if (args.empty())
                        throw "Stack is empty";
                    Value arg = args.top();
                    args.pop();
                    if (arg.get_type() != LEX_INT)
                        throw "Invalid type for not operator";
                    args.push(Value(!arg.i));
                }
                break;

            case LEX_OR:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for or operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != LEX_INT || arg2.get_type() != LEX_INT)
                        throw "Invalid types for or operator";
                    args.push(Value(arg1.i || arg2.i));
                }
                break;

            case LEX_AND:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for and operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != LEX_INT || arg2.get_type() != LEX_INT)
                        throw "Invalid types for and operator";
                    args.push(Value(arg1.i && arg2.i));
                }
                break;

            case POLIZ_GO:
                {
                    if (args.empty())
                        throw "Stack is empty";
                    Value arg = args.top();
                    args.pop();
                    if (arg.get_type() != LEX_INT)
                        throw "Invalid type for go operator";
                    ind = arg.i - 1;
                    
                }
                break;

            case POLIZ_FGO:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for conditional go operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != LEX_INT || arg2.get_type() != LEX_INT)
                        throw "Invalid types for conditional go operator";
                    if (!arg1.i)
                        ind = arg2.i - 1;
                }
                break;
            

            case LEX_WRITE:
            {
                std::vector<Value> values;
                
                while (!args.empty()) {
                    values.push_back(args.top());
                    args.pop();
                }
                if (is_case == true) values.pop_back();
                if (is_new_for == true)
                {
                    values.pop_back();
                    values.pop_back();
                    is_new_for = false;
                } else if (is_for != 0)
                {
                    is_for--;
                    values.pop_back();
                }
                is_case = false;
                for (int i = values.size() - 1; i >= 0; --i) {
                    Value arg = values[i];

                    switch (arg.get_type()) {
                        case LEX_INT:
                            std::cout << arg.i;
                            break;
                        case LEX_REAL:
                            std::cout << arg.d;
                            break;
                        case LEX_STRING:
                            std::cout << *(arg.s);
                            break;
                        case LEX_BOOL:
                            std::cout << (arg.i ? "true" : "false");
                            break;
                        
                        default:
                            throw "Unknown type for write operator";
                    }

                    if (i > 0) {
                        std::cout << " ";
                    }
                }
                std::cout << std::endl;
            }
            break;

            case LEX_READ:
                {
                    if (args.empty())
                        throw "Stack is empty";
                    Value arg = args.top();
                    args.pop();
                    if (arg.get_type() != LEX_INT)
                        throw "Invalid type for read operator";
                    int index = arg.i;
                    if (index < 0 || index >= TID.size())
                        throw "READ Invalid identifier index";
                    Ident& ident = TID[index];
                    switch (ident.get_type()) {
                        case LEX_INT:
                            {
                                int i;
                                std::cin >> i;
                                ident.put_value(i);
                            }
                            break;
                        case LEX_REAL:
                            {
                                double d;
                                std::cin >> d;
                                ident.put_real_value(d);
                            }
                            break;
                        case LEX_STRING:
                            {
                                std::string s;
                                std::cin >> s;
                                ident.put_str_value(s);
                            }
                            break;
                        default:
                            throw "Unknown type for read operator";
                    }
                    ident.put_assign();
                }
                break;

            case LEX_PLUS:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for plus operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != arg2.get_type())
                        throw "Invalid types for plus operator";
                    switch (arg1.get_type()) {
                        case LEX_INT:
                            args.push(Value(arg1.i + arg2.i));
                            break;
                        case LEX_REAL:
                            args.push(Value(arg1.d + arg2.d));
                            break;
                        case LEX_STRING:
                            {
                                std::string s = *(arg1.s) + *(arg2.s);
                                args.push(Value(s));
                            }
                            break;
                        default:
                            throw "Unknown types for plus operator";
                    }
                }
                break;

            case LEX_MINUS:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for minus operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != arg2.get_type())
                        throw "Invalid types for minus operator";
                    switch (arg1.get_type()) {
                        case LEX_INT:
                            args.push(Value(arg1.i - arg2.i));
                            break;
                        case LEX_REAL:
                            args.push(Value(arg1.d - arg2.d));
                            break;
                        default:
                            throw "Unknown types for minus operator";
                    }
                }
                break;

            case LEX_TIMES:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for times operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != arg2.get_type())
                        throw "Invalid types for times operator";
                    switch (arg1.get_type()) {
                        case LEX_INT:
                            args.push(Value(arg1.i * arg2.i));
                            break;
                        case LEX_REAL:
                            args.push(Value(arg1.d * arg2.d));
                            break;
                        default:
                            throw "Unknown types for times operator";
                    }
                }
                break;

            case LEX_SLASH:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for divide operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != arg2.get_type())
                        throw "Invalid types for divide operator";
                    switch (arg1.get_type()) {
                        case LEX_INT:
                            if (arg2.i == 0)
                                throw "Division by zero";
                            args.push(Value(arg1.i / arg2.i));
                            break;
                        case LEX_REAL:
                            if (arg2.d == 0.0)
                                throw "Division by zero";
                            args.push(Value(arg1.d / arg2.d));
                            break;
                        default:
                            throw "Unknown types for divide operator";
                    }
                }
                break;

            case LEX_PROCENT:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for mod operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != LEX_INT || arg2.get_type() != LEX_INT)
                        throw "Invalid types for mod operator";
                    if (arg2.i == 0)
                        throw "Division by zero";
                    args.push(Value(arg1.i % arg2.i));
                }
                break;

            case LEX_ASSIGN:
            {
                if (args.size() < 2)
                    throw "Not enough arguments for assign operator";
                
                Value arg2 = args.top();
                args.pop();
                Value arg1 = args.top();
                args.pop();
                
                if (arg1.get_type() != LEX_INT)
                    throw "Invalid type for assign operator";
                
                int index = arg1.i;
                if (index < 0 || index >= TID.size())
                    throw "ASSIGN: Invalid identifier index";
                
                Ident& ident = TID[index];
                
                if (ident.get_type() == LEX_ARRAY) {
                    if (args.empty())
                        throw "Not enough arguments for array element assignment";
                    
                    Value array_index = args.top();
                    args.pop();
                    
                    if (array_index.get_type() != LEX_INT)
                        throw "Invalid type for array index";
                    
                    int elem_index = array_index.i;
                    if (elem_index < 0 || elem_index >= ident.get_array_size())
                        throw "ASSIGN: Array index out of bounds";
                    
                    switch (ident.get_array_type()) {
                        case LEX_INT:
                            if (arg2.get_type() != LEX_INT)
                                throw "ASSIGN: Invalid type for array element assignment";
                            ident.set_array_element(elem_index, arg2.i);
                            break;
                        case LEX_REAL:
                            if (arg2.get_type() != LEX_REAL)
                                throw "ASSIGN: Invalid type for array element assignment";
                            ident.set_array_element(elem_index, arg2.d);
                            break;
                        case LEX_STRING:
                            if (arg2.get_type() != LEX_STRING)
                                throw "ASSIGN: Invalid type for array element assignment";
                            ident.set_array_element(elem_index, *(arg2.s));
                            break;
                        default:
                            throw "ASSIGN: Unknown array type";
                    }
                } else {
                    switch (ident.get_type()) {
                        case LEX_INT:
                            if (arg2.get_type() != LEX_INT)
                                throw "ASSIGN: Invalid type for variable assignment";
                            ident.put_value(arg2.i);
                            break;
                        case LEX_REAL:
                            if (arg2.get_type() != LEX_REAL)
                                throw "ASSIGN: Invalid type for variable assignment";
                            ident.put_real_value(arg2.d);
                            break;
                        case LEX_STRING:
                            if (arg2.get_type() != LEX_STRING)
                                throw "ASSIGN: Invalid type for variable assignment";
                            ident.put_str_value(*(arg2.s));
                            break;
                        case LEX_BOOL:
                            if (arg2.get_type() != LEX_INT)
                                throw "ASSIGN: Invalid type for variable assignment";
                            ident.put_value(arg2.i);
                            break;
                        default:
                            throw "ASSIGN: Unknown variable type";
                    }
                }
                
                ident.put_assign();
            }
            break;

            case LEX_EQ:
            case LEX_LSS:
            case LEX_GTR:
            case LEX_LEQ:
            case LEX_GEQ:
            case LEX_NEQ:
                {
                    if (args.size() < 2)
                        throw "Not enough arguments for comparison operator";
                    Value arg2 = args.top();
                    args.pop();
                    Value arg1 = args.top();
                    args.pop();
                    if (arg1.get_type() != arg2.get_type())
                        throw "Invalid types for comparison operator";
                    bool result;
                    switch (arg1.get_type()) {
                        case LEX_INT:
                            switch (lex.get_type()) {
                                case LEX_EQ:
                                    result = arg1.i == arg2.i;
                                    break;
                                case LEX_NEQ:
                                    result = arg1.i != arg2.i;
                                    break;
                                case LEX_LSS:
                                    result = arg1.i < arg2.i;
                                    break;
                                case LEX_LEQ:
                                    result = arg1.i <= arg2.i;
                                    break;
                                case LEX_GTR:
                                    result = arg1.i > arg2.i;
                                    break;
                                case LEX_GEQ:
                                    result = arg1.i >= arg2.i;
                                    break;
                                default:
                                    throw "Unknown comparison operator";
                            }
                            break;
                        case LEX_REAL:
                            switch (lex.get_type()) {
                                case LEX_EQ:
                                    result = arg1.d == arg2.d;
                                    break;
                                case LEX_NEQ:
                                    result = arg1.d != arg2.d;
                                    break;
                                case LEX_LSS:
                                    result = arg1.d < arg2.d;
                                    break;
                                case LEX_LEQ:
                                    result = arg1.d <= arg2.d;
                                    break;
                                case LEX_GTR:
                                    result = arg1.d > arg2.d;
                                    break;
                                case LEX_GEQ:
                                    result = arg1.d >= arg2.d;
                                    break;
                                default:
                                    throw "Unknown comparison operator";
                            }
                            break;
                        case LEX_STRING:switch (lex.get_type()) {
                               case LEX_EQ:
                                   result = *(arg1.s) == *(arg2.s);
                                   break;
                               case LEX_NEQ:
                                   result = *(arg1.s) != *(arg2.s);
                                   break;
                               case LEX_LSS:
                                   result = *(arg1.s) < *(arg2.s);
                                   break;
                               case LEX_LEQ:
                                   result = *(arg1.s) <= *(arg2.s);
                                   break;
                               case LEX_GTR:
                                   result = *(arg1.s) > *(arg2.s);
                                   break;
                               case LEX_GEQ:
                                   result = *(arg1.s) >= *(arg2.s);
                                   break;
                               default:
                                   throw "Unknown comparison operator";
                           }
                           break;
                       default:
                           throw "Unknown types for comparison operator";
                   }
                   args.push(Value(result));
               }
               break;
            case LEX_ARRAY:
            {
                int i = lex.get_value();
                int j = args.top().i;
                args.pop();

                if (j >= 0 && j < TID[i].get_array_size()) {

                    args.push(TID[i].get_array_element(j));
                } else {
                    throw "POLIZ: Array index out of bounds";
                }
            }
            break;
            case LEX_FIN:
                return;
            default:

                throw "Unknown element";
       }

    ++ind;}
    }
     catch (const char* e) {
        std::cout << "Error: " << e << std::endl;
    } catch (...) {
        std::cout << "Unknown exception occurred." << std::endl;
    }

}

void Interpretator::interpretation() {
        pars.analyze();
        E.execute(pars.poliz);
}