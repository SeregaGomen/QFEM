#ifndef SPARSE32_H
#define SPARSE32_H

#include <vector>

/*******************************************************************/
#define MAXINDEX     0x40000000
#define MAXBLKSZE    16
/*******************************************************************/

using namespace std;

struct TBCCSFactor
{
    int nvtxs = 0;
    int blksze = 0;
    int error = 0;
    int lspace = 0;
    vector<double> dvals;
    vector<double> lvals;
    vector<int> linds;
    vector<int> perm;
    vector<int> invp;
    vector<int> xlinds;
    vector<int> xlvals;
    vector<int> svals;
    //vector<double> vpool;
    TBCCSFactor() = default;
    ~TBCCSFactor() = default;
};
/*******************************************************************/
struct TBCCSMatrix
{
  int nvtxs = 0;
  int blksze = 0;
  vector<int> aptrs;
  vector<int> ainds;
  vector<double> avals;
  TBCCSMatrix() = default;
  ~TBCCSMatrix() = default;
  void clear(void)
  {
      aptrs.clear();
      ainds.clear();
      avals.clear();
  }
};
/*******************************************************************/
int spOrder(TBCCSFactor&, TBCCSMatrix&, bool&);
int spFactor(TBCCSFactor&, TBCCSMatrix&, double, bool&);
int spSolve(TBCCSFactor&, vector<double>&);
int spSetMatrix(TBCCSMatrix&, const int*, int, int, int, int);
int spMulMatrix(TBCCSMatrix&, const vector<double>&, vector<double>&);
int spMulMatrix(TBCCSMatrix&, double);
double spGetElem(TBCCSMatrix&, int, int);
void spSetElem(TBCCSMatrix&, int, int, double);
void spAddElem(TBCCSMatrix&, int, int, double);
/*******************************************************************/

#endif /* SPARSE32_H */

