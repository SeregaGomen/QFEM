#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <vector>
#include "msg/msg.h"
#include "tree.h"

using namespace std;

/************************************************************************************************/
enum { END = 0, DELIMITER, NUMERIC, FUNCTION, VARIABLE, STRING, FINISHED };
/************************************************************************************************/
struct idToken
{
    string name;
    int type;
};
/************************************************************************************************/
class TParser
{
private:
    Tree result;
    map<string,double> variables; // Таблица переменных
    string token;
    char* expression = nullptr;
    int token_type;
    ErrorCode error_code = NO_ERR;
    int tok;
    vector<idToken> functionList{
                                    { "SQRT", SQRT },
                                    { "SIN", SIN },
                                    { "COS", COS },
                                    { "TAN", TAN },
                                    { "EXP", EXP },
                                    { "ASIN", ASIN },
                                    { "ACOS", ACOS },
                                    { "ATAN", ATAN },
                                    { "ATAN2", ATAN2 },
                                    { "SINH", SINH },
                                    { "COSH", COSH },
                                    { "TANH", TANH },
                                    { "ABS", ABS }
                                };
    vector<idToken> booleanList{
                                    { "NOT", NOT },
                                    { "AND", AND },
                                    { "OR", OR }
                                };

    vector<idToken> opeartionList{
                                    { "+", PLUS },
                                    { "-", MINUS },
                                    { "*", MUL },
                                    { "/", DIV },
                                    { "**", POW },
                                    { "==", EQ },
                                    { ">", GT },
                                    { "<", LT },
                                    { ">=", GE },
                                    { "<=", LE },
                                    { "<>", NE }
                                };
protected:
    int get_token(void);
    int is_delim(char);
    bool is_name(string);
    bool is_find(vector<idToken>&, string, int&);
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
