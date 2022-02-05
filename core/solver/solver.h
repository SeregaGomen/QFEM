#ifndef SOLVER_H
#define SOLVER_H

#include <mutex>
#include <string>
#include "util/matrix.h"

using namespace std;

class TMesh;

template <class MATRIX, class VECTOR> class TSolver
{
protected:
    mutex mtx;
    MATRIX stiffness;
    MATRIX mass;
    MATRIX damping;
    VECTOR load;
public:
    TSolver(void) = default;
    virtual ~TSolver(void) = default;
    virtual void setBoundaryCondition(unsigned, double) = 0;
    virtual void setMatrix(TMesh*, bool = false) = 0;
    virtual void setStiffness(double, unsigned, unsigned) = 0;
    virtual void setMass(double, unsigned, unsigned) = 0;
    virtual void setDamping(double, unsigned, unsigned) = 0;
    virtual void addStiffness(double, unsigned, unsigned) = 0;
    virtual void addMass(double, unsigned, unsigned) = 0;
    virtual void addDamping(double, unsigned, unsigned) = 0;
    virtual double getStiffness(unsigned, unsigned) = 0;
    virtual double getMass(unsigned, unsigned) = 0;
    virtual double getDamping(unsigned, unsigned) = 0;
    virtual bool solve(vector<double>&, double, bool&) = 0;
    void setLoad(double value, unsigned i)
    {
        load[i] = value;
    }
    void addLoad(double value, unsigned i)
    {
        load[i] += value;
    }
    double getLoad(unsigned i)
    {
        return load[i];
    }
};

#endif // SOLVER_H
