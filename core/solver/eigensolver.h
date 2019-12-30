#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <Eigen/Sparse>
#include "solver.h"

using namespace Eigen;
using namespace std;

class TMesh;

class TEigenSolver : public TSolver< SparseMatrix<double> >
{
private:
    unsigned size;
    unsigned freedom;
    VectorXi memMap;
    void createMemMap(TMesh*);
    bool loadMatrix(string, SparseMatrix<double>&);
    bool saveMatrix(string, SparseMatrix<double>&);
public:
    TEigenSolver(void)
    {
        freedom = size = 0;
    }
    ~TEigenSolver(void) {}
    void createDynamicMatrix(double, double);
    void createDynamicVector(matrix<double>&, double, double);
    void setupStaticMatrix(TMesh*);
    void setupDynamicMatrix(TMesh*);
    void setBoundaryCondition(unsigned, unsigned, double);
    void clear(void)
    {
        freedom = size = 0;
        globalStiffnessMatrix.setZero();
        globalMassMatrix.setZero();
        globalDampingMatrix.setZero();
        globalLoadVector.clear();
        memMap.setZero();
    }
    void setStiffnessMatrix(double value, unsigned i, unsigned j)
    {
        globalStiffnessMatrix.coeffRef(i, j) = value;
    }
    void setDampingMatrix(double value, unsigned i, unsigned j)
    {
        globalDampingMatrix.coeffRef(i, j) = value;
    }
    void setMassMatrix(double value, unsigned i, unsigned j)
    {
        globalMassMatrix.coeffRef(i, j) = value;
    }
    void addStiffnessMatrix(double value, unsigned i, unsigned j)
    {
        globalStiffnessMatrix.coeffRef(i, j) += value;
    }
    void addMassMatrix(double value, unsigned i, unsigned j)
    {
        globalMassMatrix.coeffRef(i, j) += value;
    }
    void addDampingMatrix(double value, unsigned i, unsigned j)
    {
        globalDampingMatrix.coeffRef(i, j) += value;
    }
    void print(string);
    double getStiffnessMatrix(unsigned i, unsigned j)
    {
        return globalStiffnessMatrix.coeff(i, j);
    }
    double getMassMatrix(unsigned i,unsigned j)
    {
        return globalMassMatrix.coeff(i, j);
    }
    double getDampingMatrix(unsigned i,unsigned j)
    {
        return globalDampingMatrix.coeff(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // EIGENSOLVER_H
