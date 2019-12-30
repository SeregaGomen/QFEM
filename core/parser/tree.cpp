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
Tree::Tree(int op, const Tree& right)
{
    nodePtr = new UnaryNode(op,right);
}
/**********************************************************************/
Tree::Tree(const Tree& left, int op, const Tree& right)
{
    nodePtr = new BinaryNode(left,op,right);
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
    return Tree(Tree(-1.0), MUL, *this);
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
    return Tree(Tree(l),PLUS,r);
}
/**********************************************************************/
Tree operator + (const Tree& l, double r)
{
    return Tree(l,PLUS,Tree(r));
}
/**********************************************************************/
Tree operator + (const Tree& l, const Tree& r)
{
    return Tree(l,PLUS,r);
}
/**********************************************************************/
Tree operator - (double l, const Tree& r)
{
    return Tree(Tree(l),MINUS,r);
}
/**********************************************************************/
Tree operator - (const Tree& l, double r)
{
    return Tree(l,MINUS,Tree(r));
}
/**********************************************************************/
Tree operator - (const Tree& l, const Tree& r)
{
    return Tree(l,MINUS,r);
}
/**********************************************************************/
Tree operator * (double l, const Tree& r)
{
    return Tree(Tree(l),MUL,r);
}
/**********************************************************************/
Tree operator * (const Tree& l, double r)
{
    return Tree(l,MUL,Tree(r));
}
/**********************************************************************/
Tree operator * (const Tree& l, const Tree& r)
{
    return Tree(l,MUL,r);
}
/**********************************************************************/
Tree operator / (double l, const Tree& r)
{
    return Tree(Tree(l),DIV,r);
}
/**********************************************************************/
Tree operator / (const Tree& l, double r)
{
    return Tree(l,DIV,Tree(r));
}
/**********************************************************************/
Tree operator / (const Tree& l, const Tree& r)
{
    return Tree(l,DIV,r);
}
/**********************************************************************/
Tree operator ^ (double l, const Tree& r)
{
    return Tree(Tree(l),POW,r);
}
/**********************************************************************/
Tree operator ^ (const Tree& l, double r)
{
    return Tree(l,POW,Tree(r));
}
/**********************************************************************/
Tree operator ^ (const Tree& l, const Tree& r)
{
    return Tree(l,POW,r);
}
/**********************************************************************/
Tree exp(const Tree& r)
{
    return Tree(EXP,r);
}
/**********************************************************************/
