#include <cstring>
#include <string>
#include <iomanip>
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
    size_t niter = 10 * loadVector.size();
    bool is_ok = false;
    double alpha, beta, residn;
    BoostVector resid(loadVector.size()),
                d,            // search direction
                resid_old,
                temp(loadVector.size()),
                b(loadVector.size()),
                x;
    BoostSparseMatrix BSM(stiffnessMatrix);

    copy(loadVector.begin(), loadVector.end(), b.begin());
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

    stiffnessMatrix.resize(globalSize, globalSize);
    if (isDynamic)
    {
        massMatrix.resize(globalSize, globalSize);
        dampingMatrix.resize(globalSize, globalSize);
    }
    loadVector.resize(mesh->getNumVertex() * mesh->getFreedom());
}

void TCGSolver::print(string fname)
{
    size_t size = stiffnessMatrix.size1();
    fstream out;

    out.open(fname.c_str(), ios::out);
    out << size << 'x' << size + 1 << endl;

    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (auto i = 0u; i < size; i++)
    {
        for (auto j = 0u; j < size; j++)
            out << setw(20) << setprecision(5) << stiffnessMatrix(i, j) << ' ';
        out << setw(20) << setprecision(5) << loadVector[i] << endl;
    }
    out.close();
}

void TCGSolver::setBoundaryCondition(unsigned index, double value)
{
    for (auto i = 0u; i < stiffnessMatrix.size1(); i++)
        if (i not_eq index)
            if (stiffnessMatrix(index, i) not_eq 0)
                stiffnessMatrix(index, i) = stiffnessMatrix(i, index) = value;
    loadVector[index] = value * stiffnessMatrix(index, index);
}

bool TCGSolver::saveMatrix(string fname, VectorOfVector &globalMatrix)
{
    fstream out(fname, ios::out | ios::binary);
    size_t size = globalMatrix.size1(),
           nnz = globalMatrix.nnz();
    double val;

    if (not out.is_open())
        return false;
    // Запись размерности матрицы
    out.write((const char*)&size, sizeof(size_t));
    // Запись количества ненулевых эоементов
    out.write((const char*)&nnz, sizeof(size_t));
    // Запись ненулевых элементов
    for(auto rowIter = globalMatrix.begin1(); rowIter != globalMatrix.end1(); ++rowIter)
        for(auto colIter = rowIter.begin(); colIter != rowIter.end(); ++colIter)
        {
            out.write((const char*)&(nnz = colIter.index1()), sizeof(size_t));
            out.write((const char*)&(nnz = colIter.index2()), sizeof(size_t));
            out.write((const char*)&(val = *colIter), sizeof(double));
        }
    out.close();
    return not out.fail();
}

bool TCGSolver::loadMatrix(string fname, VectorOfVector &globalMatrix)
{
    size_t nnz,
           row,
           col,
           size;
    double val;
    fstream in(fname, ios::in | ios::binary);

    if (not in.is_open())
        return false;

    in.read((char*)&size, sizeof(size_t));
    in.read((char*)&nnz, sizeof(size_t));
    globalMatrix.resize(size, size);
    for (auto i = 0u; i < nnz; i++)
    {
        in.read((char*)&row, sizeof(size_t));
        in.read((char*)&col, sizeof(size_t));
        in.read((char*)&val, sizeof(double));
//        globalMatrix.insert_element(row, col, val);
        globalMatrix(row, col) = val;
    }
    in.close();
    return not in.fail();
}

void TCGSolver::product(VectorOfVector &matr, vector<double> &vec, vector<double> &res)
{
    BoostVector lhs(vec.size()),
           rhs(vec.size());

    copy(vec.begin(), vec.end(), rhs.begin());
    lhs = prod(matr, rhs);
    copy(lhs.begin(), lhs.end(), res.begin());
}
