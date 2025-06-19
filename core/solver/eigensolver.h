#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <Eigen/Sparse>
#include "solver.h"

using namespace Eigen;
using namespace std;

class TMesh;

class TEigenSolver : public TSolver<SparseMatrix<double>, VectorXd>
{
protected:
    bool solve(vector<double>&, double, bool&);
public:
    TEigenSolver(void) = default;
    virtual ~TEigenSolver(void) = default;
    void setMatrix(TMesh*, bool = false);
    void setElement(SparseMatrix<double> &m, unsigned i, unsigned j, double value)
    {
        m.coeffRef(i, j) = value;
    }
    void addElement(SparseMatrix<double> &m, unsigned i, unsigned j, double value)
    {
        m.coeffRef(i, j) += value;
    }
    double getElement(SparseMatrix<double> &m, unsigned i, unsigned j)
    {
        return m.coeff(i, j);
    }
};

#endif // EIGENSOLVER_H
