#ifndef BCCSOLVER_H
#define BCCSOLVER_H

#include <string>
#include "solver.h"
#include "sparse/sparse_bccs.h"
#include "util/matrix.h"

using namespace std;

class TMesh;

//---------------------------------------------------------------
//  Класс, реализующий решение СЛАУ методом градиентного спуска
//---------------------------------------------------------------
class TBCCSolver : public TSolver<BCCS_Matrix, vector<double>>
{
public:
    TBCCSolver(void) = default;
    virtual ~TBCCSolver(void) = default;
    void setBoundaryCondition(unsigned, double);
    void setMatrix(TMesh*, bool = false);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        spSetElem(stiffness, i, j, value);
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        spSetElem(mass, i, j, value);
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        spSetElem(damping, i, j, value);
    }
    void addStiffness(double value ,unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        spAddElem(stiffness, i, j, value);
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        spAddElem(mass, i, j, value);
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        spAddElem(damping, i, j, value);
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return spGetElem(stiffness, i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return spGetElem(mass, i, j);
    }
    double getDamping(unsigned i,unsigned j)
    {
        return spGetElem(damping, i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // BCCSOLVER_H
