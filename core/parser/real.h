#ifndef REAL_H
#define REAL_H

#include "tree.h"

class RealNode : public Node
{
private:
    double val;
public:
    RealNode(double v = 0) noexcept : val(v) {}
    virtual ~RealNode(void) noexcept {}
    double value(void)
    {
        return val;
    }
};

#endif // REAL_H
