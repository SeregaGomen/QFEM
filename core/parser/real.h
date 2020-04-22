#ifndef REAL_H
#define REAL_H

#include "tree.h"

class RealNode : public Node
{
private:
    double val;
public:
    RealNode(double v) : val(v) {}
    RealNode(void) : val(0.0) {}
    ~RealNode(void) {}
    double value(void)
    {
        return val;
    }
};

class ArgNode : public Node
{
private:
    double* val;
public:
    ArgNode(double* v) : val(v) {}
    ~ArgNode(void) {}
    double value(void)
    {
        return *val;
    }
};


#endif // REAL_H
