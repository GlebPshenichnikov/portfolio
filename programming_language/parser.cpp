#include "parser.h"
#include "execute.h"
using namespace std;
bool is_case = false;
bool is_new_for = false;
int is_for = 0;

extern vector<Ident> TID;
extern stack<Value> args;

void Parser::getlex() {
    curr_lex = scan.get_lex();
    std::cout << "Lexeme: " << curr_lex << std::endl;

    c_type = curr_lex.get_type();

    switch (c_type) {
        case LEX_NUM:
        case LEX_ID:
            c_val = curr_lex.get_value();
            break;
        case LEX_REAL:
            c_real = curr_lex.get_real();
            break;
        case LEX_STRING:
            c_str = curr_lex.get_str();
            break;
        default:
            break;
    }

}

void Parser::analyze() {
    getlex();
    P();
    if (c_type != LEX_FIN)
        throw Except(curr_lex);
    for (Lex l : poliz){
        cout << l;
    }
}


void Parser::P() {
    if (c_type == LEX_PROGRAM) {
        getlex();
    } else
        throw Except(curr_lex);
    D1();
    B();
    if (c_type != LEX_FIN)
        throw Except(curr_lex);
}

void Parser::D1() {
    if (c_type == LEX_VAR) {
        getlex();
        D();
        while (c_type == LEX_SEMICOLON) {
            getlex();
            if (c_type == LEX_BEGIN)
                continue;
            D();
        }
    } else
        throw Except(curr_lex);
}

void Parser::D() {
    if (c_type != LEX_ID)
        throw Except(curr_lex);
    else {
        st_int.push(c_val);
        getlex();
        while (c_type == LEX_COMMA) {
            getlex();
            if (c_type != LEX_ID)
                throw Except(curr_lex);
            else {
                st_int.push(c_val);
                getlex();
            }
        }
        if (c_type != LEX_COLON)
            throw Except(curr_lex);
        else {
            getlex();
            if (c_type == LEX_INT)
                dec(LEX_INT);
            else if (c_type == LEX_REAL)
                dec(LEX_REAL);
            else if (c_type == LEX_BOOL)
                dec(LEX_BOOL);
            else if (c_type == LEX_STRING)
                dec(LEX_STRING);
            else if (c_type == LEX_ARRAY)
                D_A();
            else
                throw Except(curr_lex);
        }
    }
}

void Parser::D_A() {
    getlex();
    if (c_type != LEX_LBRACKET)
        throw Except(curr_lex);
    getlex();
    if (c_type != LEX_NUM)
        throw Except(curr_lex);
    int array_size = c_val;
    getlex();
    if (c_type != LEX_RBRACKET)
        throw Except(curr_lex);
    getlex();
    if (c_type != LEX_OF)
        throw Except(curr_lex);
    getlex();
    if (c_type == LEX_INT)
        dec_array(LEX_INT, array_size);
    else if (c_type == LEX_REAL)
        dec_array(LEX_REAL, array_size);
    else if (c_type == LEX_BOOL)
        dec_array(LEX_BOOL, array_size);
    else if (c_type == LEX_STRING)
        dec_array(LEX_STRING, array_size);
    else
        throw Except(curr_lex);
}

void Parser::dec(type_of_lex type) {
    int i;
    while (!st_int.empty()) {
        i = st_int.top();
        st_int.pop();
        if (TID[i].get_declare())
            throw Except("Переменная объявлена дважды", curr_lex.get_line(), curr_lex.get_row());
        else {
            TID[i].put_declare();
            TID[i].put_type(type);
        }
    }
    getlex();
}

void Parser::dec_array(type_of_lex type, int array_size) {
    int i;
    while (!st_int.empty()) {
        i = st_int.top();
        st_int.pop();
        if (TID[i].get_declare())
            throw Except("Переменная объявлена дважды", curr_lex.get_line(), curr_lex.get_row());
        else {
            TID[i].put_declare();
            TID[i].put_type(LEX_ARRAY);
            TID[i].put_array_size(array_size);
            TID[i].put_array_type(type);
            TID[i].init_array(array_size);
        }
    }
    getlex();
}

void Parser::B() {
    if (c_type == LEX_BEGIN) {
        getlex();
        S();
        while (c_type == LEX_SEMICOLON) {
            getlex();
            if (c_type == LEX_END) {
                getlex();
                return;
            }
            S();
        }
        if (c_type == LEX_END) {
            getlex();
        } else {
            throw Except(curr_lex);
        }
    } else
        throw Except(curr_lex);
}

