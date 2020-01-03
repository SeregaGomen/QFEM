#ifndef BINARY_H
#define BINARY_H

#include "tree.h"
#include "real.h"


class BinaryNode : public Node
{
private:
    int op;
    Tree left;
    Tree right;
public:
    BinaryNode(const Tree& l, int o, const Tree& r)
    {
        left = l;
        op = o;
        right = r;
    }
    ~BinaryNode(void) {}
    double value(void);
};

#endif // BINARY_H
