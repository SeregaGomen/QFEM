#ifndef SOLVER_H
#define SOLVER_H

#include <mutex>
#include <vector>
//#include <fstream>
//#include "util/matrix.h"

using namespace std;

class TMesh;

struct Condition
{
    bool used = false;
    double value = 0;
};

template <class MATRIX, class VECTOR> class TSolver
{
protected:
    mutex mtx;
    MATRIX stiffness;
    MATRIX mass;
    MATRIX damping;
    VECTOR load;
    vector<Condition> boundary_conditions;
    virtual bool solve(vector<double>&, double, bool&) = 0;
public:
    TSolver(void) = default;
    virtual ~TSolver(void) = default;
    virtual void setMatrix(TMesh*, bool = false) = 0;
    virtual void setElement(MATRIX&, unsigned, unsigned, double) = 0;
    virtual void addElement(MATRIX&, unsigned, unsigned, double) = 0;
    virtual double getElement(MATRIX&, unsigned, unsigned) = 0;
    void setStiffness(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            setElement(stiffness, i, j, value);
        }
    }
    void addStiffness(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            addElement(stiffness, i, j, value);
        }
    }
    void setMass(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            setElement(mass, i, j, value);
        }
    }
    void addMass(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            addElement(mass, i, j, value);
        }
    }
    void setDamping(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            setElement(damping, i, j, value);
        }
    }
    void addDamping(double value, unsigned i, unsigned j)
    {
        if (boundary_conditions[i].used == false and boundary_conditions[j].used == false)
        {
            lock_guard<mutex> guard(mtx);
            addElement(damping, i, j, value);
        }
    }
    double getStiffness(unsigned i, unsigned j)
    {
        return getElement(stiffness, i, j);
    }
    double getMass(unsigned i, unsigned j)
    {
        return getElement(mass, i, j);
    }
    double getDamping(unsigned i, unsigned j)
    {
        return getElement(damping, i, j);
    }
    bool solution(vector<double> &rhs, double eps, bool &is_canceled)
    {
        // Учет граничных условий
        for (auto i = 0u; i < boundary_conditions.size(); i++)
            if (boundary_conditions[i].used)
            {
                setElement(stiffness, i, i, 1.0);
                load[i] = boundary_conditions[i].value;
            }
        //////////////////
//        {
//            auto size = load.size();
//            double val;
//            ofstream outf("matr.res");

//            outf << size << 'x' << size + 1 << endl;
//            outf.setf( std::ios::fixed, std:: ios::floatfield );
//            for (auto i = 0u; i < size; i++)
//            {
//                for (auto j = 0; j < size; j++)
//                {
//                    val = getStiffness(i, j);
//                    outf.precision(10);
//                    outf.width(20);
//                    outf << val << ' ';
//                }
//                outf.precision(10);
//                outf.width(20);
//                outf << load[i] << endl;
//            }
//            outf.close();
//        }



/*
        {
            auto size = load.size();
            double val;
            ofstream outf("matrix.dat");

            outf << size << endl;
            for (auto i = 0u; i < size; i++)
                outf << load[i] << '\n';
            for (auto i = 0u; i < size; i++)
            {
                for (auto j = i; j < size; j++)
                {
                    val = getStiffness(i, j);
                    if (val != 0.0)
                        outf << i << ' ' << j << ' ' << val << '\n';
                }
            }
            outf.close();
        }
*/
        //////////////////
        return solve(rhs, eps, is_canceled);
    }
    void setBoundaryCondition(unsigned index, double value)
    {
        boundary_conditions[index].used = true;
        boundary_conditions[index].value = value;
    }
    void setLoad(double value, unsigned i)
    {
        load[i] = value;
    }
    void addLoad(double value, unsigned i)
    {
        load[i] += value;
    }
    double getLoad(unsigned i)
    {
        return load[i];
    }
};

#endif // SOLVER_H