void Parser::S() {
    int pl0, pl1, pl2, pl3;
    if (c_type == LEX_IF) {
        getlex();
        E();
        eq_bool();
        pl2 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (c_type == LEX_THEN) {
            getlex();
            S();
            pl3 = poliz.size();
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl2] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
            if (c_type == LEX_ELSE) {
                getlex();
                S();
                poliz[pl3] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
            }else {
                poliz[pl3] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
            } 
        } else
            throw Except(curr_lex);
    } else if (c_type == LEX_WHILE) {
        pl0 = poliz.size();
        getlex();
        E();
        eq_bool();
        pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (c_type == LEX_DO) {
            getlex();
            S();
            poliz.push_back(Lex(POLIZ_LABEL, pl0));
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl1] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
        } else
            throw Except(curr_lex);
    } else if (c_type == LEX_READ) {
        getlex();
        if (c_type == LEX_LPAREN) {
            getlex();
            if (c_type == LEX_ID) {
                check_id_in_read();
                poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
                getlex();
            } else
                throw Except(curr_lex);
            if (c_type == LEX_RPAREN) {
                getlex();
                poliz.push_back(Lex(LEX_READ));
            } else
                throw Except(curr_lex);
        } else
            throw Except(curr_lex);
    } else if (c_type == LEX_WRITE) {
        getlex();
        if (c_type == LEX_LPAREN) {
            getlex();
            E();
            while (c_type == LEX_COMMA) {
                getlex();
                E();
            }
            if (c_type == LEX_RPAREN) {
                getlex();
                poliz.push_back(Lex(LEX_WRITE));
            } else
                throw Except(curr_lex);
        } else
            throw Except(curr_lex);
    }

   else if (c_type == LEX_CASE) {
    is_case = true;
    getlex();
    E(); 
    int x = poliz.size()-1; 
    if (c_type == LEX_OF) {
        getlex();
        std::vector<int> case_values; 
        std::vector<int> case_labels;
        std::vector<int> case_ends;
        std::vector<int> case_starts;
        while (c_type != LEX_END) {
            if (c_type == LEX_NUM) {
                case_starts.push_back(poliz.size());
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO));

                case_values.push_back(c_val);
                case_labels.push_back(poliz.size());
                getlex();
                while (c_type == LEX_COMMA) {
                    getlex();
                    if (c_type != LEX_NUM) throw Except(curr_lex);
                    case_values.push_back(c_val);
                    case_labels.push_back(poliz.size());
                    getlex();
                }

                if (c_type == LEX_COLON) {
                    getlex();
                    S();
                    case_ends.push_back(poliz.size());
                    poliz.push_back(Lex());
                    poliz.push_back(Lex(POLIZ_GO)); 
                } else {
                    throw Except(curr_lex);
                }

                if (c_type == LEX_SEMICOLON) getlex();
            } else if (c_type == LEX_ELSE) {
                case_starts.push_back(poliz.size());
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO));

                case_labels.push_back(poliz.size());
                getlex();
                S();
                case_ends.push_back(poliz.size());
                poliz.push_back(Lex());
                poliz.push_back(Lex(POLIZ_GO)); 
                if (c_type == LEX_SEMICOLON) getlex();
            } else {
                throw Except(curr_lex);
            }

            if (c_type == LEX_END) {
                getlex();
         
                break;
            }

        }
        std::sort(case_values.begin(), case_values.end());
        
        for (int i = 0 ; i < case_values.size() - 1; i++) {
            if (case_values[i] == case_values[i+1]) throw Except("Дублирующиеся значения в case", curr_lex.get_line(), curr_lex.get_row());
        }

        for (int i = 0; i < case_starts.size(); i++)
        {
            poliz[case_starts[i]] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
        }
        int i;
        for (i = 0; i < case_values.size(); i++)
        {
            poliz.push_back(Lex(LEX_ID, poliz[x].get_value()));
            // std::cout << poliz[x];
            poliz.push_back(Lex(LEX_NUM, case_values[i]));
            poliz.push_back(Lex(LEX_NEQ));
            poliz.push_back(Lex(POLIZ_LABEL, case_labels[i]));
            poliz.push_back(Lex(POLIZ_FGO)); 
        }
        poliz.push_back(Lex(POLIZ_LABEL, case_labels[case_labels.size()-1]));
        poliz.push_back(Lex(POLIZ_GO)); 
        for (int i = 0; i < case_ends.size(); i++)
        {
            poliz[case_ends[i]] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size()));
        }

           

    } else {
        throw Except(curr_lex);
    }
}

