#ifndef TPARDISOSOLVER_H
#define TPARDISOSOLVER_H

#include "solver/solver.h"
#include "sparse/sparse_csr.h"

using namespace std;

class TMesh;

//-----------------------------------------------------------------------
//  Класс, реализующий решение СЛАУ с использованием библиотеки PARDISO
//-----------------------------------------------------------------------
class TPardisoSolver : public TSolver<TCSRMatrix, vector<double>>
{
public:
    TPardisoSolver() = default;
    ~TPardisoSolver() = default;
    void setMatrix(TMesh*, bool = false);
    void setBoundaryCondition(unsigned, double);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffness.setElem(i, j, value);
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        damping.setElem(i, j, value);
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        mass.setElem(i, j, value);
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        stiffness.addElem(i, j, value);
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        mass.addElem(i, j, value);
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        damping.addElem(i, j, value);
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffness.getElem(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return mass.getElem(i, j);
    }
    double getDamping(unsigned i, unsigned j)
    {
        return damping.getElem(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // TPARDISOSOLVER_H
