#include <cmath>
#include "binary.h"

double BinaryNode::value(void)
{
    switch (op)
    {
        case Operation::Plus:
            return left.value() + right.value();
        case Operation::Minus:
            return left.value() - right.value();
        case Operation::Mul:
            return left.value() * right.value();
        case Operation::Div:
            return left.value() / right.value();
        case Operation::Pow:
            return pow(left.value(),right.value());
        case Operation::Eq:
            return (left.value() == right.value()) ? 1.0 : 0.0;
        case Operation::Lt:
            return (left.value() < right.value()) ? 1.0 : 0.0;
        case Operation::Le:
            return (left.value() <= right.value()) ? 1.0 : 0.0;
        case Operation::Gt:
            return (left.value() > right.value()) ? 1.0 : 0.0;
        case Operation::Ge:
            return (left.value() >= right.value()) ? 1.0 : 0.0;
        case Operation::And:
            return int(left.value()) and int(right.value());
        case Operation::Or:
            return int(left.value()) or int(right.value());
        case Operation::Atan2:
            return atan2(left.value(),right.value());
        default:
            break;
    }
    return 0;
}