else if (c_type == LEX_REPEAT) {
    
    pl0 = poliz.size(); 

    getlex();
    while (c_type != LEX_UNTIL) {
        S();
        if (c_type == LEX_SEMICOLON) {
            getlex();
        } else if (c_type != LEX_UNTIL) {
            throw Except(curr_lex);
        }
    }
    if (c_type != LEX_UNTIL) {
        throw Except(curr_lex);
    }
    getlex();
    pl1 = poliz.size(); 
    E();  
    eq_bool(); 

    poliz.push_back(Lex(POLIZ_LABEL, static_cast<int>(pl0))); 
    poliz.push_back(Lex(POLIZ_FGO));
    
    
}
else if (c_type == LEX_FOR) {
        getlex();
        is_new_for = true;
        if (c_type != LEX_ID) throw Except(curr_lex);
        int i = c_val; 
        getlex();
        if (c_type != LEX_ASSIGN) throw Except(curr_lex);
        getlex();
        poliz.push_back(Lex(POLIZ_ADDRESS, i)); 
        E();
        is_for = -poliz[poliz.size() - 1].get_value();
        poliz.push_back(Lex(LEX_ASSIGN));
        if (c_type != LEX_TO) throw Except(curr_lex);
        getlex();
        E();
        eq_type();
        Lex end_value = poliz[poliz.size() - 1];
        is_for += end_value.get_value() + 1;
        int pl0 = poliz.size();
        poliz.push_back(Lex(POLIZ_LABEL, 0));
        poliz.push_back(Lex(LEX_ID, i));
        poliz.push_back(end_value); 
        poliz.push_back(Lex(LEX_LEQ)); 
        int pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO, 0)); 
        if (c_type == LEX_DO) {
            getlex();
            
            S();
            
            poliz.push_back(Lex(POLIZ_ADDRESS, i)); 
            poliz.push_back(Lex(LEX_ID, i)); 
            poliz.push_back(Lex(LEX_NUM, 1)); 
            poliz.push_back(Lex(LEX_PLUS)); 
            poliz.push_back(Lex(LEX_ASSIGN)); 
            poliz.push_back(Lex(POLIZ_LABEL, pl0)); 
            poliz.push_back(Lex(POLIZ_GO));
            
            poliz[pl1] = Lex(POLIZ_LABEL, static_cast<int>(poliz.size())); 
            poliz.push_back(Lex(POLIZ_POP));
        } else throw Except(curr_lex);
    }

    else if (c_type == LEX_ID) {
        check_id();
        if (TID[c_val].get_type() == LEX_ARRAY) {
            int array_index = c_val;
            getlex();
            if (c_type == LEX_LBRACKET) {
                getlex();
                E();
                if (c_type == LEX_RBRACKET) {
                    getlex();
                    poliz.push_back(Lex(POLIZ_ADDRESS, array_index));
                    if (c_type == LEX_ASSIGN) {
                        getlex();
                        E();
                        eq_type();
                        poliz.push_back(Lex(LEX_ASSIGN));
                    }
                } else {
                    throw Except(curr_lex);
                }
            } else {
                throw Except(curr_lex);
            }
        } else {
            poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
            getlex();
            if (c_type == LEX_ASSIGN) {
                getlex();
                E();
                eq_type();
                poliz.push_back(Lex(LEX_ASSIGN));
            }
            else {
                poliz.pop_back();
                poliz.push_back(Lex(LEX_ID, c_val));
            }
        }
    }
    else
        B();
}



void Parser::E_A() {
    int i = c_val;
    getlex();
 
    if (c_type != LEX_LBRACKET)
        throw Except(curr_lex);

    getlex();
    E();
    if (c_type != LEX_RBRACKET)
        throw Except(curr_lex);
    getlex();
    poliz.push_back(Lex(LEX_ARRAY, i));
}

void Parser::E() {
    E1();
    if (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR ||
        c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ) {
        st_lex.push(c_type);
        getlex();
        E1();
        check_op();
    }
}

void Parser::E1() {
    T();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR) {
        st_lex.push(c_type);
        getlex();
        T();
        check_op();
    }
}

void Parser::T() {
    F();
    while (c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_AND || c_type == LEX_PROCENT) {
        st_lex.push(c_type);
        getlex();
        F();
        check_op();
    }
}

