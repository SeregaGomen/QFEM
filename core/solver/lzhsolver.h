#ifndef TLZHSOLVER_H
#define TLZHSOLVER_H

#include <Eigen/Dense>
#include "solver/eigensolver.h"


class TLZHSolver : public TEigenSolver
{
private:
    double scalar_product(VectorXd&, VectorXd&);
public:
    TLZHSolver(void) : TEigenSolver() {}
    virtual ~TLZHSolver(void) = default;
    bool solve(std::vector<double>&, double, bool&);
};

#endif // TLZHSOLVER_H
