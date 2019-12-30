#ifndef NODE_H
#define NODE_H


class Node
{
friend class Tree;
private:
    int use = 1;
public:
    Node(void) {}
    virtual ~Node(void) {}
    virtual double value(void) = 0;
};

#endif // NODE_H
