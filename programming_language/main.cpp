#include "scanner.h"
#include "parser.h"
#include "execute.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    try {
        Interpretator I ( argv[1] );
        I.interpretation ();
        return 0;
    }
    catch (Except exception) {
        if (exception.get_is_char()) {
            cout << argv[1] << ":" << exception.get_line() << ":" << exception.get_row();
            cout << ": неожиданный символ " << exception.get_char() << endl;
        } else if (exception.get_err_name() == nullptr) {
            cout << argv[1] << ":" << exception.get_line() << ":" << exception.get_row();
            cout << ": неожиданная лексема " << exception.get_lex() << endl;
        } else {
            cout << argv[1] << ":" << exception.get_line() << ":" << exception.get_row();
            cout << ": " << exception.get_err_name() << endl;
        }
    }
}
