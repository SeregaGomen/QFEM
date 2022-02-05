#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <Eigen/Sparse>
#include "solver.h"

using namespace Eigen;
using namespace std;

class TMesh;

class TEigenSolver : public TSolver<SparseMatrix<double>, VectorXd>
{
private:
    VectorXi memMap;
public:
    TEigenSolver(void) = default;
    virtual ~TEigenSolver(void) = default;
    void setMatrix(TMesh*, bool = false);
    void setBoundaryCondition(unsigned, double);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffness.coeffRef(i, j) = value;
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        damping.coeffRef(i, j) = value;
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        mass.coeffRef(i, j) = value;
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        stiffness.coeffRef(i, j) += value;
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        mass.coeffRef(i, j) += value;
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        damping.coeffRef(i, j) += value;
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffness.coeff(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return mass.coeff(i, j);
    }
    double getDamping(unsigned i, unsigned j)
    {
        return damping.coeff(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // EIGENSOLVER_H