void Parser::F() {
    if (c_type == LEX_ID) {
        check_id();
        if (TID[c_val].get_type() == LEX_ARRAY) {
            E_A();
        } else {
            poliz.push_back(Lex(LEX_ID, c_val));
            getlex();
        }
    } else if (c_type == LEX_NUM) {
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        getlex();
    } else if (c_type == LEX_REAL) {
        st_lex.push(LEX_REAL);
        poliz.push_back(Lex(LEX_REAL, c_real));
        getlex();
    } else if (c_type == LEX_TRUE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_TRUE, 1));
        getlex();
    } else if (c_type == LEX_FALSE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_FALSE, 0));
        getlex();
    } else if (c_type == LEX_STRING) {
        st_lex.push(LEX_STRING);
        poliz.push_back(Lex(LEX_STRING, c_str));
        getlex();
    } else if (c_type == LEX_NOT) {
        getlex();
        F();
        check_not();
    } else if (c_type == LEX_LPAREN) {
        getlex();
        E();
        if (c_type == LEX_RPAREN)
            getlex();
        else
            throw Except(curr_lex);
    } else
        throw Except(curr_lex);
}

void Parser::check_id() {
    if (TID[c_val].get_declare())
        st_lex.push(TID[c_val].get_type());
    else
        throw Except("Переменная не объявлена", curr_lex.get_line(), curr_lex.get_row());
}

void Parser::check_case() {
    type_of_lex t1, t2;
    t2 = st_lex.top();
    st_lex.pop();
    while (!st_lex.empty()) {
        t1 = st_lex.top();
        st_lex.pop();
        if (t1 != t2)
            throw Except("Несовместимые типы case", curr_lex.get_line(), curr_lex.get_row());
    }
}

void Parser::check_op() {
    type_of_lex t1, t2, op, r;
    t2 = st_lex.top();
    st_lex.pop();
    op = st_lex.top();
    st_lex.pop();
    t1 = st_lex.top();
    st_lex.pop();

    if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH || op == LEX_PROCENT) {
        if (t1 == LEX_INT && t2 == LEX_INT) {
            r = LEX_INT;
        } else if ((t1 == LEX_INT || t1 == LEX_REAL) && (t2 == LEX_INT || t2 == LEX_REAL)) {
            r = LEX_REAL;
        } else if (t1 == LEX_STRING && t2 == LEX_STRING && op == LEX_PLUS) {
            r = LEX_STRING;
        } else {
            throw Except("Невозможная операция", curr_lex.get_line(), curr_lex.get_row());
        }
    } else if (op == LEX_OR || op == LEX_AND) {
        if (t1 == LEX_BOOL && t2 == LEX_BOOL) {
            r = LEX_BOOL;
        } else {
            throw Except("Невозможная операция", curr_lex.get_line(), curr_lex.get_row());
        }
    } else if (op == LEX_EQ || op == LEX_LSS || op == LEX_GTR || op == LEX_LEQ || op == LEX_GEQ || op == LEX_NEQ) {
        if ((t1 == LEX_INT || t1 == LEX_REAL) && (t2 == LEX_INT || t2 == LEX_REAL)) {
            r = LEX_BOOL;
        } else if (t1 == LEX_STRING && t2 == LEX_STRING) {
            r = LEX_BOOL;
        } else if (t1 == LEX_BOOL && t2 == LEX_BOOL) {
            r = LEX_BOOL;
        } else {
            throw Except("Невозможная операция", curr_lex.get_line(), curr_lex.get_row());
        }
    } else {
        throw Except("Невозможная операция", curr_lex.get_line(), curr_lex.get_row());
    }

    st_lex.push(r);
    poliz.push_back(Lex(op));
}

void Parser::check_not() {
    if (st_lex.top() != LEX_BOOL)
    throw Except("Ожидается тип bool", curr_lex.get_line(), curr_lex.get_row());
    else
        poliz.push_back(Lex(LEX_NOT));
}

void Parser::eq_type() {
    type_of_lex t1, t2;
    t1 = st_lex.top();
    st_lex.pop();
    t2 = st_lex.top();
    st_lex.pop();
    if ((t1 == LEX_INT || t1 == LEX_REAL) && (t2 == LEX_INT || t2 == LEX_REAL ) || (t1 == t2)){
        return;
    }
    if (t1 != t2) {
        throw Except("Несоответствие типов", curr_lex.get_line(), curr_lex.get_row());
    }
}


void Parser::eq_bool() {
    if (st_lex.top() != LEX_BOOL)
    throw Except("Ожидается тип bool", curr_lex.get_line(), curr_lex.get_row());
    st_lex.pop();
}

void Parser::check_id_in_read() {
    if (!TID[c_val].get_declare())
    throw Except("Необъявленная переменная", curr_lex.get_line(), curr_lex.get_row());
}

void Parser::stack_clean() {
    while (!st_lex.empty())
        st_lex.pop();
    while (!st_int.empty())
        st_lex.pop();
};

