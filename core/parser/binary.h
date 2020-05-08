#ifndef BINARY_H
#define BINARY_H

#include "tree.h"
#include "real.h"


class BinaryNode : public Node
{
private:
    Token op;
    Tree lhs;
    Tree rhs;
public:
    BinaryNode(const Tree& l, Token o, const Tree& r) noexcept : op(o), lhs(l), rhs(r) {}
    virtual ~BinaryNode(void) noexcept {}
    double value(void);
};

#endif // BINARY_H
