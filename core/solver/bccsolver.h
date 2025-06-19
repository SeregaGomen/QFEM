#ifndef BCCSOLVER_H
#define BCCSOLVER_H

#include "solver.h"
#include "sparse/sparse_bccs.h"

using namespace std;

class TMesh;

//---------------------------------------------------------------
//  Класс, реализующий решение разреженной СЛАУ прямым методом
//---------------------------------------------------------------
class TBCCSolver : public TSolver<TBCCSMatrix, vector<double>>
{
protected:
    bool solve(vector<double>&, double, bool&);
public:
    TBCCSolver(void) = default;
    virtual ~TBCCSolver(void) = default;
    void setMatrix(TMesh*, bool = false);
    void setElement(TBCCSMatrix &m, unsigned i, unsigned j, double value)
    {
        spSetElem(m, i, j, value);
    }
    void addElement(TBCCSMatrix &m, unsigned i, unsigned j, double value)
    {
        spAddElem(m, i, j, value);
    }
    double getElement(TBCCSMatrix &m, unsigned i, unsigned j)
    {
        return spGetElem(m, i, j);
    }
};

#endif // BCCSOLVER_H
