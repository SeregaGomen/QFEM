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
        spSetElem(&stiffnessMatrix, int(i), int(j), value);
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        spSetElem(&massMatrix, int(i), int(j), value);
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        spSetElem(&dampingMatrix, int(i), int(j), value);
    }
    void addStiffness(double value ,unsigned i, unsigned j)
    {
        spAddElem(&stiffnessMatrix, int(i), int(j), value);
    }
    void addMass(double value ,unsigned i, unsigned j)
    {
        spAddElem(&massMatrix, int(i), int(j), value);
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        spAddElem(&dampingMatrix, int(i), int(j), value);
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return spGetElem(&stiffnessMatrix, int(i), int(j));
    }
    double getMass(unsigned i, unsigned j)
    {
        return spGetElem(&massMatrix, int(i), int(j));
    }
    double getDamping(unsigned i,unsigned j)
    {
        return spGetElem(&dampingMatrix, int(i), int(j));
    }
    bool solve(vector<double>&, double, bool&);
    void print(string);
    void product(BCCS_Matrix&, vector<double>&, vector<double>&);
};

#endif // BCCSOLVER_H
