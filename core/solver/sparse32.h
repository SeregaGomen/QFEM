#ifndef SPARSE32_H
#define SPARSE32_H

#include <cstdlib>

/*******************************************************************/
#define MAXINDEX     0x40000000
#define MAXBLKSZE    16
/*******************************************************************/
struct BCCS_Factor
{
  int          nvtxs = 0, blksze = 0, error = 0, level = 0;
  int          lspace = 0, ispace = 0, nnzextra = 0, supcnt = 0, nroots = 0;
  int          hyperbolic = 0;
  int          *ipool = nullptr, *isuper = nullptr;
  int          *perm = nullptr, *invp = nullptr, *xlinds = nullptr, *xlvals = nullptr;
  int          *Llen = nullptr, *xsinds = nullptr, *xtinds = nullptr, *tinds = nullptr;
  int          *linds = nullptr;
  signed char  *svals = nullptr;
  double       *vpool = nullptr;
  double       *dvals = nullptr;
  double       *lvals = nullptr;
  double       opcount = 0, opextra = 0;
  double       ssthresh = 0, precision = 0;
  BCCS_Factor() = default;
  ~BCCS_Factor()
  {
      if (perm)
          free(perm);
      if (invp)
          free(invp);
      if (xlinds)
          free(xlinds);
      if (xlvals)
          free(xlvals);
      if (linds)
          free(linds);
      if (svals)
          free(svals);
      if (vpool)
          free(vpool);
      if (xtinds)
          free(xtinds);
      if (tinds)
          free(tinds);
      if (Llen)
          free(Llen);
  }
};
/*******************************************************************/
struct BCCS_Matrix
{
  int      nvtxs = 0;
  int      nnz = 0;
  int      blksze = 0;
  int      dtype = 0;
  int      *aptrs = nullptr;
  int      *ainds = nullptr;
  double   *avals = nullptr;
  BCCS_Matrix() = default;
  ~BCCS_Matrix()
  {
      clear();
  }
  void clear(void)
  {
      if (aptrs)
          free(aptrs);
      if (ainds)
          free(ainds);
      if (avals)
          free(avals);
      aptrs = ainds = nullptr;
      avals = nullptr;
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

