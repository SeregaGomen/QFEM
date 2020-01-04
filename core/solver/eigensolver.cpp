#include <ctime>
#include <Eigen/PardisoSupport>
#include <Eigen/SparseCholesky>
#include "mesh/mesh.h"
#include "solver/eigensolver.h"
#include "msg/msg.h"

extern TMessenger* msg;

bool TEigenSolver::solve(std::vector<double>& r, double, bool&)
{
    PardisoLLT< SparseMatrix<double> > solver;
//    SimplicialLLT< SparseMatrix<double> > solver;
    VectorXd x,
             load = Map<VectorXd, Unaligned>(globalLoadVector.data(), unsigned(globalLoadVector.size()));

//    cerr << globalStiffnessMatrix.nonZeros() << endl;
    /////////////
    // print("matr1.txt");
    ///

    msg->setProcess(SYSTEM_PREPARE_PROCESS);
    solver.compute(globalStiffnessMatrix);
    msg->stop();
    if (solver.info() != Success)
        throw EQUATION_NOT_SOLVED_ERR;

    msg->setProcess(SYSTEM_SOLUTION_PROCESS);
    x = solver.solve(load);
    msg->stop();

    if(solver.info() != Success)
        throw EQUATION_NOT_SOLVED_ERR;

    r.resize(unsigned(globalStiffnessMatrix.rows()));
    for (unsigned i = 0; i < r.size(); i++)
        r[i] = x(i);
    return true;
}

void TEigenSolver::setupStaticMatrix(TMesh* mesh)
{
    unsigned globalSize;

    size = mesh->getNumVertex();
    freedom = mesh->getFreedom();
    globalSize = size * freedom;

    globalStiffnessMatrix.resize(globalSize, globalSize);
    globalStiffnessMatrix.setZero();
    globalLoadVector.resize(globalSize, 0);

    // Резервируем объем необходимой памяти
    memMap.resize(globalSize);
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
            memMap[i * freedom + j] = sizeof(double) * int(mesh->getMeshMap(i).size() * freedom);
    globalStiffnessMatrix.reserve(memMap);
    memMap.resize(0);
}

void TEigenSolver::setupDynamicMatrix(TMesh* mesh)
{
    unsigned globalSize;

    size = unsigned(mesh->getNumVertex());
    freedom = mesh->getFreedom();
    globalSize = size * freedom;

    globalStiffnessMatrix.resize(globalSize, globalSize);
    globalStiffnessMatrix.setZero();
    globalMassMatrix.resize(globalSize, globalSize);
    globalMassMatrix.setZero();
    globalDampingMatrix.resize(globalSize, globalSize);
    globalDampingMatrix.setZero();
    globalLoadVector.resize(globalSize, 0);

    // Резервируем объем необходимой памяти
    memMap.resize(globalSize);
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
            memMap[i * freedom + j] = sizeof(double) * int(mesh->getMeshMap(i).size() * freedom);
    globalStiffnessMatrix.reserve(memMap);
    globalMassMatrix.reserve(memMap);
    globalDampingMatrix.reserve(memMap);
    memMap.resize(0);
}

void TEigenSolver::setBoundaryCondition(unsigned index,unsigned function,double value)
{
    for (Eigen::SparseMatrix<double>::InnerIterator i(globalStiffnessMatrix, index * freedom + function); i; ++i)
    {
        if (i.row() != i.col())
        {
            globalStiffnessMatrix.coeffRef(i.row(),i.col()) = value;
            globalStiffnessMatrix.coeffRef(i.col(),i.row()) = value;
        }
    }
    globalLoadVector[index*freedom + function] = value * globalStiffnessMatrix.coeffRef(index * freedom + function, index * freedom + function);
}

void TEigenSolver::print(string fname)
{
    unsigned sz = unsigned(globalLoadVector.size());
    double res;
    fstream out;

    out.open(fname, ios::out);
    out << sz << 'x' << sz + 1 << endl;

    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (unsigned i = 0; i < sz; i++)
    {
        for (unsigned j = 0; j < sz; j++)
        {
            res = globalStiffnessMatrix.coeff(i,j);
            out.precision(10);
            out.width(20);
            out << res << ' ';
        }
        out.precision(10);
        out.width(20);
        out << globalLoadVector[i] << endl;
    }
    out.close();
}

bool TEigenSolver::saveMatrix(string fname,SparseMatrix<double>& globalMatrix)
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

// Формирование левой части СЛАУ в динамике (согласно методу Тета-Вильсона)
void TEigenSolver::createDynamicMatrix(double th, double theta)
{
    double val,
          k1 = 3.0/(theta*th),
          k2 = 6.0/(theta*theta*th*th);

    for (int k = 0; k < globalDampingMatrix.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(globalDampingMatrix, k); it; ++it)
            if ((val = it.value()) != 0.0)
                globalStiffnessMatrix.coeffRef(k,it.col()) += k1 * val;
    for (int k = 0; k < globalMassMatrix.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(globalMassMatrix, k); it; ++it)
            if ((val = it.value()) != 0.0)
                globalStiffnessMatrix.coeffRef(k,it.col()) += k2 * val;
}

void mulMatrix(SparseMatrix<double>& A, vector<double>& b, vector<double>& r)
{
    r.resize(b.size());
    for (unsigned k = 0; k < A.outerSize(); ++k)
        for (SparseMatrix<double>::InnerIterator it(A,k); it; ++it)
            r[k] += it.value() * b[unsigned(it.col())];
}

// Формирование правой части СЛАУ в динамике (согласно методу Тета-Вильсона)
void TEigenSolver::createDynamicVector(matrix<double>& uvw, double th, double theta)
{
    vector<double> u1(size * freedom),
                   u2(size * freedom),
                   r1(size * freedom),
                   r2(size * freedom);
    double k1 = 3.0 / (theta * th),
           k2 = 6.0 / (theta * theta * th * th),
           k3 = 0.5 * theta * th;

    // Получаем значения U, Ut и Utt предыдущей итерации (или из начальных условий)
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
        {
            u1[i * freedom + j] = (k1 * uvw[j][i] + 2.0 * k2 * uvw[uvw.size1() - 2 * freedom + j][i] + 2.0 * uvw[uvw.size1() - freedom + j][i]) / k2;
            u2[i * freedom + j] = (k2 * uvw[j][i] + 2.0 * uvw[uvw.size1() - 2 * freedom + j][i] + k3 * uvw[uvw.size1() - freedom + j][i]) / k1;
        }

    mulMatrix(globalMassMatrix, u1, r1);
    mulMatrix(globalDampingMatrix, u2, r2);

    // Формирование столбца правой части с учетом "динамической" составляющей
    for (unsigned i = 0; i < size * freedom; i++)
        globalLoadVector[i] += r1[i] + r2[i];

}

