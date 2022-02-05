#ifndef DEFS_H
#define DEFS_H

#include <string>

namespace Parser
{
    enum class Token { Indefined, Number, Variable, Sin, Cos, Tan, Exp, Asin, Acos, Atan, Atan2, Sinh, Cosh, Tanh, Sqrt, Abs, Plus, Minus, Div, Mul, Pow, Eq, Ne, Lt, Le, Gt, Ge, Not, And, Or };
    enum class TokenType { Indefined, Delimiter, Numeric, Function, Variable, String, Finished };
    struct idToken
    {
        std::string name;
        Token op;
    };
}

#endif // DEFS_H
