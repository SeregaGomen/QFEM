#include <cmath>
#include "binary.h"

double BinaryNode::value(void)
{
    switch (op)
    {
        case Token::Plus:
            return lhs.value() + rhs.value();
        case Token::Minus:
            return lhs.value() - rhs.value();
        case Token::Mul:
            return lhs.value() * rhs.value();
        case Token::Div:
            return lhs.value() / rhs.value();
        case Token::Pow:
            return pow(lhs.value(),rhs.value());
        case Token::Eq:
            return (lhs.value() == rhs.value()) ? 1.0 : 0.0;
        case Token::Lt:
            return (lhs.value() < rhs.value()) ? 1.0 : 0.0;
        case Token::Le:
            return (lhs.value() <= rhs.value()) ? 1.0 : 0.0;
        case Token::Gt:
            return (lhs.value() > rhs.value()) ? 1.0 : 0.0;
        case Token::Ge:
            return (lhs.value() >= rhs.value()) ? 1.0 : 0.0;
        case Token::And:
            return int(lhs.value()) and int(rhs.value());
        case Token::Or:
            return int(lhs.value()) or int(rhs.value());
        case Token::Atan2:
            return atan2(lhs.value(),rhs.value());
        default:
            break;
    }
    return 0;
}

