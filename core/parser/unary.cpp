#include <cmath>
#include "unary.h"


double UnaryNode::value(void)
{
    switch (op)
    {
        case Token::Minus:
            return -val.value();
        case Token::Abs:
            return fabs(val.value());
        case Token::Sqrt:
            return sqrt(val.value());
        case Token::Sin:
            return sin(val.value());
        case Token::Cos:
            return cos(val.value());
        case Token::Tan:
            return tan(val.value());
        case Token::Exp:
            return exp(val.value());
        case Token::Asin:
            return asin(val.value());
        case Token::Acos:
            return acos(val.value());
        case Token::Atan:
            return atan(val.value());
        case Token::Sinh:
            return sinh(val.value());
        case Token::Cosh:
            return cosh(val.value());
        case Token::Not:
            return not int(val.value());
        default:
            break;
    }
    return 0;
}
