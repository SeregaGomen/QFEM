#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <vector>
#include "msg/msg.h"
#include "defs.h"
#include "node.h"

using namespace std;
using namespace Parser;

/************************************************************************************************/
class TParser
{
private:
    TNode result;
    map<string, double> variables; // Таблица переменных
    string token;
    char* expression = nullptr;
    Parser::Token tok = Token::Indefined;
    Parser::TokenType token_type = TokenType::Indefined;
    ErrorCode error_code = ErrorCode::Undefined;
    vector<Parser::idToken> functionList{
                                    { "SQRT", Token::Sqrt },
                                    { "SIN", Token::Sin },
                                    { "COS", Token::Cos },
                                    { "TAN", Token::Tan },
                                    { "EXP", Token::Exp },
                                    { "ASIN", Token::Asin },
                                    { "ACOS", Token::Acos },
                                    { "ATAN", Token::Atan },
                                    { "ATAN2", Token::Atan2 },
                                    { "SINH", Token::Sinh },
                                    { "COSH", Token::Cosh },
                                    { "TANH", Token::Tanh },
                                    { "ABS", Token::Abs }
                                };
    vector<idToken> booleanList{
                                    { "NOT", Token::Not },
                                    { "AND", Token::And },
                                    { "OR", Token::Or }
                                };

    vector<idToken> opeartionList{
                                    { "+", Token::Plus },
                                    { "-", Token::Minus },
                                    { "*", Token::Mul },
                                    { "/", Token::Div },
                                    { "**", Token::Pow },
                                    { "==", Token::Eq },
                                    { ">", Token::Gt },
                                    { "<", Token::Lt },
                                    { ">=", Token::Ge },
                                    { "<=", Token::Le },
                                    { "<>", Token::Ne }
                                };
    Parser::TokenType get_token(void);
    bool is_delim(char);
    bool is_name(string);
    bool is_find(vector<idToken>&, string, Token&);
    /*[[noreturn ]]*/ void error(ErrorCode);
    void compile(void);
    void get_exp(TNode&);
    void token_or(TNode&);
    void token_and(TNode&);
    void token_not(TNode&);
    void token_eq(TNode&);
    void token_add(TNode&);
    void token_func(TNode&);
    void token_mul(TNode&);
    void token_pow(TNode&);
    void token_un(TNode&);
    void token_bracket(TNode&);
    void token_prim(TNode&);
public:
    TParser(void) {}
    ~TParser(void) {}
    void set_expression(string);
    void set_variable(string, double value = 0);
    void set_variables(map<string, double>&);
    ErrorCode get_error(void)
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
