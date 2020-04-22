#ifndef SOLVER_H
#define SOLVER_H

#include <string>
#include "util/matrix.h"

using namespace std;

class TMesh;

template <class T> class TSolver
{
protected:
    T stiffnessMatrix;
    T massMatrix;
    T dampingMatrix;
    vector<double> loadVector;
    virtual bool loadMatrix(string, T&) = 0;
    virtual bool saveMatrix(string, T&) = 0;
public:
    TSolver(void) {}
    virtual ~TSolver(void) {}
    virtual void clear(void) = 0;
    virtual void setBoundaryCondition(unsigned, double) = 0;
    virtual void setMatrix(TMesh*, bool = false) = 0;
    virtual void setStiffness(double, unsigned, unsigned) = 0;
    virtual void setMass(double, unsigned, unsigned) = 0;
    virtual void setDamping(double, unsigned, unsigned) = 0;
    virtual void addStiffness(double, unsigned, unsigned) = 0;
    virtual void addMass(double, unsigned, unsigned) = 0;
    virtual void addDamping(double, unsigned, unsigned) = 0;
    virtual void product(T&, vector<double>&, vector<double>&) = 0;
    void setLoad(double value, unsigned i)
    {
        loadVector[i] = value;
    }
    void addLoad(double value, unsigned i)
    {
        loadVector[i] += value;
    }
    double getLoad(unsigned i)
    {
        return loadVector[i];
    }
    virtual double getStiffness(unsigned, unsigned) = 0;
    virtual double getMass(unsigned, unsigned) = 0;
    virtual double getDamping(unsigned, unsigned) = 0;
    T& getStiffnessMatrix(void)
    {
        return stiffnessMatrix;
    }
    T& getMassMatrix(void)
    {
        return massMatrix;
    }
    T& getDampingMatrix(void)
    {
        return dampingMatrix;
    }
    vector<double>& getLoadVector(void)
    {
        return loadVector;
    }
    virtual bool solve(vector<double>&, double, bool&) = 0;
    virtual void print(string) = 0;
    bool saveStiffnessMatrix(string fname)
    {
        return saveMatrix(fname, stiffnessMatrix);
    }
    bool loadStiffnessMatrix(string fname)
    {
        return loadMatrix(fname, stiffnessMatrix);
    }
    bool saveMassMatrix(string fname)
    {
        return saveMatrix(fname, massMatrix);
    }
    bool loadMassMatrix(string fname)
    {
        return loadMatrix(fname, massMatrix);
    }
    bool saveDampingMatrix(string fname)
    {
        return saveMatrix(fname, dampingMatrix);
    }
    bool loadDampingMatrix(string fname)
    {
        return loadMatrix(fname, dampingMatrix);
    }
};

#endif // SOLVER_H
