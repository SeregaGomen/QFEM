#ifndef TENVSOLVER_H
#define TENVSOLVER_H


#include "solver.h"
#include "mesh/mesh.h"
#include "sparse/sparse_env.h"

class TEnvSolver : public TSolver<TEnvMatrix, vector<double>>
{
public:
    TEnvSolver() = default;
    virtual ~TEnvSolver() = default;
    void setMatrix(TMesh *mesh, bool isDynamic = false)
    {
        stiffness.setMatrix(mesh->getMeshMap(), mesh->getFreedom());
        if (isDynamic)
        {
            mass.setMatrix(mesh->getMeshMap(), mesh->getFreedom());
            damping.setMatrix(mesh->getMeshMap(), mesh->getFreedom());
        }
        load.resize(mesh->getNumVertex()*mesh->getFreedom());
    }
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffness.setValue(i, j, value);
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        mass.setValue(i, j, value);
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        damping.setValue(i, j, value);
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
        stiffness.addValue(i, j, value);
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        mass.addValue(i, j, value);
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        damping.addValue(i, j, value);
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffness.getValue(i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return mass.getValue(i, j);
    }
    double getDamping(unsigned i,unsigned j)
    {
        return damping.getValue(i, j);
    }
    void setBoundaryCondition(unsigned index, double value)
    {
//        stiffness.setValue(index, index, stiffness.getValue(index, index) * 1.0E+15);
//        load[index] = 1.0E+15 * value;
        for (auto i = 0u; i < stiffness.size(); i++)
            if (i < index)
                stiffness.setValue(index, i, value);
            else if (i > index)
                stiffness.setValue(i, index, value);
        load[index] = value * stiffness.getValue(index, index);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // TENVSOLVER_H
