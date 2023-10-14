#ifndef LE_CALCULATOR_CALCULATORMAIN_H
#define LE_CALCULATOR_CALCULATORMAIN_H

#include <utility>

#include "std_lib_facilities.h"

// Оъект [тип, имя, значение] -> наименьшая смысловая еденица информации (ну считай что будто буквы)
struct Token {
    char kind;
    double value;
    string name;

    Token(char ch) : kind{ch}, value{0} {}
    Token(char ch, string s) : kind{ch}, name{s}, value{0} {}
    Token(char ch, double val) : kind{ch}, value{val} {}
};

// поток токенов, ...
class Token_stream {
    bool full{false};  // обозначает переполнение потока????
    Token buffer{'\0'}; // текущий токен в потоке, \0 значит eof

public:
    Token_stream() {}

    Token get();

    void putback(Token t);

    void ignore(char);
};

// впихиваем токен в буфер
void Token_stream::putback(Token t) {
    if (full)
        error("putback() into a full buffer");

    buffer = t;
    full = true; // пиздатый поток из 1 токена даа
}

// тут константы объявляем
constexpr char quit = 'q'; // символ выхода
constexpr char print = ';';  // пока идет ввод
constexpr char number = '8'; // тип - число
constexpr char name = 'a';  // тип - имя ТОКЕНА
constexpr char let = 'L';  // тип - ввод имени токена

const string prompt = "> "; // подсказка возможности ввода
const string result = "= ";  // показываем для красоты вывода
const string declkey = "let";  // аэту поеботу мы вводим с клавы когда хотим сделать ввод кода

Token Token_stream::get() {
    if (full) { // вытаскиваем символ из буфера если он заполнен
        full = false;
        return buffer;
    }

    char ch;
    cin >> ch; // ввод токенаa

    switch (ch) {
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case ';':
        case '=':  // вот эта поебота выше - обозначения для мат. операций И СУКА СКОБКИ
            return Token{ch};

        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': { // для чисел
            cin.putback(ch); // вернули ввод обратно в поток ввода, ЗАЧЕМ МЫ ЭТО ДЕЛАЕМ????
            double val; // ввели val и создали токен с типом 'число'
            cin >> val;
            return Token{number, val}; // [тип, значение]
        }

        default:
            // Если не буквенное (имя переменной начинается с буквы), то уходим
            if (isalpha(ch)) {
                string s;
                s += ch;
                while (cin.get(ch) && (isalpha(ch) || isdigit(ch)))
                    s = ch;
                cin.putback(ch); // возвращаем в поток istream??

                if (s == declkey) // ключевое слово - теперь мы начинаем писать переменную
                    return Token{let};

                return Token{name, s};
            }
            error("Bad token");
    }
} //TODO: UNDERSTAND ME

void Token_stream::ignore(char c) {
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (cin >> ch)
        if (ch == c)
            return;
} //TODO: UNDERSTAND ME

struct Variable {
    string name;
    double value;

    Variable(string n, double v) : name{n}, value{v} {}
};

vector<Variable> var_table;

double get_value(string s) {
    for (auto & i : var_table)
        if (i.name == s)
            return i.value;

    error("get: undefined name ", s);
}

void set_value(string s, double d) {
    for (int i = 0; i <= var_table.size(); ++i) {
        if (var_table[i].name == s) {
            var_table[i].value = d;
            return;
        }
    }

    error("set: undefined name ", s);
}

bool is_declared(string s) {
    for (auto & i : var_table)
        if (i.name == s)
            return true;

    return false;
}

double define_name(string var, double val) {
    if (is_declared(var))
        error(var, " declared twice");

    var_table.push_back(Variable{var, val});

    return val;
}

Token_stream ts; //TODO: UNDERSTAND ME

double expression();

double primary() {
    Token t = ts.get();
    switch (t.kind) {
        case '(': {
            double d = expression();
            t = ts.get();
            if (t.kind != ')')
                error("'(' expected");
        }

        case '-':
            return -primary();
        case '+':
            return +primary();

        case number:
            return t.value;

        case name:
            return get_value(t.name);

        default:
            error("primary expected");
    }
}

double term() {
    double left = primary();

    while (true) {
        Token t = ts.get();

        switch (t.kind) {
            case '*':
                left *= primary();
                break;

            case '/': {
                double d = primary();
                if (d == 0)
                    error("divide by zero");
                left /= d;
                break;
            }

            default:
                ts.putback(t);
                return left;
        }
    }
}

double expression() {
    double left = term();

    while (true) {
        Token t = ts.get();

        switch (t.kind) {
            case '+':
                left += term();
                break;

            case '-':
                left -= term();
                break;

            default:
                ts.putback(t);
                return left;
        }
    }
}

double declaration() {
    Token t = ts.get();
    if (t.kind != name)
        error("name expected in declaration");

    string var = t.name;
    if (is_declared(var))
        error(var, " declared twice");

    t = ts.get();
    if (t.kind != '=')
        error("'=' missing in declaration of ", var);

    return define_name(var, expression());
}

double statement() {
    Token t = ts.get();
    switch (t.kind) {
        case let:
            return declaration();
        default:
            ts.putback(t);
            return expression();
    }
}

void clean_up_mess() { ts.ignore(print); }

void calculate() { // жээээээсть
    while (true)
        try {
            cout << prompt; // подсказка что мы можем щас вводить
            Token t = ts.get(); //todo: пойми потом класс этот
            while (t.kind == print)
                t = ts.get();
            if (t.kind == quit)
                return;

            ts.putback(t);
            cout << result << statement() << endl;
        }
        catch (runtime_error &e) {
            cerr << e.what() << endl;
            clean_up_mess();
        }
}

int main()
try {
    define_name("pi", 3.141592653589793); // дефайн_нейм и все с ним связанное проверил, вроде все норм
    define_name("e", 2.718281828459045);

    calculate(); // тут весь пиздец происходит
}
catch (exception &e) {  // стд ошибки ловим
    cerr << "exception: " << e.what() << endl;
    return 1;
}
catch (...) { // эт вщ как попасть сюда
    cerr << "Oops, unknown exception" << endl;
    return 2;
}

#endif //LE_CALCULATOR_CALCULATORMAIN_H
