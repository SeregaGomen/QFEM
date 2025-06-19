#include "bccsolver.h"
#include "msg/msg.h"
#include "mesh/mesh.h"

using namespace std;

extern TMessenger *msg;


bool TBCCSolver::solve(vector<double>& result,double eps,bool& isAborted)
{
    TBCCSFactor factor;


    if (spOrder(factor, stiffness, isAborted))
    {
        if (!isAborted)
            throw ErrorCode::EEquationNotSolved;
        return false;
    }

    if (spFactor(factor, stiffness, eps, isAborted))
    {
        if (!isAborted)
            throw ErrorCode::EEquationNotSolved;
        return false;
    }

    spSolve(factor, load);
    if (isAborted)
        return false;
    result = load;
    ////////////////////
//    for (unsigned i = 0; i < right.size(); i++)
//        cout << right[i] << endl;
    ////////////////////
    return true;
}

void TBCCSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    stiffness.clear();
    spSetMatrix(stiffness, (const int*)(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    if (isDynamic)
    {
        mass.clear();
        damping.clear();
        spSetMatrix(mass, (const int*)(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
        spSetMatrix(damping, (const int*)(mesh->getDataFE()), int(mesh->getNumFE()), int(mesh->getSizeFE()), int(mesh->getNumVertex()), int(mesh->getFreedom()));
    }
    load.resize(mesh->getNumVertex()*mesh->getFreedom());
    boundary_conditions.resize(mesh->getNumVertex() * mesh->getFreedom(), {false, 0});
}

