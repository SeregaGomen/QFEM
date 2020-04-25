#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "parser.h"

/********************************************************************/
void TParser::set_expression(string prog)
{
    // Удаляем пробелы в начале и конце строки
    prog.erase(0, prog.find_first_not_of(" \t\n\r\f\v")).erase(prog.find_last_not_of(" \t\n\r\f\v") + 1);
    if (not prog.length())
        throw (error_code = ErrorCode::EEmptyExpression);

    expression = const_cast<char*>(prog.c_str());
    error_code = ErrorCode::Undefined;
    tok = Operation::Undefined;
    compile();
}
/********************************************************************/
bool TParser::is_find(vector<idToken>& table, string id, Operation& token)
{
    transform(id.begin(), id.end(),id.begin(), ::toupper);
    for (auto it : table)
        if (it.name == id)
        {
            token = it.op;
            return true;
        }
    return false;
}
/********************************************************************/
Token TParser::get_token(void)
{
    token.clear();
    token_type = Token::Undefined;
    tok = Operation::Undefined;
    if (*expression == 0)
        return (token_type = Token::Finished);
    while (*expression == ' ' or *expression == '\t')
        expression++;
    if (strchr(" +-*/()=.,><",*expression))
    {
        token = *expression++;
        // Проверка на наличие двойного разделителя
        if (*expression and strchr("=><*",*expression))
            if (is_find(opeartionList, token + *expression, tok))
                token += *expression++;
        return (token_type = Token::Delimiter);
    }
    if (isdigit(*expression))
    {
        while (isdigit(*expression))
            token += *expression++;
        if (*expression == '.')
        {
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        if (*expression == 'E' or *expression == 'e')
        {
            token += *expression++;
            if (*expression not_eq '+' and *expression not_eq '-' )
                error(ErrorCode::ESyntax);
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        return (token_type = Token::Numeric);
    }
    if (isalpha(*expression) or *expression == '_')
    {
        while (not is_delim(*expression))
            token += *expression++;
        token_type = Token::String;
    }
    if (token_type == Token::String)
    {
        if (is_find(functionList, token,tok))
            return token_type = Token::Function;
        if (is_find(booleanList, token,tok))
            return token_type = Token::Delimiter;
        return token_type = Token::Variable;
    }
    return token_type;
}
/********************************************************************/
bool TParser::is_delim(char chr)
{
    return (strchr(" +-*/()=.,><\t\n", chr) or chr == 0) ? true : false;
}
/********************************************************************/
bool TParser::is_name(string token)
{
    unsigned i;

    if (not isalpha(token[0]) and token[0] not_eq '_')
        return false;
    for (i = 1; i < token.length() + 1; i++)
        if (token[i] == ' ' or token[i] == 0)
            break;
        else
            if (not (isalpha(token[0]) or isdigit(token[0]) or token[0] == '_')) return false;
    token[i] = 0;
    return true;
}
/********************************************************************/
void TParser::compile(void)
{
    tok = Operation::Undefined;
    token_type = Token::Undefined;
    while (1)
    {
        if (token_type == Token::Finished)
            break;
        get_exp(result);
        if (token_type == Token::Delimiter)
        {
            if (token[0] == ')')
                error(ErrorCode::ECramp);
            else
                error(ErrorCode::ESyntax);
        }
    }
}
/********************************************************************/
void TParser::error(ErrorCode error)
{
    throw (error_code = error);
}
/********************************************************************/
void TParser::get_exp(Tree& code)
{
    get_token();
    if (not token.length())
        error(ErrorCode::ESyntax);
    token_or(code);
}
/********************************************************************/
void TParser::token_or(Tree& code)
{
    Tree hold;

    token_and(code);
    while (token_type not_eq Token::Finished and tok == Operation::Or)
    {
        get_token();
        token_and(hold);
        code = Tree(code, Operation::Or, hold);
    }
}
/********************************************************************/
void TParser::token_and(Tree& code)
{
    Tree hold;

    token_not(code);
    while (token_type not_eq Token::Finished and tok == Operation::And)
    {
        get_token();
        token_not(hold);
        code = Tree(code, Operation::And, hold);
    }
}
/********************************************************************/
void TParser::token_not(Tree& code)
{
    Operation op = tok;

    if (token_type == Token::Delimiter and op == Operation::Not)
        get_token();

    token_add(code);
    if (op == Operation::Not)
        code = Tree(Operation::Not, code);
}
/********************************************************************/
void TParser::token_add(Tree& code)
{
    Operation op;
    Tree hold;
    string pm;

    token_mul(code);
    while (token_type not_eq Token::Finished and ((pm = token) == "+" or pm == "-" or pm == ">" or pm == "<" or pm == ">=" or pm == "<=" or pm == "<>" or pm == "=="))
    {
        get_token();
        token_mul(hold);
        is_find(opeartionList, pm, op);
        code = Tree(code, op, hold);
    }
}
/********************************************************************/
void TParser::token_mul(Tree& code)
{
    Operation  op;
    Tree hold;
    char pm;

    token_pow(code);
    while (token_type not_eq Token::Finished and ((pm = token[0]) == '*' or pm == '/'))
    {
        get_token();
        token_pow(hold);
        if (pm == '*')
            op = Operation::Mul;
        else
            op = Operation::Div;
        code = Tree(code, op, hold);
    }
}
/********************************************************************/
void TParser::token_pow(Tree& code)
{
    Tree hold;

    token_un(code);
    if (token_type not_eq Token::Finished and token == "**")
    {
        get_token();
        token_cramp(hold);
        code = Tree(code, Operation::Pow, hold);
    }
}
/********************************************************************/
void TParser::token_un(Tree& code)
{
    Operation op = Operation::Undefined;

    if ((token_type == Token::Delimiter) and (token[0] == '+' or token[0] == '-'))
    {
        if (token[0] == '+')
            op = Operation::Plus;
        else
            op = Operation::Minus;
        get_token();
    }
    token_cramp(code);
    if (op not_eq Operation::Undefined)
        code = Tree(op, code);
}
/********************************************************************/
void TParser::token_prim(Tree& code)
{
    stringstream s;
    string var_name;
    double val;

    switch (token_type)
    {
        case Token::Variable:
            var_name = token;
            get_token();
            if (variables.find(var_name) == variables.end())
                error(ErrorCode::EUndefVariable);
            code = Tree(&(variables[var_name]));
            break;
        case Token::Numeric:
            s << token;
            s >> val;
            if (s.fail())
                error(ErrorCode::ESyntax);
            //            val = std::stod(token);
            code = Tree(val);
            get_token();
            break;
        case Token::Function:
            token_func(code);
            break;
        default:
            error(ErrorCode::ESyntax);
    }
}
/********************************************************************/
void TParser::token_cramp(Tree& code)
{
    if (token[0] == '(' and token_type == Token::Delimiter)
    {
        get_token();
        token_or(code);
        if(token[0] not_eq ')')
            error(ErrorCode::ECramp);
        get_token();
    }
    else
        token_prim(code);
}
/********************************************************************/
void TParser::token_func(Tree& code)
{
    Operation fun_tok = tok;
    Tree code2;

    if (token_type == Token::Function)
    {
        get_token();
        if (not token.length() or token[0] not_eq '(')
            error(ErrorCode::ESyntax);
        get_token();

        token_add(code);
        if (fun_tok == Operation::Atan2)
        {
            if (token[0] not_eq ',')
                error(ErrorCode::ESyntax);
            get_token();
            token_add(code2);
            code = Tree(code, Operation::Atan2, code2);
        }
        else
            code = Tree(fun_tok, code);
        if (token[0] not_eq ')')
            error(ErrorCode::ESyntax);
        get_token();
    }
}
/********************************************************************/
void TParser::set_variable(string name, double value)
{
    variables[name] = value;
}
/********************************************************************/
void TParser::set_variables(map<string, double>& m)
{
    for (auto it : m)
        variables[it.first] = it.second;
}
/********************************************************************/

