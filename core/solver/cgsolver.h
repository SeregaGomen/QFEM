#ifndef CGSOLVER_H
#define CGSOLVER_H

#include <string>
#include <boost/numeric/ublas/matrix_sparse.hpp>
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/lock_guard.hpp>
#include "solver.h"

using namespace std;

class TMesh;

using BoostSparseMatrix = boost::numeric::ublas::compressed_matrix<double, boost::numeric::ublas::row_major>;
using BoostVector = boost::numeric::ublas::vector<double>;

class TCGSolver : public TSolver<BoostSparseMatrix>
{
private:
//    boost::mutex mtx_;
    void residual(const BoostSparseMatrix&, const BoostVector&, const BoostVector&, BoostVector&);
    void sps_prod(const BoostSparseMatrix&, const BoostVector&, BoostVector&);
protected:
    bool loadMatrix(string, BoostSparseMatrix&);
    bool saveMatrix(string, BoostSparseMatrix&);
public:
    TCGSolver(void) {}
    virtual ~TCGSolver(void)
    {
        clear();
    }
    void clear(void)
    {
        loadVector.clear();
        stiffnessMatrix.clear();
        massMatrix.clear();
        dampingMatrix.clear();
    }
    void setBoundaryCondition(unsigned, double);
    void setMatrix(TMesh*, bool = false);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffnessMatrix(i, j) = value;
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        massMatrix(i, j) = value;
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        dampingMatrix(i, j) = value;
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
//        boost::lock_guard<boost::mutex> guard(mtx_);
        lock_guard<mutex> guard(mtx);
        stiffnessMatrix(i, j) += value;
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        massMatrix(i, j) += value;
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        dampingMatrix(i, j) += value;
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffnessMatrix(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return massMatrix(i, j);
    }
    double getDamping(unsigned i,unsigned j)
    {
        return dampingMatrix(i, j);
    }
    bool solve(vector<double>&, double, bool&);
    void print(string);
    void product(BoostSparseMatrix&, vector<double>&, vector<double>&);
};

#endif // CGSOLVER_H
