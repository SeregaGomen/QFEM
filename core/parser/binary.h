#ifndef BINARY_H
#define BINARY_H

#include "tree.h"
#include "real.h"


class BinaryNode : public Node
{
private:
    Token op;
    Tree left;
    Tree right;
public:
    BinaryNode(const Tree& l, Token o, const Tree& r) : op(o), left(l), right(r) {}
    ~BinaryNode(void) {}
    double value(void);
};

#endif // BINARY_H
