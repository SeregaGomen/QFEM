#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <mutex>
#include <Eigen/Sparse>
#include "solver.h"

using namespace Eigen;
using namespace std;

class TMesh;

class TEigenSolver : public TSolver< SparseMatrix<double> >
{
private:
    VectorXi memMap;
    mutex mtx;
    bool loadMatrix(string, SparseMatrix<double>&);
    bool saveMatrix(string, SparseMatrix<double>&);
public:
    TEigenSolver(void) {}
    virtual ~TEigenSolver(void) {}
    void setMatrix(TMesh*, bool = false);
    void setBoundaryCondition(unsigned, double);
    void clear(void)
    {
        stiffnessMatrix.resize(0, 0);
        massMatrix.resize(0, 0);
        dampingMatrix.resize(0, 0);
        memMap.resize(0);
        loadVector.clear();
    }
    void product(SparseMatrix<double>&, vector<double>&, vector<double>&);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffnessMatrix.coeffRef(i, j) = value;
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        dampingMatrix.coeffRef(i, j) = value;
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        massMatrix.coeffRef(i, j) = value;
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        stiffnessMatrix.coeffRef(i, j) += value;
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        massMatrix.coeffRef(i, j) += value;
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        dampingMatrix.coeffRef(i, j) += value;
    }
    void print(string);
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffnessMatrix.coeff(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return massMatrix.coeff(i, j);
    }
    double getDamping(unsigned i, unsigned j)
    {
        return dampingMatrix.coeff(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // EIGENSOLVER_H
