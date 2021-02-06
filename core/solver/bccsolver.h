#ifndef BCCSOLVER_H
#define BCCSOLVER_H

#include <string>
#include "solver.h"
#include "sparse32.h"
#include "util/matrix.h"

using namespace std;

class TMesh;

//---------------------------------------------------------------
//  Класс, реализующий решение СЛАУ методом градиентного спуска
//---------------------------------------------------------------
class TBCCSolver : public TSolver<BCCS_Matrix>
{
private:
    bool loadMatrix(string, BCCS_Matrix&);
    bool saveMatrix(string, BCCS_Matrix&);
public:
    TBCCSolver(void) {}
    virtual ~TBCCSolver(void)
    {
        clear();
    }
    void clear(void)
    {
        loadVector.clear();
        stiffnessMatrix.clear();
        massMatrix.clear();
        dampingMatrix.clear();
    }
    void setBoundaryCondition(unsigned, double);
    void setMatrix(TMesh*, bool = false);
    void setStiffness(double value, unsigned i, unsigned j)
    {
        stiffnessMatrix.spSetElem(int(i), int(j), value);
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        massMatrix.spSetElem(int(i), int(j), value);
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        dampingMatrix.spSetElem(int(i), int(j), value);
    }
    void addStiffness(double value ,unsigned i, unsigned j)
    {
        stiffnessMatrix.spAddElem(int(i), int(j), value);
    }
    void addMass(double value ,unsigned i, unsigned j)
    {
        massMatrix.spAddElem(int(i), int(j), value);
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        dampingMatrix.spAddElem(int(i), int(j), value);
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return stiffnessMatrix.spGetElem(int(i), int(j));
    }
    double getMass(unsigned i, unsigned j)
    {
        return massMatrix.spGetElem(int(i), int(j));
    }
    double getDamping(unsigned i,unsigned j)
    {
        return dampingMatrix.spGetElem(int(i), int(j));
    }
    bool solve(vector<double>&, double, bool&);
    void print(string);
    void product(BCCS_Matrix&, vector<double>&, vector<double>&);
};

#endif // BCCSOLVER_H
