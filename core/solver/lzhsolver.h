#ifndef TLZHSOLVER_H
#define TLZHSOLVER_H

#include <Eigen/Dense>
#include "solver/eigensolver.h"


class TLZHSolver : public TEigenSolver
{
private:
    double scalar_product(VectorXd&, VectorXd&);
protected:
    bool solve(vector<double>&, double, bool&);
public:
    TLZHSolver(void) : TEigenSolver() {}
    virtual ~TLZHSolver(void) = default;
};

#endif // TLZHSOLVER_H
