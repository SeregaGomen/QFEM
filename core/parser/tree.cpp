#include "tree.h"
#include "binary.h"
#include "unary.h"

/**********************************************************************/
Tree::Tree(void)
{
    nodePtr = new RealNode();
}
/**********************************************************************/
Tree::Tree(double value)
{
    nodePtr = new RealNode(value);
}
/**********************************************************************/
Tree::Tree(double* value)
{
    nodePtr = new ArgNode(value);
}
/**********************************************************************/
Tree::Tree(Operation op, const Tree& right)
{
    nodePtr = new UnaryNode(op, right);
}
/**********************************************************************/
Tree::Tree(const Tree& left, Operation op, const Tree& right)
{
    nodePtr = new BinaryNode(left, op, right);
}
/**********************************************************************/
Tree::~Tree(void)
{
    if (!--nodePtr->use)
        delete nodePtr;
}
/**********************************************************************/
Tree Tree::operator = (const Tree& value)
{
    ++value.nodePtr->use;
    if (!--nodePtr->use)
        delete nodePtr;
    nodePtr = value.nodePtr;
    return *this;
}
/**********************************************************************/
Tree Tree::operator - (void)
{
    return Tree(Tree(-1.0), Operation::Mul, *this);
}
/**********************************************************************/
Tree::Tree(const Tree& value)
{
    nodePtr = value.nodePtr;
    ++nodePtr->use;
}
/**********************************************************************/
double Tree::value(void)
{
    return nodePtr->value();
}
/**********************************************************************/
Tree Tree::operator += (const Tree& r)
{
    *this = *this + r;
    return *this;
}
/**********************************************************************/
Tree operator + (double l, const Tree& r)
{
    return Tree(Tree(l), Operation::Plus, r);
}
/**********************************************************************/
Tree operator + (const Tree& l, double r)
{
    return Tree(l, Operation::Plus, Tree(r));
}
/**********************************************************************/
Tree operator + (const Tree& l, const Tree& r)
{
    return Tree(l, Operation::Plus, r);
}
/**********************************************************************/
Tree operator - (double l, const Tree& r)
{
    return Tree(Tree(l), Operation::Minus, r);
}
/**********************************************************************/
Tree operator - (const Tree& l, double r)
{
    return Tree(l, Operation::Minus, Tree(r));
}
/**********************************************************************/
Tree operator - (const Tree& l, const Tree& r)
{
    return Tree(l, Operation::Minus, r);
}
/**********************************************************************/
Tree operator * (double l, const Tree& r)
{
    return Tree(Tree(l), Operation::Mul, r);
}
/**********************************************************************/
Tree operator * (const Tree& l, double r)
{
    return Tree(l, Operation::Mul, Tree(r));
}
/**********************************************************************/
Tree operator * (const Tree& l, const Tree& r)
{
    return Tree(l, Operation::Mul, r);
}
/**********************************************************************/
Tree operator / (double l, const Tree& r)
{
    return Tree(Tree(l), Operation::Div, r);
}
/**********************************************************************/
Tree operator / (const Tree& l, double r)
{
    return Tree(l, Operation::Div, Tree(r));
}
/**********************************************************************/
Tree operator / (const Tree& l, const Tree& r)
{
    return Tree(l, Operation::Div, r);
}
/**********************************************************************/
Tree operator ^ (double l, const Tree& r)
{
    return Tree(Tree(l), Operation::Pow, r);
}
/**********************************************************************/
Tree operator ^ (const Tree& l, double r)
{
    return Tree(l, Operation::Pow, Tree(r));
}
/**********************************************************************/
Tree operator ^ (const Tree& l, const Tree& r)
{
    return Tree(l, Operation::Pow, r);
}
/**********************************************************************/
Tree exp(const Tree& r)
{
    return Tree(Operation::Exp, r);
}
/**********************************************************************/
