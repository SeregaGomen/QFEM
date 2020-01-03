#include <cstring>
#include <string>
#include <iomanip>
#include "bccsolver.h"
#include "msg/msg.h"
#include "mesh/mesh.h"

using namespace std;

extern TMessenger* msg;

bool TBCCSolver::solve(vector<double>& result,double eps,bool& isAborted)
{
    int error;

    /////////////
    // print("matr.txt");
    ///

    if ((error = spOrder(globalStiffnessMatrix, isAborted)))
    {
        if (!isAborted)
            throw EQUATION_NOT_SOLVED_ERR;
        return false;
    }

    if ((error = spFactor(globalStiffnessMatrix, eps, isAborted)))
    {
        if (!isAborted)
            throw EQUATION_NOT_SOLVED_ERR;
        return false;
    }
    spSolve(globalStiffnessMatrix, globalLoadVector.data());
    if (isAborted)
        return false;
    result = globalLoadVector;
    ////////////////////
//    for (unsigned i = 0; i < right.size(); i++)
//        cout << right[i] << endl;
    ////////////////////
    return true;
}

void TBCCSolver::setupStaticMatrix(TMesh* mesh)
{
    globalStiffnessMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    globalLoadVector.resize(mesh->getNumVertex()*mesh->getFreedom());
}

void TBCCSolver::setupDynamicMatrix(TMesh* mesh)
{
    globalStiffnessMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    globalMassMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    globalDampingMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    globalLoadVector.resize(mesh->getNumVertex()*mesh->getFreedom());
}

void TBCCSolver::print(string fname)
{
    int size = globalStiffnessMatrix.nvtxs * globalStiffnessMatrix.blksze;
    fstream out;

    out.open(fname.c_str(),ios::out);
    out << size << 'x' << size + 1 << endl;

    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
            out << setw(20) << setprecision(5) << globalStiffnessMatrix.spGetElem(i, j) << ' ';
        out << setw(20) << setprecision(5) << globalLoadVector[unsigned(i)] << endl;
    }
    out.close();
}

void TBCCSolver::setBoundaryCondition(unsigned index, unsigned function, double value)
{
    for (int i = 0; i < globalStiffnessMatrix.nvtxs * globalStiffnessMatrix.blksze; i++)
        if (i != int(index) * globalStiffnessMatrix.blksze + int(function))
            if (globalStiffnessMatrix.spGetElem(int(index) * globalStiffnessMatrix.blksze + int(function), i) != 0.0)
            {
                globalStiffnessMatrix.spSetElem(int(index) * globalStiffnessMatrix.blksze + int(function), i, value);
                globalStiffnessMatrix.spSetElem(i, int(index) * globalStiffnessMatrix.blksze + int(function), value);
            }
    globalLoadVector[index * unsigned(globalStiffnessMatrix.blksze) + function] =
            value * globalStiffnessMatrix.spGetElem(int(index) * globalStiffnessMatrix.blksze + int(function), int(index) * globalStiffnessMatrix.blksze + int(function));
}

bool TBCCSolver::saveMatrix(string fname, BCCS_Matrix& globalMatrix)
{
    fstream out(fname, ios::out | ios::binary);
    int len = globalMatrix.aptrs[globalMatrix.nvtxs] * globalMatrix.blksze * globalMatrix.blksze,
        signature = 12031971;

    if (out.fail())
        return false;

    out.write(reinterpret_cast<char*>(&signature), sizeof(int));
    out.write(reinterpret_cast<char*>(&globalMatrix.nvtxs), sizeof(int));
    out.write(reinterpret_cast<char*>(&globalMatrix.nnz), sizeof(int));
    out.write(reinterpret_cast<char*>(&globalMatrix.blksze), sizeof(int));
    out.write(reinterpret_cast<char*>(globalMatrix.aptrs), unsigned(globalMatrix.nvtxs + 1) * sizeof(int));
    out.write(reinterpret_cast<char*>(globalMatrix.ainds), unsigned(globalMatrix.aptrs[globalMatrix.nvtxs]) * sizeof(int));
    out.write(reinterpret_cast<char*>(globalMatrix.avals), unsigned(len) * sizeof(double));
    out.close();
    return !out.fail();
}

