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
Tree::Tree(Token op, const Tree& right)
{
    nodePtr = new UnaryNode(op, right);
}
/**********************************************************************/
Tree::Tree(const Tree& left, Token op, const Tree& right)
{
    nodePtr = new BinaryNode(left, op, right);
}
/**********************************************************************/
Tree::~Tree(void)
{
    if (not nodePtr->dec())
        delete nodePtr;
}
/**********************************************************************/
Tree& Tree::operator = (const Tree& value)
{
    value.nodePtr->inc();
    if (not nodePtr->dec())
        delete nodePtr;
    nodePtr = value.nodePtr;
    return *this;
}
/**********************************************************************/
Tree Tree::operator - (void)
{
    return Tree(Tree(-1.0), Token::Mul, *this);
}
/**********************************************************************/
Tree::Tree(const Tree& value)
{
    nodePtr = value.nodePtr;
    nodePtr->inc();
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
    return Tree(Tree(l), Token::Plus, r);
}
/**********************************************************************/
Tree operator + (const Tree& l, double r)
{
    return Tree(l, Token::Plus, Tree(r));
}
/**********************************************************************/
Tree operator + (const Tree& l, const Tree& r)
{
    return Tree(l, Token::Plus, r);
}
/**********************************************************************/
Tree operator - (double l, const Tree& r)
{
    return Tree(Tree(l), Token::Minus, r);
}
/**********************************************************************/
Tree operator - (const Tree& l, double r)
{
    return Tree(l, Token::Minus, Tree(r));
}
/**********************************************************************/
Tree operator - (const Tree& l, const Tree& r)
{
    return Tree(l, Token::Minus, r);
}
/**********************************************************************/
Tree operator * (double l, const Tree& r)
{
    return Tree(Tree(l), Token::Mul, r);
}
/**********************************************************************/
Tree operator * (const Tree& l, double r)
{
    return Tree(l, Token::Mul, Tree(r));
}
/**********************************************************************/
Tree operator * (const Tree& l, const Tree& r)
{
    return Tree(l, Token::Mul, r);
}
/**********************************************************************/
Tree operator / (double l, const Tree& r)
{
    return Tree(Tree(l), Token::Div, r);
}
/**********************************************************************/
Tree operator / (const Tree& l, double r)
{
    return Tree(l, Token::Div, Tree(r));
}
/**********************************************************************/
Tree operator / (const Tree& l, const Tree& r)
{
    return Tree(l, Token::Div, r);
}
/**********************************************************************/
Tree operator ^ (double l, const Tree& r)
{
    return Tree(Tree(l), Token::Pow, r);
}
/**********************************************************************/
Tree operator ^ (const Tree& l, double r)
{
    return Tree(l, Token::Pow, Tree(r));
}
/**********************************************************************/
Tree operator ^ (const Tree& l, const Tree& r)
{
    return Tree(l, Token::Pow, r);
}
/**********************************************************************/
Tree exp(const Tree& r)
{
    return Tree(Token::Exp, r);
}
/**********************************************************************/
