#include <cstring>
#include <string>
#include "cgsolver.h"
#include "msg/msg.h"
#include "mesh/mesh.h"

using namespace std;

extern TMessenger* msg;

void TCGSolver::residual(const BoostSparseMatrix &A, const BoostVector &x, const BoostVector &b, BoostVector &r)
{
    for (auto i = 0u; i < A.size1(); ++ i)
    {
        auto begin = A.index1_data()[i],
             end = A.index1_data()[i + 1];
        auto t(b(i));

        for (auto j = begin; j < end; ++ j)
            t -= A.value_data()[j] * x(A.index2_data()[j]);
        r(i) = t;
    }
}

void TCGSolver::sps_prod(const BoostSparseMatrix &A, const BoostVector &x, BoostVector &r)
{
    for (auto i = 0u; i < A.size1(); ++ i)
    {
        auto begin = A.index1_data()[i],
             end = A.index1_data()[i + 1];
        auto t(0.0);

        for (auto j = begin; j < end; ++ j)
            t += A.value_data()[j] * x(A.index2_data()[j]);
        r(i) = t;
    }
}

bool TCGSolver::solve(vector<double> &result, double eps, bool &isAborted)
{
    size_t niter = 10 * load.size();
    bool is_ok = false;
    double alpha, beta, residn;
    BoostVector resid(load.size()),
                d,            // search direction
                resid_old,
                temp(load.size()),
                b(load.size()),
                x;
    BoostSparseMatrix BSM(stiffness);

    copy(load.begin(), load.end(), b.begin());
    x = b;

    residual(BSM, x, b, resid);

    d = resid;
    // CG loop

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    for (auto i = 1u; i <= niter; i++)
    {
        if (isAborted)
            break;
        sps_prod(BSM, d, temp);
        alpha = inner_prod(resid, resid) / inner_prod(d, temp);
        x += (d*alpha);
        resid_old = resid;
        resid -= (temp * alpha);
        residn = norm_2(resid);
        if(residn <= eps)
        {
            is_ok = true;
            break;
        }
        beta = inner_prod(resid, resid) / inner_prod(resid_old, resid_old);
        d = resid + d*beta;
    }
    msg->stop();
    if (is_ok)
    {
        result.resize(x.size());
        copy(x.begin(), x.end(), result.begin());
    }
    return is_ok;
}

void TCGSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    size_t globalSize = mesh->getNumVertex() * mesh->getFreedom();

    stiffness.resize(globalSize, globalSize);
    if (isDynamic)
    {
        mass.resize(globalSize, globalSize);
        damping.resize(globalSize, globalSize);
    }
    load.resize(mesh->getNumVertex() * mesh->getFreedom());
}

void TCGSolver::setBoundaryCondition(unsigned index, double value)
{
    for (auto i = 0u; i < stiffness.size1(); i++)
        if (i not_eq index)
            if (stiffness(index, i) not_eq 0)
                stiffness(index, i) = stiffness(i, index) = value;
    load[index] = value * stiffness(index, index);
}
