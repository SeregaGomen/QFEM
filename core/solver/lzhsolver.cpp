#include <cmath>
#include "lzhsolver.h"
//#include "mesh/mesh.h"
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
    unsigned size = (unsigned)stiffness.rows(),
             i,
             maxIter = 5 * size;
    double norm,
           new_norm,
           err,
           a;
    VectorXd x = Map<VectorXd, Unaligned>(load.data(), unsigned(load.size())),
             s,
             r0,
             r;
    bool is_ok = false;

    r0 = stiffness * x - x;
    s = r0;

    norm = scalar_product(r0, r0);
    msg->setProcess(ProcessCode::SolutionSystemEquation);
    for (i = 0; i < maxIter; i++)
    {
        if (isAborted)
            break;
        if (norm < eps)
        {
            is_ok = true;
            break;
        }
        r = stiffness * s;
        err = scalar_product(r, s);
        if (fabs(err) < eps)
        {
            is_ok = true;
            break;
        }
        a = norm / err;
        r0 -= a * r;
        x -= a * s;
        new_norm = scalar_product(r0, r0);
        a = new_norm / norm;
        norm = new_norm;
        s = s * a + r0;
    }
    msg->stop();
    if (is_ok)
    {
        res.resize(stiffness.rows());
        copy(&x[0], x.data() + x.cols() * x.rows(), res.begin());
    }
    return is_ok;
}

