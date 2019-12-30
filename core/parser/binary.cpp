#include <cmath>
#include "binary.h"

double BinaryNode::value(void)
{
    double result = 0;

    switch (op)
    {
        case PLUS:
            result = left.value() + right.value();
            break;
        case MINUS:
            result = left.value() - right.value();
            break;
        case MUL:
            result = left.value()*right.value();
            break;
        case DIV:
            result = left.value()/right.value();
            break;
        case POW:
            result = pow(left.value(),right.value());
            break;
        case EQ:
            result = (left.value() == right.value()) ? 1.0 : 0.0;
            break;
        case LT:
            result = (left.value() < right.value()) ? 1.0 : 0.0;
            break;
        case LE:
            result = (left.value() <= right.value()) ? 1.0 : 0.0;
            break;
        case GT:
            result = (left.value() > right.value()) ? 1.0 : 0.0;
            break;
        case GE:
            result = (left.value() >= right.value()) ? 1.0 : 0.0;
            break;
        case AND:
            result = int(left.value()) && int(right.value());
            break;
        case OR:
            result = int(left.value()) || int(right.value());
            break;
        case ATAN2:
            result = atan2(left.value(),right.value());
    }
    return result;
}

