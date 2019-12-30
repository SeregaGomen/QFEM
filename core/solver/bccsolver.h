#ifndef BCCSOLVER_H
#define BCCSOLVER_H

#include <string>
#include "solver.h"
#include "sparse/sparse32.h"
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
        globalLoadVector.clear();
        globalStiffnessMatrix.clear();
        globalMassMatrix.clear();
        globalDampingMatrix.clear();
    }
    void createDynamicMatrix(double, double);
    void createDynamicVector(matrix<double>&, double, double);
    void setBoundaryCondition(unsigned, unsigned, double);
    void setupStaticMatrix(TMesh*);
    void setupDynamicMatrix(TMesh*);
    void setStiffnessMatrix(double value, unsigned i, unsigned j)
    {
        globalStiffnessMatrix.spSetElem(int(i), int(j), value);
    }
    void setMassMatrix(double value, unsigned i, unsigned j)
    {
        globalMassMatrix.spSetElem(int(i), int(j), value);
    }
    void setDampingMatrix(double value, unsigned i, unsigned j)
    {
        globalDampingMatrix.spSetElem(int(i), int(j), value);
    }
    void addStiffnessMatrix(double value ,unsigned i, unsigned j)
    {
        globalStiffnessMatrix.spAddElem(int(i), int(j), value);
    }
    void addMassMatrix(double value ,unsigned i, unsigned j)
    {
        globalMassMatrix.spAddElem(int(i), int(j), value);
    }
    void addDampingMatrix(double value, unsigned i, unsigned j)
    {
        globalDampingMatrix.spAddElem(int(i), int(j), value);
    }
    double getStiffnessMatrix(unsigned i, unsigned j)
    {
        return globalStiffnessMatrix.spGetElem(int(i), int(j));
    }
    double getMassMatrix(unsigned i, unsigned j)
    {
        return globalMassMatrix.spGetElem(int(i), int(j));
    }
    double getDampingMatrix(unsigned i,unsigned j)
    {
        return globalDampingMatrix.spGetElem(int(i), int(j));
    }
    bool solve(vector<double>&, double, bool&);
    void print(string);
};

#endif // BCCSOLVER_H
