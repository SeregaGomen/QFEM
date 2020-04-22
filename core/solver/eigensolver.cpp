#include <ctime>
#include <Eigen/PardisoSupport>
#include <Eigen/SparseCholesky>
#include "mesh/mesh.h"
#include "solver/eigensolver.h"
#include "msg/msg.h"

extern TMessenger* msg;

bool TEigenSolver::solve(vector<double>& r, double, bool&)
{
    PardisoLLT< SparseMatrix<double> > solver;
//    SimplicialLLT< SparseMatrix<double> > solver;
    VectorXd x,
             load = Map<VectorXd, Unaligned>(loadVector.data(), unsigned(loadVector.size()));

//    cerr << globalStiffnessMatrix.nonZeros() << endl;
    /////////////
    // print("matr1.txt");
    ///

    msg->setProcess(SYSTEM_PREPARE_PROCESS);
    solver.compute(stiffnessMatrix);
    msg->stop();
    if (solver.info() != Success)
        throw EQUATION_NOT_SOLVED_ERR;

    msg->setProcess(SYSTEM_SOLUTION_PROCESS);
    x = solver.solve(load);
    msg->stop();

    if(solver.info() != Success)
        throw EQUATION_NOT_SOLVED_ERR;

    r.resize(unsigned(stiffnessMatrix.rows()));
    for (unsigned i = 0; i < r.size(); i++)
        r[i] = x(i);
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
        if (i.row() != i.col())
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

bool TEigenSolver::saveMatrix(string fname, SparseMatrix<double>& globalMatrix)
{
    fstream out(fname, ios::out | ios::binary);
    int signature = 12031971,
        len = 0;
    double val;

    if (out.fail())
        return false;
    // Запись сигнатуры
    out.write(reinterpret_cast<char*>(&signature), sizeof(int));
    // Вычисление количества ненулевых элементов
    for (int k = 0; k < globalMatrix.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(globalMatrix,k); it; ++it)
            len++;
    out.write(reinterpret_cast<char*>(&len), sizeof(int));
    // Запись ненулевых элементов
    for (int k = 0; k < globalMatrix.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(globalMatrix,k); it; ++it)
        {
            out.write(reinterpret_cast<char*>(&(len = int(it.row()))), sizeof(int));
            out.write(reinterpret_cast<char*>(&(len = int(it.col()))), sizeof(int));
            out.write(reinterpret_cast<char*>(&(val = int(it.value()))), sizeof(double));
        }
    out.close();
    return !out.fail();
}

bool TEigenSolver::loadMatrix(string fname, SparseMatrix<double>& globalMatrix)
{
    int len,
        signature,
        row,
        col;
    double val;
    fstream in(fname, ios::in | ios::binary);

    globalMatrix.setZero();

    // Резервируем объем необходимой памяти
    globalMatrix.reserve(memMap);


    if (in.fail())
        return false;
    in.read(reinterpret_cast<char*>(&signature), sizeof(int));
    if (signature != 12031971)
    {
        in.close();
        return false;
    }
    in.read(reinterpret_cast<char*>(&len), sizeof(int));


    for (int i = 0; i < len; i++)
    {
        in.read(reinterpret_cast<char*>(&row), sizeof(int));
        in.read(reinterpret_cast<char*>(&col), sizeof(int));
        in.read(reinterpret_cast<char*>(&val), sizeof(double));
        if (in.good())
            globalMatrix.coeffRef(row,col) = val;
        else
            break;
    }
    in.close();
    return !in.fail();
}

void TEigenSolver::product(SparseMatrix<double>& matr, vector<double>& vec, vector<double>& res)
{
    VectorXd tmp = Map<VectorXd, Unaligned>(vec.data(), unsigned(vec.size()));

    tmp = matr * tmp;
    res.resize(tmp.size());
    VectorXd::Map(&res[0], tmp.size()) = tmp;
}