bool TBCCSolver::loadMatrix(string fname, BCCS_Matrix& globalMatrix)
{
    unsigned len,
             signature;
    fstream in(fname,ios::in | ios::binary);

    if (in.fail())
        return false;

    in.read(reinterpret_cast<char*>(&signature), sizeof(int));
    if (signature != 12031971)
    {
        in.close();
        return false;
    }
    in.read(reinterpret_cast<char*>(&globalMatrix.nvtxs), sizeof(int));
    in.read(reinterpret_cast<char*>(&globalMatrix.nnz), sizeof(int));
    in.read(reinterpret_cast<char*>(&globalMatrix.blksze), sizeof(int));
    if ((globalMatrix.aptrs = new int[unsigned(globalMatrix.nvtxs + 1)]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read(reinterpret_cast<char*>(&globalMatrix.aptrs), unsigned(globalMatrix.nvtxs + 1) * sizeof(int));
    if ((globalMatrix.ainds = new int[unsigned(globalMatrix.aptrs[globalMatrix.nvtxs])]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read(reinterpret_cast<char*>(globalMatrix.ainds), unsigned(globalMatrix.aptrs[globalMatrix.nvtxs]) * sizeof(int));
    len = unsigned(globalMatrix.aptrs[globalMatrix.nvtxs] * globalMatrix.blksze * globalMatrix.blksze);
    if ((globalMatrix.avals = new double[len]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read(reinterpret_cast<char*>(globalMatrix.avals), len * sizeof(double));
    in.close();
    return !in.fail();
}

// Формирование левой части СЛАУ в динамике (согласно методу Тета-Вильсона)
void TBCCSolver::createDynamicMatrix(double th, double theta)
{
    double k1 = 3.0 / (theta * th),
           k2 = 6.0 / (theta * theta * th * th);
    int size = globalStiffnessMatrix.aptrs[globalStiffnessMatrix.nvtxs] * globalStiffnessMatrix.blksze * globalStiffnessMatrix.blksze;

    for (int i = 0; i < size; i++)
        globalStiffnessMatrix.avals[i] = globalStiffnessMatrix.avals[i] + k1 * globalDampingMatrix.avals[i] + k2 * globalMassMatrix.avals[i];
}

// Формирование правой части СЛАУ в динамике (согласно методу Тета-Вильсона)
void TBCCSolver::createDynamicVector(matrix<double>& uvw, double th, double theta)
{
    unsigned size = unsigned(globalStiffnessMatrix.nvtxs * globalStiffnessMatrix.blksze);
    vector<double> u1(size),
                   u2(size),
                   r1(size),
                   r2(size);
    double k1 = 3.0 / (theta * th),
           k2 = 6.0 / (theta * theta * th * th),
           k3 = 0.5 * theta * th;

    // Получаем значения U, Ut и Utt предыдущей итерации (или из начальных условий)
    for (unsigned i = 0; i < unsigned(globalStiffnessMatrix.nvtxs); i++)
        for (unsigned j = 0; j < unsigned(globalStiffnessMatrix.blksze); j++)
        {
            u1[i * unsigned(globalStiffnessMatrix.blksze) + j] = (k1 * uvw[j][i] + 2.0 * k2 * uvw[uvw.size1() - 2 * unsigned(globalStiffnessMatrix.blksze) + j][i] + 2.0 * uvw[uvw.size1() - unsigned(globalStiffnessMatrix.blksze) + j][i]) / k2;
            u2[i * unsigned(globalStiffnessMatrix.blksze) + j] = (k2 * uvw[j][i] + 2.0 * uvw[uvw.size1() - 2 * unsigned(globalStiffnessMatrix.blksze) + j][i] + k3 * uvw[uvw.size1() - unsigned(globalStiffnessMatrix.blksze) + j][i]) / k1;
        }
    globalMassMatrix.spMulMatrix(u1.data(), r1.data());
    globalDampingMatrix.spMulMatrix(u2.data(), r2.data());

    // Формирование столбца правой части с учетом "динамической" составляющей
    for (unsigned i = 0; i < size; i++)
        globalLoadVector[i] += r1[i] + r2[i];

}
