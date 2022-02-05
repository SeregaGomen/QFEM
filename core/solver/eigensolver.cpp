#include <ctime>
//#include <Eigen/PardisoSupport>
#include <Eigen/SparseCholesky>
#include "mesh/mesh.h"
#include "solver/eigensolver.h"
#include "msg/msg.h"

extern TMessenger* msg;

bool TEigenSolver::solve(vector<double> &r, double, bool&)
{
//    PardisoLLT<SparseMatrix<double>> solver;
    SimplicialLLT<SparseMatrix<double>> solver;
//    ConjugateGradient<SparseMatrix<double>, Eigen::Upper> solver;
//    BiCGSTAB<SparseMatrix<double>> solver;
    VectorXd x,
             right = load;

//    cerr << globalStiffnessMatrix.nonZeros() << endl;
    /////////////
    // print("matr1.txt");
    ///

    msg->setProcess(ProcessCode::PreparingSystemEquation);
    solver.compute(stiffness);
    msg->stop();
    if (solver.info() not_eq Success)
        throw ErrorCode::EEquationNotSolved;

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    x = solver.solve(right);
    msg->stop();

    if (solver.info() not_eq Success)
        throw ErrorCode::EEquationNotSolved;

    r.resize(stiffness.rows());
    copy(&x[0], x.data() + x.cols() * x.rows(), r.begin());
//    for (unsigned i = 0; i < r.size(); i++)
//        r[i] = x(i);

    return true;
}

void TEigenSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    unsigned size = mesh->getNumVertex(),
             freedom = mesh->getFreedom();

    // Резервируем объем необходимой памяти
    memMap.resize(size * freedom);
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
            memMap[i * freedom + j] = int(mesh->getMeshMap(i).size() * freedom * freedom);

    stiffness.resize(size * freedom, size * freedom);
    stiffness.setZero();
    stiffness.reserve(memMap);
    if (isDynamic)
    {
        mass.resize(size * freedom, size * freedom);
        mass.setZero();
        mass.reserve(memMap);
        damping.resize(size * freedom, size * freedom);
        damping.setZero();
        damping.reserve(memMap);
    }
    load.resize(size * freedom);
    load.setZero();
    memMap.resize(0);
}

void TEigenSolver::setBoundaryCondition(unsigned index, double value)
{
    for (Eigen::SparseMatrix<double>::InnerIterator i(stiffness, index); i; ++i)
    {
        if (i.row() not_eq i.col())
        {
            stiffness.coeffRef(i.row(), i.col()) = value;
            stiffness.coeffRef(i.col(), i.row()) = value;
        }
    }
    load[index] = value * stiffness.coeffRef(index, index);
//    stiffnessMatrix.coeffRef(index, index) *= 1.0E+8;
//    loadVector[index] *= 1.0E+8 * value;
}
