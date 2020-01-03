#ifndef TREE_H
#define TREE_H

#include "node.h"


enum Op { SIN = 1, COS, TAN, EXP, ASIN, ACOS, ATAN, ATAN2, SINH, COSH, TANH,
          ABS, PLUS, MINUS, DIV, MUL, POW, EQ, NE, LT, LE, GT, GE, NOT, AND, OR };


class Tree
{
private:
    Node* nodePtr;
public:
    Tree(void);
    Tree(double*);
    Tree(double);
    Tree(int,const Tree&);
    Tree(const Tree&, int, const Tree&);
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
