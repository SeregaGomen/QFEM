#ifndef SPARSE32_H
#define SPARSE32_H

#include <vector>

/*******************************************************************/
#define MAXINDEX     0x40000000
#define MAXBLKSZE    16
/*******************************************************************/
struct BCCS_Factor
{
  int nvtxs = 0;
  int blksze = 0;
  int error = 0;
  int lspace = 0;
  int hyperbolic = 0;
  double *dvals = nullptr;
  double *lvals = nullptr;
  std::vector<int> linds;
  std::vector<int> perm;
  std::vector<int> invp;
  std::vector<int> xlinds;
  std::vector<int> xlvals;
  std::vector<char> svals;
  std::vector<double> vpool;
  BCCS_Factor() = default;
  ~BCCS_Factor() = default;
};
/*******************************************************************/
struct BCCS_Matrix
{
  int nvtxs = 0;
  int blksze = 0;
  std::vector<int> aptrs;
  std::vector<int> ainds;
  std::vector<double> avals;
  BCCS_Matrix() = default;
  ~BCCS_Matrix() = default;
  void clear(void)
  {
      aptrs.clear();
      ainds.clear();
      avals.clear();
  }
};
/*******************************************************************/
int spOrder(BCCS_Factor&, BCCS_Matrix&, bool&);
int spFactor(BCCS_Factor&, BCCS_Matrix&, double, bool&);
int spSolve(BCCS_Factor&, double*);
int spSetMatrix(BCCS_Matrix&, const int*, int, int, int, int);
int spMulMatrix(BCCS_Matrix&, const double*, double*);
int spMulMatrix(BCCS_Matrix&, double);
double spGetElem(BCCS_Matrix&, int, int);
void spSetElem(BCCS_Matrix&, int, int, double);
void spAddElem(BCCS_Matrix&, int, int, double);
/*******************************************************************/

#endif /* SPARSE32_H */

