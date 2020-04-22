#include <cmath>
#include "unary.h"


double UnaryNode::value(void)
{
    switch (op)
    {
        case Operation::Minus:
            return -val.value();
        case Operation::Abs:
            return fabs(val.value());
        case Operation::Sqrt:
            return sqrt(val.value());
        case Operation::Sin:
            return sin(val.value());
        case Operation::Cos:
            return cos(val.value());
        case Operation::Tan:
            return tan(val.value());
        case Operation::Exp:
            return exp(val.value());
        case Operation::Asin:
            return asin(val.value());
        case Operation::Acos:
            return acos(val.value());
        case Operation::Atan:
            return atan(val.value());
        case Operation::Sinh:
            return sinh(val.value());
        case Operation::Cosh:
            return cosh(val.value());
        case Operation::Not:
            return not int(val.value());
        default:
            break;
    }
    return 0;
}
