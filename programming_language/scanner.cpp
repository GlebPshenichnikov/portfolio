#include "scanner.h"

using namespace std;

vector <Ident> TID;

const char *
        Scanner::TW[] = {"","for", "to", "pop", "and", "begin", "bool", "do", "else", "end", "if", "false", "int", "not", "or", "program",
                         "read", "then", "true", "var", "while", "write", "repeat", "until", "continue", "break",
                         "real", "case", "of", "string", "array", nullptr};

const char *
        Scanner::TD[] = {".", ";", ",", ":", ":=", "(", ")", "=", "<", ">", "+", "-", "*", "%", "/", "<=", "!=", ">=", "[", "]", nullptr};

Scanner::Scanner(const char *program) {
    fd = fopen(program, "r");
    if (fd == nullptr)
        throw "Файл не найден";
}

int put(const string &buf) {
    vector<Ident>::iterator i;

    if ((i = find(TID.begin(), TID.end(), buf)) != TID.end())
        return i - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

int Scanner::look(const string buf, const char ** list) {
    int i = 0;
    while (list[i]) {
        if (buf == list[i])
            return i;
        i++;
    }
    return 0;
}

void Scanner::gc() {
    c = fgetc(fd);
    row++;
    if (c == '\n') {
        line++;
        prev_line_len = row - 1;
        row = 1;
    }
}

Lex Scanner::get_lex() {
    enum state { H, IDENT, NUMB, COM, ALE, NEQ, REAL, STR };
    int d, j, d1, zn =1;
    double w = 1;
    string buf;
    state CS = H;
    do {
        gc();
        switch (CS) {
            case H:
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t');
                else if (c == '-') {
                    zn = -1;
                    buf.push_back(c);
                    gc();
                    if (isdigit(c)) {
                        d = c - '0';
                        CS = NUMB;
                    }
                    else {
                        ungetc(c, fd);
                        if (c == '\n') {
                            line--;
                            row = prev_line_len;
                        } else
                            row--;
                        j = look(buf, TD);
                        return Lex((type_of_lex) (j + (int) LEX_FIN), j, line, row);
                    }
                }
                else if (isalpha(c)) {
                    buf.push_back(c);
                    CS = IDENT;
                } else if (isdigit(c)) {
                    d = c - '0';
                    CS = NUMB;
                } else if (c == '{') {
                    CS = COM;
                } else if (c == ':' || c == '<' || c == '>') {
                    buf.push_back(c);
                    CS = ALE;
                } else if (c == '!') {
                    buf.push_back(c);
                    CS = NEQ;
                } else if (c == '"') {
                    CS = STR;
                } else if (c == '[') {
                    return Lex(LEX_LBRACKET, 0, line, row);
                } else if (c == ']') {
                    return Lex(LEX_RBRACKET,0, line, row);
                } else if (c == '.') {
                    return Lex(LEX_FIN, 0, line, row);
                } else {
                    buf.push_back(c);
                    if ((j = look(buf, TD))) {
                        return Lex((type_of_lex) (j + (int) LEX_FIN), j, line, row);
                    } else
                        throw Except(c, line, row);
                }
                break;

            case IDENT:
                if (isalpha(c) || isdigit(c)) {
                    buf.push_back(c);
                } else {
                    ungetc(c, fd);
                    if (c == '\n') {
                        line--;
                        row = prev_line_len;
                    } else
                        row--;
                    if ((j = look(buf, TW))) {
                        return Lex((type_of_lex) j, j, line, row);
                    } else {
                        j = put(buf);
                        return Lex(LEX_ID, j, line, row);
                    }
                }
                break;

            case NUMB:
                if (isdigit(c)) {
                    d = d * 10 + (c - '0');
                } else if (c == '.') {
                    d1 = 0;
                    CS = REAL;
                } else {
                    ungetc(c, fd);
                    if (c == '\n') {
                        line--;
                        row = prev_line_len;
                    } else
                        row--;
                    return Lex(LEX_NUM, d * zn, line, row);
                }
                break;

            case REAL:
                if (isdigit(c)) {
                    d1 = d1 * 10 + (c - '0');
                    w *= 10;
                } else {
                    ungetc(c, fd);
                    if (c == '\n') {
                        line--;
                        row = prev_line_len;
                    } else
                        row--;
                    double real_val = (double) d + (double) d1 / w;
                    return Lex(LEX_REAL, real_val * zn, line, row);
                }
                break;

            case COM:
                if (c == '}') {
                    CS = H;
                } else if (c == '.' || c == '{')
                    throw Except(c, line, row);
                break;

            case ALE:
                if (c == '=') {
                    buf.push_back(c);
                    j = look(buf, TD);
                    return Lex((type_of_lex) (j + (int) LEX_FIN), j, line, row);
                } else {
                    ungetc(c, fd);
                    if (c == '\n') {
                        line--;
                        row = prev_line_len;
                    } else
                        row--;
                    j = look(buf, TD);
                    return Lex((type_of_lex) (j + (int) LEX_FIN), j, line, row);
                }

            case NEQ:
                if (c == '=') {
                    buf.push_back(c);
                    j = look(buf, TD);
                    return Lex(LEX_NEQ, j, line, row);
                } else
                    throw Except('!', line, row);
                break;

            case STR:
                if (c == '"') {
                    return Lex(LEX_STRING, buf, line, row);
                } else if (c == '\\') {
                    gc();
                    if (c == 'n')
                        buf.push_back('\n');
                    else if (c == 't')
                        buf.push_back('\t');
                    else if (c == '"')
                        buf.push_back('"');
                    else if (c == '\\')
                        buf.push_back('\\');
                    else
                        throw Except("Неверный escape-символ", line, row);
                } else if (c == EOF)
                    throw Except("Незавершенная строковая константа", line, row);
                else
                    buf.push_back(c);
                break;
        }
    } while (1);
}

ostream &operator<<(ostream &s, Lex l) {
    string t;
    if (l.t_lex <= LEX_ARRAY)
        t = Scanner::TW[l.t_lex];
    else if (l.t_lex >= LEX_FIN && l.t_lex <= LEX_RBRACKET)
        t = Scanner::TD[l.t_lex - LEX_FIN];
    else if (l.t_lex == LEX_NUM)
        t = "NUMB";
    else if (l.t_lex == LEX_REAL)
        t = "REAL";
    else if (l.t_lex == LEX_ID)
        t = TID[l.v_lex].get_name();
    else if (l.t_lex == POLIZ_LABEL)
        t = "Label";
    else if (l.t_lex == POLIZ_ADDRESS)
        t = "Addr";
    else if (l.t_lex == POLIZ_GO)
        t = "!";
    else if (l.t_lex == POLIZ_FGO)
        t = "!F";
    else
        throw Except(l);

    if (l.t_lex == LEX_REAL) s << "(" << t << ", " << l.v_real << ");" << endl;
    else if (l.t_lex == LEX_STRING) s << "(" << t << ", " << l.str_lex << ");" << endl;
    else s << "(" << t << ", " << l.v_lex << ");" << endl;
    return s;
}

Except::Except(Lex c) {
    this->l = c;
    this->row = c.get_row();
    this->line = c.get_line();
    is_char = false;
}

Except::Except(char c, int line, int row) {
    this->c = c;
    this->row = row;
    this->line = line;
    is_char = true;
}

Except::Except(const char * err_name, int line, int row) {
    this->err_name = err_name;
    this->row = row;
    this->line = line;
    is_char = false;
}