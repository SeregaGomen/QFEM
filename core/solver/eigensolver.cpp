#include <ctime>
#include <Eigen/PardisoSupport>
#include <Eigen/SparseCholesky>
#include "mesh/mesh.h"
#include "solver/eigensolver.h"
#include "msg/msg.h"

extern TMessenger* msg;

bool TEigenSolver::solve(vector<double> &r, double, bool&)
{
    PardisoLLT<SparseMatrix<double>> solver;
//    SimplicialLLT<SparseMatrix<double>> solver;
//    ConjugateGradient<SparseMatrix<double>, Eigen::Upper> solver;
//    BiCGSTAB<SparseMatrix<double>> solver;
    VectorXd x,
             right = Map<VectorXd, Unaligned>(loadVector.data(), unsigned(loadVector.size()));

//    cerr << globalStiffnessMatrix.nonZeros() << endl;
    /////////////
    // print("matr1.txt");
    ///

    msg->setProcess(ProcessCode::PreparingSystemEquation);
    solver.compute(stiffnessMatrix);
    msg->stop();
    if (solver.info() not_eq Success)
        throw ErrorCode::EEquationNorSolved;

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    x = solver.solve(right);
    msg->stop();

    if (solver.info() not_eq Success)
        throw ErrorCode::EEquationNorSolved;

    r.resize(stiffnessMatrix.rows());
    copy(&x[0], x.data() + x.cols() * x.rows(), r.begin());
//    for (unsigned i = 0; i < r.size(); i++)
//        r[i] = x(i);

    return true;
}

void TEigenSolver::setMatrix(TMesh* mesh, bool isDynamic)
{
    unsigned size = mesh->getNumVertex(),
             freedom = mesh->getFreedom();

    // Резервируем объем необходимой памяти
    memMap.resize(size * freedom);
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
            memMap[i * freedom + j] = int(mesh->getMeshMap(i).size() * freedom * freedom);

    stiffnessMatrix.resize(size * freedom, size * freedom);
    stiffnessMatrix.setZero();
    stiffnessMatrix.reserve(memMap);
    if (isDynamic)
    {
        massMatrix.resize(size * freedom, size * freedom);
        massMatrix.setZero();
        massMatrix.reserve(memMap);
        dampingMatrix.resize(size * freedom, size * freedom);
        dampingMatrix.setZero();
        dampingMatrix.reserve(memMap);
    }
    loadVector.resize(size * freedom, 0);
    memMap.resize(0);
}

void TEigenSolver::setBoundaryCondition(unsigned index, double value)
{
    for (Eigen::SparseMatrix<double>::InnerIterator i(stiffnessMatrix, index); i; ++i)
    {
        if (i.row() not_eq i.col())
        {
            stiffnessMatrix.coeffRef(i.row(), i.col()) = value;
            stiffnessMatrix.coeffRef(i.col(), i.row()) = value;
        }
    }
    loadVector[index] = value * stiffnessMatrix.coeffRef(index, index);
//    stiffnessMatrix.coeffRef(index, index) *= 1.0E+8;
//    loadVector[index] *= 1.0E+8 * value;
}

void TEigenSolver::print(string fname)
{
    unsigned sz = unsigned(loadVector.size());
    double res;
    fstream out(fname, ios::out);

    out << sz << 'x' << sz + 1 << endl;
    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (unsigned i = 0; i < sz; i++)
    {
        for (unsigned j = 0; j < sz; j++)
        {
            res = stiffnessMatrix.coeff(i,j);
            out.precision(10);
            out.width(20);
            out << res << ' ';
        }
        out.precision(10);
        out.width(20);
        out << loadVector[i] << endl;
    }
    out.close();
}

bool TEigenSolver::saveMatrix(string fname, SparseMatrix<double> &globalMatrix)
{
    fstream out(fname, ios::out | ios::binary);
    size_t size = globalMatrix.cols(),
           nnz = globalMatrix.nonZeros();
    double val;

    if (not out.is_open())
        return false;
    // Запись размерности матрицы и кол-ва ненулевых членов
    out.write((char*)(&size), sizeof(size_t));
    out.write((char*)(&nnz), sizeof(size_t));
    // Запись ненулевых элементов
    for (int k = 0; k < globalMatrix.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(globalMatrix,k); it; ++it)
        {
            out.write((char*)&(nnz = int(it.row())), sizeof(size_t));
            out.write((char*)&(nnz = int(it.col())), sizeof(size_t));
            out.write((char*)&(val = int(it.value())), sizeof(double));
        }
    out.close();
    return not out.fail();
}

bool TEigenSolver::loadMatrix(string fname, SparseMatrix<double>& globalMatrix)
{
    size_t size,
           nnz,
           row,
           col;
    double val;
    fstream in(fname, ios::in | ios::binary);

    if (not in.is_open())
        return false;

    in.read((char*)(&size), sizeof(size_t));
    in.read((char*)(&nnz), sizeof(size_t));
    // Резервируем объем необходимой памяти
    globalMatrix.resize(size, size);
    globalMatrix.setZero();
    globalMatrix.reserve(nnz);

    for (auto i = 0u; i < nnz; i++)
    {
        in.read((char*)&row, sizeof(size_t));
        in.read((char*)&col, sizeof(size_t));
        in.read((char*)&val, sizeof(double));
        globalMatrix.coeffRef(row, col) = val;
    }
    in.close();
    return not in.fail();
}

void TEigenSolver::product(SparseMatrix<double>& matr, vector<double>& vec, vector<double>& res)
{
    VectorXd tmp = Map<VectorXd, Unaligned>(vec.data(), unsigned(vec.size()));

    tmp = matr * tmp;
    res.resize(tmp.size());
    VectorXd::Map(&res[0], tmp.size()) = tmp;
}
