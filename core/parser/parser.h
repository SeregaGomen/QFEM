#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <vector>
#include "msg/msg.h"
#include "tree.h"

using namespace std;

/************************************************************************************************/
enum class Token { Undefined, Delimiter, Numeric, Function, Variable, String, Finished };
/************************************************************************************************/
struct idToken
{
    string name;
    Operation op;
};
/************************************************************************************************/
class TParser
{
private:
    Tree result;
    map<string,double> variables; // Таблица переменных
    string token;
    char* expression = nullptr;
    Token token_type = Token::Undefined;
    ErrorCode error_code = NO_ERR;
    Operation tok = Operation::Undefined;
    vector<idToken> functionList{
                                    { "SQRT", Operation::Sqrt },
                                    { "SIN", Operation::Sin },
                                    { "COS", Operation::Cos },
                                    { "TAN", Operation::Tan },
                                    { "EXP", Operation::Exp },
                                    { "ASIN", Operation::Asin },
                                    { "ACOS", Operation::Acos },
                                    { "ATAN", Operation::Atan },
                                    { "ATAN2", Operation::Atan2 },
                                    { "SINH", Operation::Sinh },
                                    { "COSH", Operation::Cosh },
                                    { "TANH", Operation::Tanh },
                                    { "ABS", Operation::Abs }
                                };
    vector<idToken> booleanList{
                                    { "NOT", Operation::Not },
                                    { "AND", Operation::And },
                                    { "OR", Operation::Or }
                                };

    vector<idToken> opeartionList{
                                    { "+", Operation::Plus },
                                    { "-", Operation::Minus },
                                    { "*", Operation::Mul },
                                    { "/", Operation::Div },
                                    { "**", Operation::Pow },
                                    { "==", Operation::Eq },
                                    { ">", Operation::Gt },
                                    { "<", Operation::Lt },
                                    { ">=", Operation::Ge },
                                    { "<=", Operation::Le },
                                    { "<>", Operation::Ne }
                                };
protected:
    Token get_token(void);
    bool is_delim(char);
    bool is_name(string);
    bool is_find(vector<idToken>&, string, Operation&);
    [[noreturn ]] void error(ErrorCode);
    void compile(void);
    void get_exp(Tree&);
    void token_or(Tree&);
    void token_and(Tree&);
    void token_not(Tree&);
    void token_add(Tree&);
    void token_func(Tree&);
    void token_mul(Tree&);
    void token_pow(Tree&);
    void token_un(Tree&);
    void token_cramp(Tree&);
    void token_prim(Tree&);
public:
    TParser(void) {}
    ~TParser(void) {}
    void set_expression(string);
    void set_variable(string, double value = 0);
    void set_variables(map<string, double>&);
    int get_error(void)
    {
        return error_code;
    }
    double run(void)
    {
        return  result.value();
    }
};
/************************************************************************************************/
#endif //PARSER_H
