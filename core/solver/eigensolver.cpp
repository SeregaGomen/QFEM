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
    VectorXd x;

    /////////////
    // print("matr1.txt");
    ///

    msg->setProcess(ProcessCode::PreparingSystemEquation);
    solver.compute(stiffness);
    msg->stop();
    if (solver.info() != Success)
        throw ErrorCode::EEquationNotSolved;

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    x = solver.solve(load);
    msg->stop();

    if (solver.info() != Success)
        throw ErrorCode::EEquationNotSolved;

    r.resize(stiffness.rows());
    copy(&x[0], x.data() + x.cols() * x.rows(), r.begin());
    return true;
}

void TEigenSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    unsigned size = mesh->getNumVertex(),
             freedom = mesh->getFreedom();
    VectorXi memMap;

    // Резервируем объем необходимой памяти
    memMap.resize(size * freedom);
    for (unsigned i = 0; i < size; i++)
        for (unsigned j = 0; j < freedom; j++)
            memMap[i * freedom + j] = int(mesh->getMeshMap(i).size() * freedom * freedom * freedom);

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
    boundary_conditions.resize(mesh->getNumVertex() * mesh->getFreedom(), {false, 0});
}

void TEigenSolver::print(string fname)
{
    unsigned sz = unsigned(load.size());
    double res;
    fstream out(fname, ios::out);

    out << sz << 'x' << sz + 1 << endl;
    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (unsigned i = 0; i < sz; i++)
    {
        for (unsigned j = 0; j < sz; j++)
        {
            res = stiffness.coeff(i,j);
            out.precision(1);
            out.width(6);
            out << res << ' ';
        }
        out.precision(2);
        out.width(6);
        out << load[i] << endl;
    }
    out.close();
}
