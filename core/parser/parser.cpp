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
        throw (error_code = EMPTY_EXPRESSION_ERR);

    expression = const_cast<char*>(prog.c_str());
    error_code = NO_ERR;
    tok = 0;
    compile();
}
/********************************************************************/
bool TParser::is_find(vector<idToken>& table, string id, int& token)
{
    transform(id.begin(), id.end(),id.begin(), ::toupper);
    for (auto it : table)
        if (it.name == id)
        {
            token = it.type;
            return true;
        }
    return false;
}
/********************************************************************/
int TParser::get_token(void)
{
    token.clear();
    token_type = tok = 0;
    if (*expression == 0)
    {
        tok = FINISHED;
        return (token_type = DELIMITER);
    }
    while (*expression == ' ' || *expression == '\t')
        expression++;
    if (strchr(" +-*/()=.,><",*expression))
    {
        token = *expression++;
        // Проверка на наличие двойного разделителя
        if (*expression && strchr("=><*",*expression))
            if (is_find(opeartionList, token + *expression, tok))
                token += *expression++;
        return (token_type = DELIMITER);
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
                error(SYNTAX_ERR);
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        return (token_type = NUMERIC);
    }
    if (isalpha(*expression) || *expression == '_')
    {
        while (!is_delim(*expression))
            token += *expression++;
        token_type = STRING;
    }
    if (token_type == STRING)
    {
        if (is_find(functionList, token,tok))
            return token_type = FUNCTION;
        if (is_find(booleanList, token,tok))
            return token_type = DELIMITER;
        return token_type = VARIABLE;
    }
    return token_type;
}
/********************************************************************/
int TParser::is_delim(char chr)
{
    if (strchr(" +-*/()=.,><\t\n", chr) || chr == 0)
        return 1;
    return 0;
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
    tok = 0;
    while (1)
    {
        if (tok == FINISHED)
            break;
        get_exp(result);
        if (token_type == DELIMITER && tok != FINISHED)
        {
            if (token[0] == ')')
                error(CRAMP_ERR);
            else
                error(SYNTAX_ERR);
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
    if (!token.length())
        error(SYNTAX_ERR);
    token_or(code);
}
/********************************************************************/
void TParser::token_or(Tree& code)
{
    Tree hold;

    token_and(code);
    while (tok != FINISHED && tok == OR)
    {
        get_token();
        token_and(hold);
        code = Tree(code,OR,hold);
    }
}
/********************************************************************/
void TParser::token_and(Tree& code)
{
    Tree hold;

    token_not(code);
    while (tok != FINISHED && tok == AND)
    {
        get_token();
        token_not(hold);
        code = Tree(code,AND,hold);
    }
}
/********************************************************************/
void TParser::token_not(Tree& code)
{
    int op = tok;

    if (tok == DELIMITER && op == NOT)
        get_token();

    token_add(code);
    if (op == NOT)
        code = Tree(NOT,code);
}
/********************************************************************/
void TParser::token_add(Tree& code)
{
    int op;
    Tree hold;
    string pm;

    token_mul(code);
    while (tok != FINISHED && ((pm = token) == "+" || pm == "-" || pm == ">" || pm == "<" || pm == ">=" || pm == "<=" || pm == "<>" || pm == "=="))
    {
        get_token();
        token_mul(hold);
        is_find(opeartionList, pm, op);
        code = Tree(code,op,hold);
    }
}
/********************************************************************/
void TParser::token_mul(Tree& code)
{
    int  op;
    Tree hold;
    char pm;

    token_pow(code);
    while (tok != FINISHED && ((pm = token[0]) == '*' || pm == '/'))
    {
        get_token();
        token_pow(hold);
        if (pm == '*')
            op = MUL;
        else
            op = DIV;
        code = Tree(code,op,hold);
    }
}
/********************************************************************/
void TParser::token_pow(Tree& code)
{
    Tree hold;

    token_un(code);
    if (tok != FINISHED && token == "**")
    {
        get_token();
        token_cramp(hold);
        code = Tree(code,POW,hold);
    }
}
/********************************************************************/
void TParser::token_un(Tree& code)
{
    int op = 0;

    if ((token_type == DELIMITER) && (token[0] == '+' || token[0] == '-'))
    {
        if (token[0] == '+')
            op = PLUS;
        else
            op = MINUS;
        get_token();
    }
    token_cramp(code);
    if (op)
        code = Tree(op,code);
}
/********************************************************************/
void TParser::token_prim(Tree& code)
{
    stringstream s;
    string var_name;
    double val;

    switch(token_type)
    {
        case VARIABLE:
            var_name = token;
            get_token();
            if (variables.find(var_name) == variables.end())
                error(UNDEF_VARIABLE_ERR);
            code = Tree(&(variables[var_name]));
            break;
        case NUMERIC:
            s << token;
            s >> val;
            if (s.fail())
                error(SYNTAX_ERR);
            //            val = std::stod(token);
            code = Tree(val);
            get_token();
            break;
        case FUNCTION:
            token_func(code);
            break;
        default:
            error(SYNTAX_ERR);
    }
}
/********************************************************************/
void TParser::token_cramp(Tree& code)
{
    if(tok != FINISHED  && token[0] == '(' && token_type == DELIMITER)
    {
        get_token();
        token_or(code);
        if(token[0] != ')')
            error(CRAMP_ERR);
        get_token();
    }
    else
        token_prim(code);
}
/********************************************************************/
void TParser::token_func(Tree& code)
{
    int fun_tok = tok;
    Tree code2;

    if (token_type == FUNCTION)
    {
        get_token();
        if (!token.length() || token[0] != '(')
            error(SYNTAX_ERR);
        get_token();

        token_add(code);
        if (fun_tok == ATAN2)
        {
            if (token[0] != ',')
                error(SYNTAX_ERR);
            get_token();
            token_add(code2);
            code = Tree(code,ATAN2,code2);
        }
        else
            code = Tree(fun_tok,code);
        if (token[0] != ')')
            error(SYNTAX_ERR);
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

