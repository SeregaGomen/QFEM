#include <math.h>
#include "lzhsolver.h"
#include "mesh/mesh.h"
#include "msg/msg.h"


extern TMessenger *msg;


double TLZHSolver::scalar_product(VectorXd &lhs, VectorXd &rhs)
{
    double ret = 0;

    for (auto i = 0; i < lhs.size(); i++)
        ret += lhs[i] * rhs[i];
    return ret;
}

bool TLZHSolver::solve(vector<double> &res, double eps, bool &isAborted)
{
    unsigned size = stiffnessMatrix.size(),
             i,
             maxIter = size;
    double norm,
           new_norm,
           err,
           a,
           b;
    VectorXd x = Map<VectorXd, Unaligned>(loadVector.data(), unsigned(loadVector.size())),
             s(size),
             xp(size),
             r(size),
             R = x;
    bool is_ok = false;

    r = stiffnessMatrix * x;
    r -= R;
    s = r;

    norm = scalar_product(r, r);
    msg->setProcess(ProcessCode::SolutionSystemEquation);
    for (i = 0; i < maxIter; i++)
    {
        if (isAborted)
            return false;

        R = stiffnessMatrix * s;
        err = scalar_product(R, s);
        if (fabs(err) < eps)
        {
            is_ok = true;
            break;
        }
        a = norm / err;
        r -= a * R;
        x -= a * s;
        new_norm = scalar_product(r, r);
        if (norm < eps)
        {
            is_ok = true;
            break;
        }
        b = new_norm / norm;
        norm = new_norm;
        s *= b;
        s += r;
        xp = x;
    }
    msg->stop();
    if (is_ok)
    {
        res.resize(stiffnessMatrix.rows());
        copy(&x[0], x.data() + x.cols() * x.rows(), res.begin());
    }
    return is_ok;
}

