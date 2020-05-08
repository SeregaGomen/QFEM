#ifndef UNARY_H
#define UNARY_H

#include "node.h"
#include "tree.h"

class UnaryNode : public Node
{
private:
    Token op;
    Tree val;
public:
    UnaryNode(Token o, const Tree& v) noexcept : op(o), val(v)  {}
    virtual ~UnaryNode(void) noexcept {}
    double value(void);
};

#endif // UNARY_H
