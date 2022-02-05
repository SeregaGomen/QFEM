#ifndef CGSOLVER_H
#define CGSOLVER_H

#include <string>
#include <boost/numeric/ublas/vector_of_vector.hpp>
//#include <boost/numeric/ublas/matrix_sparse.hpp>
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/lock_guard.hpp>
#include "solver.h"

using namespace std;

class TMesh;

using BoostSparseMatrix = boost::numeric::ublas::compressed_matrix<double, boost::numeric::ublas::row_major>;
using BoostVector = boost::numeric::ublas::vector<double>;
using VectorOfVector = boost::numeric::ublas::generalized_vector_of_vector<double, boost::numeric::ublas::row_major, boost::numeric::ublas::vector<boost::numeric::ublas::compressed_vector<double>>>;

class TCGSolver : public TSolver<VectorOfVector, BoostVector>
{
private:
    void residual(const BoostSparseMatrix&, const BoostVector&, const BoostVector&, BoostVector&);
    void sps_prod(const BoostSparseMatrix&, const BoostVector&, BoostVector&);
public:
    TCGSolver(void) = default;
    virtual ~TCGSolver(void) = default;
    void setBoundaryCondition(unsigned, double);
    void setMatrix(TMesh*, bool = false);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffness(i, j) = value;
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        mass(i, j) = value;
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        damping(i, j) = value;
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
//        boost::lock_guard<boost::mutex> guard(mtx_);
        lock_guard<mutex> guard(mtx);
        stiffness(i, j) += value;
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        mass(i, j) += value;
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        damping(i, j) += value;
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffness(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return mass(i, j);
    }
    double getDamping(unsigned i,unsigned j)
    {
        return damping(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // CGSOLVER_H
