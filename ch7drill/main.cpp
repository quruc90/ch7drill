#include "../std_lib_facilities.h"


constexpr char number = 'n';
constexpr char quit = 'Q';
constexpr char print = ';';
constexpr char name = 'a';
constexpr char let = 'L';
constexpr char root = 'R';
constexpr char power = 'P';

const string declkeyword = "let";       //a feladat szerint #-nak kéne lennie let helyett, de ha spec. karakter van benne, nem működik
const string exitkeyword = "exit";
const string sqrtkeyword = "sqrt";
const string powerkeyword = "pow";

double expression();        //Előre deklarálni kell, később a primary() használja

//---------------------------------------------------- START OF VARIABLES

class Variable{
public:
    string name;
    double value;
};

vector<Variable> var_table;         //Itt tároljuk a változókat, amiket a felhasználó megad

bool is_declared(string var){
    for(auto v : var_table)
        if(v.name == var) return true;
    return false;
}

double define_name(string var, double val){
    if (is_declared(var)) error("Variable was already declared.");
    var_table.push_back(Variable{var,val});
    return val;
}

double get_value(string var)
{
    for(auto v: var_table)
        if(v.name == var) return v.value;
    error("get:Variable not defined.");
    return 0;
}

void set_value(string var, double val)
{
    for (auto v: var_table)
        if(v.name == var){
            v.value = val;
            return;
        }
    error("set:Variable not defined.");
}

//------------------------------------------------------------------------ END OF VARIABLES
//---------------------------------------------------------------------- START OF TOKEN (STREAM)

class Token {                       //a program Token típusú elemeket használ
public:
    char kind;
    double value;
    string name;

    Token(): kind(0) {}
    Token(char ch): kind(ch), value(0) {}                       //minden tokennek van kind-ja
    Token(char ch, double val): kind(ch), value(val) {}         //ha szám, értéke is lesz
    Token(char ch, string n): kind(ch), name(n) {}              //ha változó, neve is lesz
};

class Token_stream {    //ezzel többjegyű számot is tud kezelni
public:
    Token_stream();
    Token get();
    void putback(Token t);
    void ignore(char c);
private:
    bool full;
    Token buffer;
};

Token_stream::Token_stream(): full(false), buffer(0) {}

void Token_stream::putback(Token t) {
    if (full) error("Buffer full\n");
    full = true;
    buffer = t;
}

Token Token_stream::get() {

    if(full)
    {
        full = false;
        return buffer;
    }

//--------------------------------------------------- END OF TOKEN
//------------------------------------------------- START OF OPERATORS AND OPERANDS

    char ch;
    cin >> ch;

    switch(ch){     //ezeknet a karaktereket tudja kezelni a program (változó neveket NEM beleértve)
    case print:
    case 'R':
    case 'P':
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
        return Token(ch);               //ha mûveleti jel, akkor azt adja vissza
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        {
            cin.putback(ch);            //ezzel több számjegyû számot is tud kezelni
            double val = 0;
            cin >> val;
            return Token(number, val);     //ha szám, akkor a szám "kind"-ot adja vissza, meg az értékét
        }
    default:
        {
            if(isalpha(ch)){
                string s;
                s += ch;
                while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s += ch;
                cin.putback(ch);
                if (s == declkeyword) return Token(let);
                if (s == exitkeyword) return Token(quit);
                if (s == sqrtkeyword) return Token(root);
                if (s == powerkeyword) return Token(power);
                else if(is_declared(s))
                    return Token(number, get_value(s));
                return Token(name, s);
            }
            error("Bad token");
            return 0;
        }
    }
}

//--------------------------------------------------- END OF OPERATORS AND OPERANDS
//--------------------------------------------------- START OF ORDER OF OPERATIONS

void Token_stream::ignore(char c) {
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while(cin >> ch)
        if (ch == c) return;
}

Token_stream ts;

double rootfunc(){
    double r;
    Token t = ts.get();
    r = t.value;
    return sqrt(r);
}

double primary() {
    Token t = ts.get();
    switch (t.kind){
        case '(':
        {
            double d = expression();
            t = ts.get();
            if(t.kind != ')') error("Missing ')'\n");
            return d;
        }
        case number:
            return t.value;
        case '-':
            return - primary();
        case 'R':
            return rootfunc();
            break;
        case '+':
            return primary();
        default:
            error("primary expected");
            return 0;
    }
}

double term() {
    double left = primary();
    Token t = ts.get();

    while(true){
        switch(t.kind){
        case '*':
            left *= primary();
            t=ts.get();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0) error("Divided by 0 (/)");
            left /= d;
            t=ts.get();
            break;
        }
        case '%':
        {
            double d = primary();
            if (d == 0) error("Divided by 0 (%)");
            //left %= primary();  //modulo
            left = fmod(left,d);
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
    Token t = ts.get();

    while(true){
        switch(t.kind){
        case '+':
            left += term();
            t=ts.get();
            break;
        case '-':
            left -= term();
            t=ts.get();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

void clean_up_mess() {
    ts.ignore(print);}

double declaration(){
    Token t = ts.get();
    if(t.kind != name) error("Name expected in declaration.\n");
    string var_name = t.name;

    t = ts.get();
    if (t.kind != '=') error("'=' expected in declaration.\n");

    double d = expression();

    define_name(var_name,d);

    return d;
}

void calculate();

double statement(){
    Token t = ts.get();
    switch(t.kind){
    case let:
        return declaration();
    case quit:
        exit(0);
//    case root:
//        return rootfunc();
//    case power:
//        return powerfunc();
    default:
        ts.putback(t);
        return expression();
    }
}

const string prompt = "> ";
const string result = "= ";

void calculate() {

    while(cin)
    try{
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t = ts.get();
        ts.putback(t);

        cout << result << statement() << '\n';
    }catch (exception& e) {
        cerr << e.what()<< '\n';
        clean_up_mess();
    }
}

int main()
try {
    define_name("pi",3.1415926535);
    define_name("e",2.7182818284);
    define_name("k",1000);
    calculate();
    return 0;
}
catch (exception& e) {
    cerr << "Error: " << e.what () << '\n';
    return 1;
}
