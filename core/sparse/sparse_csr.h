#ifndef TCSRMATRIX_H
#define TCSRMATRIX_H

#include <vector>

using namespace std;

class TMesh;

struct TCSRMatrix
{
    int size = 0;
    vector<int> ia;
    vector<int> ja;
    vector<double> a;
    void setMatrix(TMesh*);
    void setElem(int, int, double);
    void addElem(int, int, double);
    void clearRow(int);
    void clearCol(int);
    double getElem(int, int);
    int findPos(int, int);
};

#endif // TCSRMATRIX_H
