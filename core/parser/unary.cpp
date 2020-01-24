#include <cmath>
#include "unary.h"


double UnaryNode::value(void)
{
    double result = 0;

    switch (op)
    {
        case MINUS:
            result = -val.value();
            break;
        case ABS:
            result = fabs(val.value());
            break;
        case SQRT:
            result = sqrt(val.value());
            break;
        case SIN:
            result = sin(val.value());
            break;
        case COS:
            result = cos(val.value());
            break;
        case TAN:
            result = tan(val.value());
            break;
        case EXP:
            result = exp(val.value());
            break;
        case ASIN:
            result = asin(val.value());
            break;
        case ACOS:
            result = acos(val.value());
            break;
        case ATAN:
            result = atan(val.value());
            break;
        case SINH:
            result = sinh(val.value());
            break;
        case COSH:
            result = cosh(val.value());
            break;
        case NOT:
            result = !int(val.value());
    }
    return result;
}
