#ifndef SPARSE32_H
#define SPARSE32_H


/*******************************************************************/
class BCCS_Matrix
{
public:
    int error = 0,
        lspace = 0,
        ispace = 0,
        hyperbolic = 0,
        *perm = nullptr,
        *invp = nullptr,
        *xlinds = nullptr,
        *xlvals = nullptr,
        *linds = nullptr,
        nvtxs = 0,
        nnz = 0,
        blksze = 0,
        dtype = 0,
        *aptrs = nullptr,
        *ainds = nullptr;
    signed char *svals = nullptr;
    double opcount = 0,
           *vpool = nullptr,
           *dvals = nullptr,
           *lvals = nullptr,
           *avals = nullptr;
    BCCS_Matrix(void) {}
    ~BCCS_Matrix(void)
    {
        clear();
    }
    void clear(void)
    {
        if (perm)
            delete [] perm;
        if (invp)
            delete [] invp;
        if (xlinds)
            delete [] xlinds;
        if (xlvals)
            delete [] xlvals;
        if (linds)
            delete [] linds;
        if (svals)
            delete [] svals;
        if (vpool)
            delete [] vpool;
        if (aptrs)
            delete [] aptrs;
        if (ainds)
            delete [] ainds;
        if (avals)
            delete [] avals;
        nvtxs = nnz = blksze = dtype = 0;
        perm = invp = xlinds = xlvals = linds = aptrs = ainds = nullptr;
        avals = vpool = dvals = lvals = nullptr;
        svals = nullptr;
    }
    void spSetElem(int, int, double);
    void spAddElem(int, int, double);
    void spPrepareMatrix(int);
    void spSetMatrix(const int*, int, int, int, int);
    void spMulMatrix(double);
    void spMulMatrix(const double*, double*);
    double spGetElem(int, int);
};
/*******************************************************************/
int spOrder(BCCS_Matrix&, bool&);
int spFactor(BCCS_Matrix&, double, bool&);
int spSolve(BCCS_Matrix&, double*);
/***********************************************************************/

#endif // SPARSE32_H

