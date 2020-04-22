#ifndef TREE_H
#define TREE_H

#include "node.h"


enum class Operation { Undefined, Sin, Cos, Tan, Exp, Asin, Acos, Atan, Atan2, Sinh, Cosh, Tanh, Sqrt, Abs, Plus, Minus, Div, Mul, Pow, Eq, Ne, Lt, Le, Gt, Ge, Not, And, Or };


class Tree
{
private:
    Node* nodePtr;
public:
    Tree(void);
    Tree(double*);
    Tree(double);
    Tree(Operation, const Tree&);
    Tree(const Tree&, Operation, const Tree&);
    Tree(const Tree&);
   ~Tree(void);
    double value(void);
    Tree operator = (const Tree&);
    Tree operator - (void);
    Tree operator += (const Tree&);
    friend Tree operator + (double, const Tree&);
    friend Tree operator + (const Tree&, double);
    friend Tree operator + (const Tree&, const Tree&);
    friend Tree operator - (double, const Tree&);
    friend Tree operator - (const Tree&, double);
    friend Tree operator - (const Tree&, const Tree&);
    friend Tree operator * (double, const Tree&);
    friend Tree operator * (const Tree&, double);
    friend Tree operator * (const Tree&, const Tree&);
    friend Tree operator / (double, const Tree&);
    friend Tree operator / (const Tree&, double);
    friend Tree operator / (const Tree&, const Tree&);
    friend Tree operator ^ (double, const Tree&);
    friend Tree operator ^ (const Tree&, double);
    friend Tree operator ^ (const Tree&, const Tree&);
    friend Tree exp(const Tree&);
};

#endif //TREE_H
