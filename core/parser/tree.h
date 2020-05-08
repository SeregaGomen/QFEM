#ifndef TREE_H
#define TREE_H

#include "defs.h"
#include "node.h"

using namespace Parser;

class Tree
{
private:
    Node* nodePtr;
public:
    Tree(void);
    Tree(double);
    Tree(Token, const Tree&);
    Tree(const Tree&, Token, const Tree&);
    Tree(const Tree&);
    ~Tree(void);
    double value(void);
    Tree& operator = (const Tree&);
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
