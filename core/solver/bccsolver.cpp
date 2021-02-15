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

    if ((error = spOrder(stiffnessMatrix, isAborted)))
    {
        if (!isAborted)
            throw ErrorCode::EEquationNorSolved;
        return false;
    }

    if ((error = spFactor(stiffnessMatrix, eps, isAborted)))
    {
        if (!isAborted)
            throw ErrorCode::EEquationNorSolved;
        return false;
    }
    spSolve(stiffnessMatrix, loadVector.data());
    if (isAborted)
        return false;
    result = loadVector;
    ////////////////////
//    for (unsigned i = 0; i < right.size(); i++)
//        cout << right[i] << endl;
    ////////////////////
    return true;
}

void TBCCSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    stiffnessMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    if (isDynamic)
    {
        massMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
        dampingMatrix.spSetMatrix(reinterpret_cast<const int*>(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    }
    loadVector.resize(mesh->getNumVertex()*mesh->getFreedom());
}

void TBCCSolver::print(string fname)
{
    int size = stiffnessMatrix.nvtxs * stiffnessMatrix.blksze;
    fstream out;

    out.open(fname.c_str(),ios::out);
    out << size << 'x' << size + 1 << endl;

    out.setf( std::ios::fixed, std:: ios::floatfield );
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
            out << setw(20) << setprecision(5) << stiffnessMatrix.spGetElem(i, j) << ' ';
        out << setw(20) << setprecision(5) << loadVector[unsigned(i)] << endl;
    }
    out.close();
}

void TBCCSolver::setBoundaryCondition(unsigned index, double value)
{
    for (auto i = 0; i < stiffnessMatrix.nvtxs * stiffnessMatrix.blksze; i++)
        if (i not_eq int(index))
            if (stiffnessMatrix.spGetElem(index, i) not_eq 0)
            {
                stiffnessMatrix.spSetElem(index, i, value);
                stiffnessMatrix.spSetElem(i, index, value);
            }
    loadVector[index] = value * stiffnessMatrix.spGetElem(index, index);
}

bool TBCCSolver::saveMatrix(string fname, BCCS_Matrix& globalMatrix)
{
    fstream out(fname, ios::out | ios::binary);
    int len = globalMatrix.aptrs[globalMatrix.nvtxs] * globalMatrix.blksze * globalMatrix.blksze;

    if (out.fail())
        return false;

    out.write((char*)&globalMatrix.nvtxs, sizeof(int));
    out.write((char*)&globalMatrix.nnz, sizeof(int));
    out.write((char*)&globalMatrix.blksze, sizeof(int));
    out.write((char*)globalMatrix.aptrs, (globalMatrix.nvtxs + 1) * sizeof(int));
    out.write((char*)globalMatrix.ainds, globalMatrix.aptrs[globalMatrix.nvtxs] * sizeof(int));
    out.write((char*)globalMatrix.avals, len * sizeof(double));
    out.close();
    return !out.fail();
}

bool TBCCSolver::loadMatrix(string fname, BCCS_Matrix& globalMatrix)
{
    unsigned len;
    fstream in(fname,ios::in | ios::binary);

    if (in.fail())
        return false;

    in.read((char*)&globalMatrix.nvtxs, sizeof(int));
    in.read((char*)&globalMatrix.nnz, sizeof(int));
    in.read((char*)&globalMatrix.blksze, sizeof(int));
    if ((globalMatrix.aptrs = new int[globalMatrix.nvtxs + 1]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read((char*)globalMatrix.aptrs, (globalMatrix.nvtxs + 1) * sizeof(int));
    if ((globalMatrix.ainds = new int[globalMatrix.aptrs[globalMatrix.nvtxs]]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read((char*)globalMatrix.ainds, globalMatrix.aptrs[globalMatrix.nvtxs] * sizeof(int));
    len = globalMatrix.aptrs[globalMatrix.nvtxs] * globalMatrix.blksze * globalMatrix.blksze;
    if ((globalMatrix.avals = new double[len]) == nullptr)
    {
        in.close();
        return false;
    }
    in.read((char*)globalMatrix.avals, len * sizeof(double));
    in.close();
    return !in.fail();
}

void TBCCSolver::product(BCCS_Matrix &matr, vector<double> &vec, vector<double> &res)
{
//    res.resize(vec.size());
    matr.spMulMatrix(vec.data(), res.data());
}
