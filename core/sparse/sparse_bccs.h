#ifndef SPARSE32_H
#define SPARSE32_H

#include <vector>

/*******************************************************************/
#define MAXINDEX     0x40000000
#define MAXBLKSZE    16
/*******************************************************************/
struct TBCCSFactor
{
    int nvtxs = 0;
    int blksze = 0;
    int error = 0;
    int lspace = 0;
    int hyperbolic = 0;
    std::vector<double> dvals;
    std::vector<double> lvals;
    std::vector<int> linds;
    std::vector<int> perm;
    std::vector<int> invp;
    std::vector<int> xlinds;
    std::vector<int> xlvals;
    std::vector<char> svals;
    //std::vector<double> vpool;
    TBCCSFactor() = default;
    ~TBCCSFactor() = default;
};
/*******************************************************************/
struct TBCCSMatrix
{
  int nvtxs = 0;
  int blksze = 0;
  std::vector<int> aptrs;
  std::vector<int> ainds;
  std::vector<double> avals;
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
int spSolve(TBCCSFactor&, double*);
int spSetMatrix(TBCCSMatrix&, const int*, int, int, int, int);
int spMulMatrix(TBCCSMatrix&, const double*, double*);
int spMulMatrix(TBCCSMatrix&, double);
double spGetElem(TBCCSMatrix&, int, int);
void spSetElem(TBCCSMatrix&, int, int, double);
void spAddElem(TBCCSMatrix&, int, int, double);
/*******************************************************************/

#endif /* SPARSE32_H */

