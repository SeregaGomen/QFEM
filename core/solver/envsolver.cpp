#include "envsolver.h"

bool TEnvSolver::solve(vector<double> &result, double /* eps */, bool &isAborted)
{
    if (!stiffness.solve(load, isAborted))
        return false;
    result = load;
    return true;
}

