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
protected:
    bool solve(vector<double>&, double, bool&);
public:
    TPardisoSolver() = default;
    ~TPardisoSolver() = default;
    void setMatrix(TMesh*, bool = false);
    void setElement(TCSRMatrix &m, unsigned i, unsigned j, double value)
    {
        m.setElem(i, j, value);
    }
    void addElement(TCSRMatrix &m, unsigned i, unsigned j, double value)
    {
        m.addElem(i, j, value);
    }
    double getElement(TCSRMatrix &m, unsigned i, unsigned j)
    {
        return m.getElem(i, j);
    }
};

#endif // TPARDISOSOLVER_H
