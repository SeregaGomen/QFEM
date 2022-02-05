#ifndef NODE_H
#define NODE_H

#include <variant>
#include <cmath>
#include <memory>
#include "defs.h"

using namespace std;
using namespace Parser;

class TNode
{
private:
    variant<double, double*> val = 0.;
    Token tok = Token::Indefined;
    shared_ptr<TNode> left;
    shared_ptr<TNode> right;
public:
    TNode(double v = 0) : val{v}, tok{Token::Number} {}
    TNode(double *v) : val{v}, tok{Token::Variable} {}
    TNode(Token t, shared_ptr<TNode> n) : tok{t}, right{n} {}
    TNode(shared_ptr<TNode> lhs,  Token t, shared_ptr<TNode> rhs) : tok{t}, left{make_shared<TNode>(*lhs)}, right{make_shared<TNode>(*rhs)} {}
    TNode(const TNode &rhs) : val{rhs.val}, tok{rhs.tok}, left{rhs.left}, right{rhs.right} {}
    ~TNode(void) noexcept {}
    double value(void)
    {
        switch (tok)
        {
        case Token::Number:
            return get<0>(val);
        case Token::Variable:
            return *get<1>(val);
        case Token::Plus:
            if (left == nullptr) // unary
                return +right->value();
            else
                return left->value() + right->value();
        case Token::Minus:
            if (left == nullptr)
                return -right->value();
            else
                return left->value() - right->value();
        case Token::Mul:
            return left->value() * right->value();
        case Token::Div:
            return left->value() / right->value();
        case Token::Pow:
            return pow(left->value(), right->value());
        case Token::Eq:
            return (left->value() == right->value()) ? 1 : 0;
        case Token::Ne:
            return (left->value() == right->value()) ? 0 : 1;
        case Token::Lt:
            return (left->value() < right->value()) ? 1 : 0;
        case Token::Le:
            return (left->value() <= right->value()) ? 1 : 0;
        case Token::Gt:
            return (left->value() > right->value()) ? 1 : 0;
        case Token::Ge:
            return (left->value() >= right->value()) ? 1 : 0;
        case Token::And:
            return left->value() and right->value();
        case Token::Or:
            return left->value() or right->value();
        case Token::Not:
            return not right->value();
        case Token::Abs:
            return fabs(right->value());
        case Token::Sin:
            return sin(right->value());
        case Token::Cos:
            return cos(right->value());
        case Token::Tan:
            return tan(right->value());
        case Token::Exp:
            return exp(right->value());
        case Token::Asin:
            return asin(right->value());
        case Token::Acos:
            return acos(right->value());
        case Token::Atan:
            return atan(right->value());
        case Token::Atan2:
            return atan2(left->value(), right->value());
        case Token::Sinh:
            return sinh(right->value());
        case Token::Cosh:
            return cosh(right->value());
        case Token::Tanh:
            return tanh(right->value());
        case Token::Sqrt:
            return sqrt(right->value());
        default:
            throw exception();
        }
        return 0;
    }
    TNode& operator = (const TNode &rhs)
    {
        val = rhs.val;
        tok = rhs.tok;
//        left = shared_ptr<TNode>(new TNode(*rhs.left));
//        right = shared_ptr<TNode>(new TNode(*rhs.right));
        left = rhs.left;
        right = rhs.right;
        return *this;
    }
};

#endif // NODE_H
