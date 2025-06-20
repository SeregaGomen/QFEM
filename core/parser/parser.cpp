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
    if (!prog.length())
        throw (error_code = ErrorCode::EEmptyExpression);

    expression = const_cast<char*>(prog.c_str());
    error_code = ErrorCode::Undefined;
    tok = Token::Indefined;
    compile();
}
/********************************************************************/
bool TParser::is_find(vector<idToken>& table, string id, Token& token)
{
    transform(id.begin(), id.end(),id.begin(), ::toupper);
    for (auto &it : table)
        if (it.name == id)
        {
            token = it.op;
            return true;
        }
    return false;
}
/********************************************************************/
TokenType TParser::get_token(void)
{
    token.clear();
    token_type = TokenType::Indefined;
    tok = Token::Indefined;
    if (*expression == 0)
        return (token_type = TokenType::Finished);
    while (*expression == ' ' || *expression == '\t')
        expression++;
    if (strchr(" +-*/()=.,><",*expression))
    {
        token = *expression++;
        // Проверка на наличие двойного разделителя
        if (*expression && strchr("=><*",*expression))
            if (is_find(opeartionList, token + *expression, tok))
                token += *expression++;
        return (token_type = TokenType::Delimiter);
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
        if (*expression == 'E' || *expression == 'e')
        {
            token += *expression++;
            if (*expression != '+' && *expression != '-' )
                error(ErrorCode::ESyntax);
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        return (token_type = TokenType::Numeric);
    }
    if (isalpha(*expression) || *expression == '_')
    {
        while (not is_delim(*expression))
            token += *expression++;
        token_type = TokenType::String;
    }
    if (token_type == TokenType::String)
    {
        if (is_find(functionList, token,tok))
            return token_type = TokenType::Function;
        if (is_find(booleanList, token,tok))
            return token_type = TokenType::Delimiter;
        return token_type = TokenType::Variable;
    }
    return token_type;
}
/********************************************************************/
bool TParser::is_delim(char chr)
{
    return (strchr(" +-*/()=.,><\t\n", chr) || chr == 0) ? true : false;
}
/********************************************************************/
bool TParser::is_name(string token)
{
    unsigned i;

    if (!isalpha(token[0]) && token[0] != '_')
        return false;
    for (i = 1; i < token.length() + 1; i++)
        if (token[i] == ' ' || token[i] == 0)
            break;
        else
            if (!(isalpha(token[0]) || isdigit(token[0]) || token[0] == '_')) return false;
    token[i] = 0;
    return true;
}
/********************************************************************/
void TParser::compile(void)
{
    tok = Token::Indefined;
    token_type = TokenType::Indefined;
    while (1)
    {
        if (token_type == TokenType::Finished)
            break;
        get_exp(result);
        if (token_type == TokenType::Delimiter)
        {
            if (token[0] == ')')
                error(ErrorCode::EBracket);
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
void TParser::get_exp(TNode& code)
{
    get_token();
    if (!token.length())
        error(ErrorCode::ESyntax);
    token_or(code);
}
/********************************************************************/
void TParser::token_or(TNode& code)
{
    TNode hold;

    token_and(code);
    while (token_type != TokenType::Finished && tok == Token::Or)
    {
        get_token();
        token_and(hold);
        code = TNode(make_shared<TNode>(code), Token::Or, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_and(TNode& code)
{
    TNode hold;

    token_not(code);
    while (token_type != TokenType::Finished && tok == Token::And)
    {
        get_token();
        token_not(hold);
        code = TNode(make_shared<TNode>(code), Token::And, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_not(TNode& code)
{
    Token op = tok;

    if (token_type == TokenType::Delimiter && op == Token::Not)
        get_token();

    token_eq(code);
    if (op == Token::Not)
        code = TNode(Token::Not, make_shared<TNode>(code));
}
/********************************************************************/
void TParser::token_eq(TNode& code)
{
    Token op;
    TNode hold;
    string pm;

    token_add(code);
    while (token_type != TokenType::Finished && ((pm = token) == ">" || pm == "<" || pm == ">=" || pm == "<=" || pm == "<>" || pm == "=="))
    {
        get_token();
        token_add(hold);
        is_find(opeartionList, pm, op);
        code = TNode(make_shared<TNode>(code), op, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_add(TNode& code)
{
    Token op;
    TNode hold;
    string pm;

    token_mul(code);
    while (token_type != TokenType::Finished && ((pm = token) == "+" || pm == "-"))
    {
        get_token();
        token_mul(hold);
        is_find(opeartionList, pm, op);
        code = TNode(make_shared<TNode>(code), op, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_mul(TNode& code)
{
    TNode hold;
    char pm;

    token_pow(code);
    while (token_type != TokenType::Finished && ((pm = token[0]) == '*' || pm == '/'))
    {
        get_token();
        token_pow(hold);
        code = TNode(make_shared<TNode>(code), (pm == '*') ? Token::Mul : Token::Div, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_pow(TNode& code)
{
    TNode hold;

    token_un(code);
    if (token_type != TokenType::Finished && token == "**")
    {
        get_token();
        token_bracket(hold);
        code = TNode(make_shared<TNode>(code), Token::Pow, make_shared<TNode>(hold));
    }
}
/********************************************************************/
void TParser::token_un(TNode& code)
{
    Token op = Token::Indefined;

    if ((token_type == TokenType::Delimiter) && (token[0] == '+' || token[0] == '-'))
    {
        if (token[0] == '+')
            op = Token::Plus;
        else
            op = Token::Minus;
        get_token();
    }
    token_bracket(code);
    if (op != Token::Indefined)
        code = TNode(op, make_shared<TNode>(code));
}
/********************************************************************/
void TParser::token_prim(TNode& code)
{
    stringstream s;
    string var_name;
    double val;

    switch (token_type)
    {
        case TokenType::Variable:
            var_name = token;
            get_token();
            if (variables.find(var_name) == variables.end())
                error(ErrorCode::EUndefVariable);
//            code = TNode(variables[var_name]);
            code = TNode(&variables[var_name]);
            break;
        case TokenType::Numeric:
            s << token;
            s >> val;
            if (s.fail())
                error(ErrorCode::ESyntax);
            //            val = std::stod(token);
            code = TNode(val);
            get_token();
            break;
        case TokenType::Function:
            token_func(code);
            break;
        default:
            error(ErrorCode::ESyntax);
    }
}
/********************************************************************/
void TParser::token_bracket(TNode& code)
{
    if (token[0] == '(' && token_type == TokenType::Delimiter)
    {
        get_token();
        token_or(code);
        if(token[0] != ')')
            error(ErrorCode::EBracket);
        get_token();
    }
    else
        token_prim(code);
}
/********************************************************************/
void TParser::token_func(TNode& code)
{
    Token fun_tok = tok;
    TNode hold;

    if (token_type == TokenType::Function)
    {
        get_token();
        if (!token.length() || token[0] != '(')
            error(ErrorCode::ESyntax);
        get_token();

        token_add(code);
        if (fun_tok == Token::Atan2)
        {
            if (token[0] != ',')
                error(ErrorCode::ESyntax);
            get_token();
            token_add(hold);
            code = TNode(make_shared<TNode>(code), Token::Atan2, make_shared<TNode>(hold));
        }
        else
            code = TNode(fun_tok, make_shared<TNode>(code));
        if (token[0] != ')')
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
    for (auto &it : m)
        variables[it.first] = it.second;
}
/********************************************************************/

