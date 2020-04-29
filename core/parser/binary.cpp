#include <cmath>
#include "binary.h"

double BinaryNode::value(void)
{
    switch (op)
    {
        case Token::Plus:
            return left.value() + right.value();
        case Token::Minus:
            return left.value() - right.value();
        case Token::Mul:
            return left.value() * right.value();
        case Token::Div:
            return left.value() / right.value();
        case Token::Pow:
            return pow(left.value(),right.value());
        case Token::Eq:
            return (left.value() == right.value()) ? 1.0 : 0.0;
        case Token::Lt:
            return (left.value() < right.value()) ? 1.0 : 0.0;
        case Token::Le:
            return (left.value() <= right.value()) ? 1.0 : 0.0;
        case Token::Gt:
            return (left.value() > right.value()) ? 1.0 : 0.0;
        case Token::Ge:
            return (left.value() >= right.value()) ? 1.0 : 0.0;
        case Token::And:
            return int(left.value()) and int(right.value());
        case Token::Or:
            return int(left.value()) or int(right.value());
        case Token::Atan2:
            return atan2(left.value(),right.value());
        default:
            break;
    }
    return 0;
}

