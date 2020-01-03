#ifndef SOLVER_H
#define SOLVER_H

#include <string>
#include "util/matrix.h"

using namespace std;

class TMesh;

template <class T> class TSolver
{
protected:
    T globalStiffnessMatrix;
    T globalMassMatrix;
    T globalDampingMatrix;
    vector<double> globalLoadVector;
    virtual bool loadMatrix(string, T&) = 0;
    virtual bool saveMatrix(string, T&) = 0;
public:
    TSolver(void) {}
    virtual ~TSolver(void) {}
    virtual void clear(void) = 0;
    virtual void createDynamicMatrix(double, double) = 0;
    virtual void createDynamicVector(matrix<double>&, double, double) = 0;
    virtual void setBoundaryCondition(unsigned, unsigned, double) = 0;
    virtual void setupStaticMatrix(TMesh*) = 0;
    virtual void setupDynamicMatrix(TMesh*) = 0;
    virtual void setStiffnessMatrix(double, unsigned, unsigned) = 0;
    virtual void setMassMatrix(double, unsigned, unsigned) = 0;
    virtual void setDampingMatrix(double, unsigned, unsigned) = 0;
    virtual void addStiffnessMatrix(double, unsigned, unsigned) = 0;
    virtual void addMassMatrix(double, unsigned, unsigned) = 0;
    virtual void addDampingMatrix(double, unsigned, unsigned) = 0;
    void setLoadVector(double value, unsigned i)
    {
        globalLoadVector[i] = value;
    }
    void addLoadVector(double value, unsigned i)
    {
        globalLoadVector[i] += value;
    }
    virtual double getStiffnessMatrix(unsigned i, unsigned j) = 0;
    virtual double getMassMatrix(unsigned, unsigned) = 0;
    virtual double getDampingMatrix(unsigned, unsigned) = 0;
    double getLoadVector(unsigned i)
    {
        return globalLoadVector[i];
    }
    virtual bool solve(vector<double>&, double, bool&) = 0;
    virtual void print(string) = 0;
    bool saveStiffnessMatrix(string fname)
    {
        return saveMatrix(fname, globalStiffnessMatrix);
    }
    bool loadStiffnessMatrix(string fname)
    {
        return loadMatrix(fname, globalStiffnessMatrix);
    }
    bool saveMassMatrix(string fname)
    {
        return saveMatrix(fname, globalMassMatrix);
    }
    bool loadMassMatrix(string fname)
    {
        return loadMatrix(fname, globalMassMatrix);
    }
    bool saveDampingMatrix(string fname)
    {
        return saveMatrix(fname, globalDampingMatrix);
    }
    bool loadDampingMatrix(string fname)
    {
        return loadMatrix(fname, globalDampingMatrix);
    }
};

#endif // SOLVER_H
