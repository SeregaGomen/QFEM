#include <cmath>
#include "sparse_env.h"
#include "../msg/msg.h"

extern TMessenger *msg;

// Решение нижней треугольной системы L*X = RHS
// Матрица L хранится в профильном формате
//
// neqns - количество уравнений
// diag - диагональные элементы матрицы
// env - профильные элементы
// xenv - индексы ...
// rhs - вектор правой части
void elslv(unsigned neqns, double *diag, double *env, unsigned *xenv, double *rhs, bool &aborted)
{
    auto ifirst = 0u,
         last = 0u;

    // Найти номер первого ненулевого элемента в правой части и поместить его в ifirst
    for (auto i = 0u; i < neqns; i++)
        if (rhs[i] != 0.)
        {
            ifirst = i;
            break;
        }
    // last содержит номер последней вычисленной ненулевой компоненты решения
    for (auto i = ifirst; i < neqns; i++)
    {
        if (aborted)
            break;
        auto iband = xenv[i + 1] - xenv[i];
        if (iband > i)
            iband = i;
        auto s = rhs[i];
        auto l = i - iband;
        // Строка оболочки пуста или все соответствующие компоненты решения - нули
        if (!(iband == 0 || last < l))
        {
            auto kstrt = xenv[i + 1] - iband;
            auto kstop = xenv[i + 1];
            for (auto k = kstrt; k < kstop; k++)
            {
                s -= env[k] * rhs[l];
                l += 1;
            }
        }
        if (s != 0.)
        {
            rhs[i] = s / diag[i];
            last = i;
        }
    }
}

// Решение верхней треугольной системы профильным методом
void euslv(unsigned neqns, vector<double> &diag, vector<double> &env, vector<unsigned> &xenv, vector<double> &rhs, bool &aborted)
{
    for (int i = (int)neqns - 1; i >= 0; i--)
    {
        if (aborted)
            break;
        if (rhs[i] != 0.)
        {
            auto s = rhs[i] / diag[i];
            rhs[i] = s;
            auto iband = xenv[i + 1] - xenv[i];
            if (iband > (unsigned)i)
                iband -= 1;
            if (iband == 0)
                continue;
            auto kstrt = i - iband;
            auto kstop = i;
            auto l = xenv[i + 1] - iband;
            for (int k = kstrt; k < kstop; k++)
            {
                rhs[k] -= s * env[l];
                l += 1;
            }
        }
    }
}


// Симметричное разложение профильным методом
bool TEnvMatrix::esfct(bool &aborted)
{
    if (diag[0] < 0.)
        return false;

    diag[0] = sqrt(diag[0]);
    if (nvtxs == 1)
        return true;

    msg->setProcess(ProcessCode::FactorizationSystemEquation, 0, nvtxs * blksze - 1, 10);
    for (auto i = 1u; i < nvtxs * blksze; msg->addProgress(), i++)
    {
        if (aborted)
            return false;
        auto ixenv = xenv[i];
        auto iband = xenv[i + 1] - ixenv;
        auto temp = diag[i];
        if (iband != 0)
        {
            auto ifirst = i - iband;
            // Вычислить строку i треугольного множителя
            elslv(iband, &diag[ifirst], &env[0], &xenv[ifirst], &env[ixenv], aborted);
            auto jstop = xenv[i + 1];
            for (auto j = ixenv; j < jstop; j++)
            {
                auto s = env[j];
                temp -= s * s;
            }
        }
        if (temp <= 0.)
        {
            // Матрица не положительно определенная
            return false;
        }
        diag[i] = sqrt(temp);
    }
    msg->stopProcess();
    return true;
}

bool TEnvMatrix::solve(vector<double> &rhs, bool &aborted)
{
    if (!esfct(aborted))
        return false;
    msg->setProcess(ProcessCode::SolutionSystemEquation);
    elslv(size(), &diag[0], &env[0], &xenv[0], &rhs[0], aborted);
    if (aborted)
        return false;
    euslv(size(), diag, env, xenv, rhs, aborted);
    if (aborted)
        return false;
    msg->stop();
    return true;
}
