//---------------------------------------------------------------------------
#include <cmath>
#include <cassert>
#include <cfloat>

#include "sparse32.h"
#include "msg/msg.h"


extern TMessenger* msg;


#define MAXBLKSZE   16
#define MAXINDEX    0x40000000

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
// функция сравнения элементов массива
int cmp(const void *x1, const void *x2)
{
    return *static_cast<const int*>(x1) - *static_cast<const int*>(x2);
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void BCCS_Matrix::spSetElem(int i, int j, double val)
{
    int k, strt, stop, col, row, ofst;


    col  = i / blksze;
    row  = j / blksze;
    ofst = blksze * (i % blksze) + (j % blksze);

    strt = aptrs[col];
    stop = aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (ainds[k] == row)
        {
            avals[blksze * blksze * k + ofst] = val;
            break;
        }
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void BCCS_Matrix::spAddElem(int i, int j, double val)
{
    int k, strt, stop, col, row, ofst;


    col  = i / blksze;
    row  = j / blksze;
    ofst = blksze * (i % blksze) + (j % blksze);

    strt = aptrs[col];
    stop = aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (ainds[k] == row)
        {
            avals[blksze * blksze * k + ofst] += val;
            break;
        }
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
double BCCS_Matrix::spGetElem(int i, int j)
{
    int k, strt, stop, col, row, ofst;


    col  = i / blksze;
    row  = j / blksze;
    ofst = blksze * (i % blksze) + (j % blksze);

    strt = aptrs[col];
    stop = aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (ainds[k] == row)
            return avals[blksze * blksze * k + ofst];
    return 0;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void BCCS_Matrix::spSetMatrix(const int* mesh, int nelmnts, int elmsze, int nv, int bs)
{
    int i, j, m, k, n, nedges, node, *nptr, *mark, *nind, len, ilim;
    double *cv;

    nvtxs = nv;
    blksze = bs;
    /* allocate & set all element adjacency lenghts to nullptr */
    nptr = new int[unsigned(nvtxs) + 1];
    fill_n(nptr, size_t(nvtxs), 0);

    /* calculate sizes for list of all references to adjacency elements */
    j = elmsze * nelmnts;

    for (i = 0; i < j; i++)
    {
        assert(mesh[i] >= 0);
        assert(mesh[i] < nvtxs);
        nptr[mesh[i]]++;
    }

    /* calculate references from lenghts */
    for (node = i = 0; i < nvtxs; i++)
    {
        j = nptr[i];
        nptr[i] = node;
        node += j;
    }
    nptr[nvtxs] = node;
    nind = new int[unsigned(node)];

    /* save into nind all references to adjacency elements */
    for (k = i = 0; i < nelmnts; i++)
        for (j = 0; j < elmsze; j++)
            nind[nptr[mesh[k++]]++] = i;

    /* restore nptr */
    for (i = nvtxs; i > 0; i--)
        nptr[i] = nptr[i - 1];

    nptr[0] = 0;

    mark = new int[unsigned(nvtxs)];
    aptrs = new int[unsigned(nvtxs) + 1];

    /* calculate memory for adjacency structure */
    fill_n(mark, size_t(nvtxs), -1);

    for (nedges = i = 0; i < nvtxs; i++)
    {
        aptrs[i] = nedges;
        node = nptr[i + 1];
        for (j = nptr[i]; j < node; j++)
        {
            m = elmsze*nind[j];
            for (k = 0; k < elmsze; k++)
            {
                n = mesh[m++];
                if (mark[n] != i)
                {
                    mark[n] = i;
                    nedges++;
                }
            }
        }
    }
    aptrs[nvtxs] = nedges;
    ainds = new int[unsigned(nedges)];

    /* save adjacency structure */
    fill_n(mark, size_t(nvtxs), -1);
    for (nedges = i = 0; i < nvtxs; i++)
    {
        node = nptr[i + 1];
        for (j = nptr[i]; j < node; j++)
        {
            m = elmsze * nind[j];
            for (k = 0; k < elmsze; k++)
            {
                n = mesh[m++];
                if (mark[n] != i)
                {
                    mark[n] = i;
                    ainds[nedges++] = n;
                }
            }
        }
        assert(nedges - aptrs[i] > 0);
        qsort(&ainds[aptrs[i]], size_t(nedges - aptrs[i]), sizeof(int), cmp);
    }

    /* free work memory */
    delete [] mark;
    delete [] nind;
    delete [] nptr;

    /* prepare matrix */
    nnz = aptrs[nvtxs];
    len = nnz * blksze * blksze;
    avals = new double[unsigned(len)];

    for (ilim = len - 8, cv = avals, i = 0; i <= ilim; i += 8, cv += 8)
        cv[0] = cv[1] = cv[2] = cv[3] = cv[4] = cv[5] = cv[6] = cv[7] = 0;
    for (; i < len; i++)
        avals[i] = 0;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void BCCS_Matrix::spMulMatrix(double val)
{
    int len = aptrs[nvtxs] * blksze * blksze;

    for (int i = 0; i < len; i++)
        avals[i] *= val;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void BCCS_Matrix::spMulMatrix(const double* x, double *y)
{
    int i, j, bi, bj, strt, stop, node, sqrblk, ilim;
    const double *cx, *a, *ca;
    double *cy, *cv;

    sqrblk = blksze * blksze;
    for (ilim = nvtxs * blksze - 8, cv = y, i = 0; i <= ilim; i += 8, cv += 8)
        cv[0] = cv[1] = cv[2] = cv[3] = cv[4] = cv[5] = cv[6] = cv[7] = 0;
    for(; i < nvtxs * blksze; i++)
        y[i] = 0;

    cx = x;

    for (i = 0; i < nvtxs; i++, cx += blksze)
    {

        strt = aptrs[i];
        stop = aptrs[i+1];
        a = &avals[sqrblk*strt];

        for (j = strt; j < stop; j++, a += sqrblk)
        {

            node = ainds[j];
            cy = &y[blksze*node];
            ca = a;

            for (bi = 0; bi < blksze; bi++, ca += blksze)
                for (bj = 0; bj < blksze; bj++)
                    cy[bj] += ca[bj] * cx[bi];

        }
    }
}
/********************************************************************/
/********************************************************************/
/********************************************************************/
/*                                                                  */
/*   GENERAL SPARSE SYMMETRIC SYSTEM                                */
/*   ORDER METHOD: QMD                                              */
/*   FUNCTION: genqmd                                               */
/*                                                                  */
/********************************************************************/
/********************************************************************/
/********************************************************************/
int genqmd(const int nvtxs, const int xadj[], int adjncy[], int perm[], int invp[], int degree[], int marker[], int reach[], int nbrhd[], int qsize[], int qlink[], int qhead[], int *maxnonz, int *maxsub)
{
    int MinDeg, thresh, Deg, nDeg,nxnode, search, num, ip, np, counter, Flag, SetPower, i, j, k, node, nabor, Root, tmpRoot, jstrt, jstop,
        kstrt, kstop, rchsze, nhdsze, srchsze, snhdsze, ovlsze, *sreach, *snbrhd, *ovrlp, mrgsze, lnode, head, *mask;
    bool find;

    /* initialisation */
    *maxnonz = *maxsub = num = search = 0;
    MinDeg = nvtxs;
    for (i = 0; i < nvtxs; i++)
    {
        perm[i] = invp[i] = qhead[i] = i;
        qsize[i] = 1;
        qlink[i] = MAXINDEX;
        marker[i] = 0;
        degree[i] = nDeg = xadj[i+1] - xadj[i];
        if (nDeg < MinDeg)
            MinDeg = nDeg;
    }
    thresh = MinDeg;
    MinDeg = nvtxs;

    while (num < nvtxs)
    {
        /* threshold searching */
        for(;;)
        {
            if (num >= search)
                search = num;
            find = false;
            for (i = search; i < nvtxs; i++)
            {
                node = perm[i];
                if (marker[node] < 0)
                    continue;
                nDeg = degree[node];
                if (nDeg <= thresh)
                {
                    search = i;
                    find = true;
                    break;
                }
                if (nDeg <  MinDeg)
                    MinDeg = nDeg;
            }
            if (find)
                break;
            thresh = MinDeg;
            MinDeg = nvtxs;
            search = 0;
        }

        /* Root is pretendent! */
        Root = perm[search];
        marker[Root] = 1;
        Deg = degree[Root];
        *maxsub += Deg;

        /* calc reachable power (SetPower) */
        SetPower = rchsze = nhdsze = 0;
        jstrt = xadj[Root];
        jstop = xadj[Root+1];
        if (jstrt < jstop)
            for(j = jstrt; j < jstop; j++)
            {
                node = adjncy[j];
                assert(node < nvtxs);
                if (marker[node])
                    continue;
                if (degree[node] >= 0)
                {
                    reach[rchsze++] = node;
                    marker[node] = 1;
                    SetPower += qsize[node];
                    continue;
                }
                nbrhd[nhdsze++] = node;
                marker[node] = -1;
ReachSegment:
                kstrt = xadj[node];
                kstop = xadj[node + 1];
                for (k = kstrt; k < kstop; k++)
                {
                    node = adjncy[k];
                    if (node >= nvtxs)
                    {
                        node -= nvtxs;
                        if (node < nvtxs)
                            goto ReachSegment;
                        break;
                    }
                    if(marker[node]) continue;
                    reach[rchsze++] = node;
                    SetPower += qsize[node];
                    marker[node] = 1;
                } /* next k */
            } /* next j */

        /* numerate merged roots */
        for (nxnode = Root; nxnode < nvtxs; nxnode = qlink[nxnode])
        {
            assert(num < nvtxs);
            np = invp[nxnode];
            ip = perm[num];
            perm[np] = ip;
            invp[ip] = np;
            perm[num] = nxnode;
            invp[nxnode] = num;
            degree[nxnode] = -1;
            *maxnonz += Deg;
            Deg--;
            num++;
        }

        if (num >= nvtxs)
            break;
        if (rchsze == 0)
        {
            /* isolate root */
            marker[Root] = 0;
            continue; /* main loop */
        }
        sreach = &reach[rchsze];
        snbrhd = &nbrhd[nhdsze];
        counter = 0;

        /* find merged roots */
        for (snhdsze = i = 0; i < rchsze; i++)
        {
            node = reach[i];
            jstrt = xadj[node];
            jstop = xadj[node+1];
            for (j = jstrt; j < jstop; j++)
            {
                nabor = adjncy[j];
                if (degree[nabor] >= 0)
                    continue;
                if (marker[nabor])
                    continue;
                snbrhd[snhdsze++] = nabor;
                marker[nabor] = -1;
            } /* next j */
        } /* next i */
        if(snhdsze)
        {
            for (i = 0; i < snhdsze; i++)
                marker[snbrhd[i]] = 0;
            ovrlp = &snbrhd[snhdsze]; /* overlapped */
            for (i = 0; i < snhdsze; i++)
            {
                tmpRoot = snbrhd[i];
                marker[tmpRoot] = -1;
                Deg = ovlsze = srchsze = 0;
                node = tmpRoot;
MergeSegment:
                jstrt = xadj[node];
                jstop = xadj[node+1];
                for (j = jstrt; j < jstop; j++)
                {
                    node = adjncy[j];
                    if(node >= nvtxs)
                    {
                        node -= nvtxs;
                        if (node < nvtxs)
                            goto MergeSegment;
                        break;
                    }
                    mask = &marker[node];
                    if (*mask < 0)
                        continue;
                    if (*mask > 0)
                    {
                        if (*mask > 1)
                            continue;
                        *mask = 2; /* marker for overlapped */
                        ovrlp[ovlsze++] = node;
                        continue;
                    }
                    *mask = -2;
                    sreach[srchsze++] = node;
                    Deg += qsize[node];
                } /* next j */

                head = nvtxs;
                for (mrgsze = j = 0; j < ovlsze; j++)
                {
                    node = ovrlp[j];
                    kstrt = xadj[node];
                    kstop = xadj[node+1];
                    Flag = 0;
                    for (k = kstrt; k < kstop; k++)
                    {
                        if (marker[adjncy[k]])
                            continue;
                        Flag = 1;
                        break;
                    }
                    if (Flag)
                        marker[node] = 1;
                    else
                    {
                        mrgsze += qsize[node];
                        counter++;
                        marker[node] = -1;
                        lnode = qhead[node];
                        qlink[lnode] = head;
                        if (head < nvtxs)
                            qhead[node] = qhead[head];
                        head = node;
                    }
                } /* next j */
                if (head < nvtxs)
                {
                    degree[head] = SetPower + Deg - 1;
                    marker[head] = 2;
                    qsize[head] = mrgsze;
                }
                marker[tmpRoot] = 0;
                if (srchsze)
                    for (j = 0; j < srchsze; j++)
                        marker[sreach[j]] = 0;
            } /* next i */
        } /* if(snhdsze) */

        /* update root degree */
        for (i = 0; i < rchsze; i++)
        {
            tmpRoot = reach[i];
            mask = &marker[tmpRoot];
            if (*mask > 1 || *mask < 0)
                continue;
            *mask = 2;
            Deg = srchsze = snhdsze = 0;
            jstrt = xadj[tmpRoot];
            jstop = xadj[tmpRoot+1];
            for (j = jstrt; j < jstop; j++)
            {
                node = adjncy[j];
                assert(node < nvtxs);
                if (marker[node])
                    continue;
                if (degree[node] >= 0)
                {
                    sreach[srchsze++] = node;
                    Deg += qsize[node];
                    marker[node] = 1;
                    continue;
                }
                snbrhd[snhdsze++] = node;
                marker[node] = -1;
UpdateSegment:
                kstrt = xadj[node];
                kstop = xadj[node+1];
                for (k = kstrt; k < kstop; k++)
                {
                    node = adjncy[k];
                    if (node >= nvtxs)
                    {
                        node -= nvtxs;
                        if (node < nvtxs)
                            goto UpdateSegment;
                        break;
                    }
                    if (marker[node])
                        continue;
                    sreach[srchsze++] = node;
                    Deg += qsize[node];
                    marker[node] = 1;
                } /* next k */
            } /* next j */

            degree[tmpRoot] = SetPower + Deg - 1;
            counter++;
            if (srchsze)
                for (j = 0; j < srchsze; j++)
                    marker[sreach[j]] = 0;
            if (snhdsze)
                for (j = 0; j < snhdsze; j++)
                    marker[snbrhd[j]] = 0;
        } /* next i */

        assert(counter == rchsze);

        /* update thresh */
        marker[Root] = 0;
        for (j = 0; j < rchsze; j++)
        {
            nabor = reach[j];
            if (marker[nabor] < 0)
                continue;
            marker[nabor] = 0;
            nDeg = degree[nabor];
            if (nDeg < MinDeg)
                MinDeg = nDeg;
            if (nDeg > thresh)
                continue;
            MinDeg = thresh;
            thresh = nDeg;
            search = invp[nabor];
        } /* next j */

        /* transform graph */
        if (nhdsze)
        {
            nxnode = qhead[Root];
            for (counter = i = 0; i<nhdsze; i++)
            {
                node = nbrhd[i];
                qlink[nxnode] = node;
                counter += qsize[node];
                nxnode = qhead[node];
            }
            node = Root;
            qhead[Root] = nxnode;
            qsize[Root] += counter;
            jstrt = xadj[Root];
            jstop = xadj[Root+1] - 1;
            for (i = 0; i < rchsze; i++)
            {
                nabor = reach[i];
                if (marker[nabor] < 0)
                    continue;
                if (jstrt == jstop)
                {
                    lnode = jstrt;
                    do
                    {
                        node = qlink[node];
                        assert(node < nvtxs);
                        jstrt = xadj[node];
                        jstop = xadj[node + 1] - 1;
                    }
                    while (jstrt >= jstop);
                    adjncy[lnode] = node + nvtxs;
                }
                adjncy[jstrt++] = nabor;
            } /* next i */
            adjncy[jstrt] = MAXINDEX;
            for (i = 0; i < rchsze; i++)
            {
                node = reach[i];
                if (marker[node] < 0)
                    continue;
                jstrt = xadj[node];
                jstop = xadj[node+1];
                for (j = jstrt; j < jstop; j++)
                {
                    if (marker[adjncy[j]] < 0)
                    {
                        adjncy[j] = Root;
                        break;
                    }
                } /* next j */
            } /* next i */
        } /* end transforming */
    } /* main loop */
    return 0;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
void dtrsd(int n, const signed char d[], double r[])
{
    int i, ilim;
    double *cr;
    const signed char *cd;

    ilim = n - 8;
    for (cd = d, cr = r, i = 0; i <= ilim; i += 8, cr += 8, cd += 8)
    {
        cr[0] *= double(cd[0]);
        cr[1] *= double(cd[1]);
        cr[2] *= double(cd[2]);
        cr[3] *= double(cd[3]);
        cr[4] *= double(cd[4]);
        cr[5] *= double(cd[5]);
        cr[6] *= double(cd[6]);
        cr[7] *= double(cd[7]);
    }

    for (; i<n; i++)
        r[i] *= double(d[i]);

} /* end dtrsd */
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
static int merge(int a[], int alen, int b[], int blen, int t[])
{
    int i, j, k;

    i = j = k = 0;

    while ((i < alen) && (j < blen))
    {
        if (a[i] < b[j])
            t[k] = a[i++];
        else
            if(a[i] == b[j])
            {
                t[k] = a[i++];
                j++;
            }
            else
                t[k] = b[j++];
        k++;
    }

    while (i < alen)
        t[k++] = a[i++];

    while (j < blen)
        t[k++] = b[j++];

    return k;
}
/********************************************************************/
/********************************************************************/
/********************************************************************/
int funInSymbolic(BCCS_Matrix& matrix, int nvtxs, const int* aptrs, const int* ainds)
{
    int i, j, k, node, lsize, setsze, strt, stop, *linds, *pool, *mrglnk, *stack, *nodeset;

    matrix.xlvals = new int[unsigned(nvtxs) + 1];
    matrix.xlinds = new int[unsigned(nvtxs)];
    matrix.opcount = 0.0;

    /* allocate work memory */
    pool = new int[unsigned(3 * nvtxs)];
    mrglnk  = pool;
    stack   = pool + nvtxs;
    nodeset = pool + 2 * nvtxs;

    /* allocate initial buffer for compressed row indices */
    lsize = 20 * nvtxs;
    linds = new int [unsigned(lsize)];

    /* must be */
    matrix.xlvals[0] = 0;

    /* set all lists as empty */
    for (i = 0; i<nvtxs; i++)
        mrglnk[i] = nvtxs;

    for (k = 0; k < nvtxs; k++)
    {
        node = matrix.perm[k];
        assert(node < nvtxs);
        strt = aptrs[node];
        stop = aptrs[node+1];

        /* isolated root */
        if (strt == stop)
        {
            matrix.xlinds[k] = matrix.ispace;
            matrix.xlvals[k + 1] = matrix.lspace;
            continue;
        }

        /* consolidate A(*,k) */
        setsze = 0;
        for (j = strt; j < stop; j++)
        {
            node = matrix.invp[ainds[j]];
            assert(node < nvtxs);
            if (node <= k)
                continue;
            nodeset[setsze++] = node;
        }
        qsort(nodeset, size_t(setsze), size_t(sizeof(int)), cmp);

        /* inspect kids list and consolidate L(*,k) */
        for (i = mrglnk[k]; i < nvtxs; i = mrglnk[i])
        {
            /* i must be kid of k */
            assert(linds[matrix.xlinds[i]] == k);
            /* merge two sets */
            setsze = merge(nodeset, setsze, &linds[matrix.xlinds[i] + 1], matrix.xlvals[i + 1] - matrix.xlvals[i] - 1, stack);
            /* now set is in stack */
            swap(stack, nodeset);
            /* now set is in nodeset */
        }

        /* may be compression? */
        if (k > 0 && linds[matrix.xlinds[k - 1]] == k && setsze == (matrix.xlvals[k] - matrix.xlvals[k - 1] - 1))
        {
            /* it's indistinguishable node */
            matrix.xlinds[k] = matrix.xlinds[k - 1] + 1;
            matrix.lspace += setsze;
            matrix.xlvals[k + 1] = matrix.lspace;
        }
        else
        {
            if (matrix.ispace + setsze > lsize)
            {
                lsize = 2*lsize;
                linds = reinterpret_cast<int*>(realloc (linds, unsigned(lsize) * sizeof(int)));
            }

            j = matrix.ispace;
            for (i=0; i<setsze; i++)
                linds[j++] = nodeset[i];

            matrix.xlinds[k] = matrix.ispace;
            matrix.ispace += setsze;
            matrix.lspace += setsze;
            matrix.xlvals[k+1] = matrix.lspace;

        }

        matrix.opcount += double(setsze * setsze);

        if (setsze > 1)
        {
            i = linds[matrix.xlinds[k]];
            mrglnk[k] = mrglnk[i];
            mrglnk[i] = k;
        }

    } /* next k */

    assert(matrix.xlvals[nvtxs] == matrix.lspace);
    delete [] pool;

    /* truncate row indices array */
    matrix.linds = reinterpret_cast<int*>(realloc (linds, unsigned(matrix.ispace) * sizeof(int)));

    return 0;
}
/********************************************************************/
/********************************************************************/
/********************************************************************/
int spOrder(BCCS_Matrix& matrix, bool& aborted)
{
    int i, j, k, l, nvtxs, error, nedges, strt, stop, *perm, *invp, *mempool, *xadj, *adjncy, maxlnz, ispace;
    const int *aptrs, *ainds;

    nvtxs = matrix.nvtxs;
    aptrs = matrix.aptrs;
    ainds = matrix.ainds;

    /* allocate standart graph structure without diagonal indices */
    nedges = aptrs[nvtxs] - nvtxs;
    xadj = new int[unsigned(nvtxs + 1)];
    adjncy = new int[unsigned(nedges)];

    /* form (xadj,adjncy) from (aptrs,ainds) */
    msg->setProcess(ProcessCode::PreparingSystemEquation, 1, nvtxs);
    for(l = i = 0; i < nvtxs; i++)
    {
        msg->addProgress();
        if (aborted)
            return 1;

        xadj[i] = l;
        strt = aptrs[i];
        stop = aptrs[i+1];
        for (j=strt; j<stop; j++)
        {
            k = ainds[j];
            if (k != i)
            {
                assert(l < nedges);
                adjncy[l++] = k;
            }
        }
    }
    msg->stopProcess();
    xadj[nvtxs] = l;
    assert(l == nedges);

    matrix.nvtxs = nvtxs;

    /* now compute permutation */
    matrix.perm = perm = new int[unsigned(nvtxs)];
    matrix.invp = invp = new int[unsigned(nvtxs)];

    mempool = new int[7 * unsigned(nvtxs)];
    error = genqmd(nvtxs, xadj, adjncy, perm, invp, &mempool[0 * nvtxs], &mempool[1 * nvtxs], &mempool[2 * nvtxs], &mempool[3 * nvtxs],
            &mempool[4 * nvtxs], &mempool[5 * nvtxs], &mempool[6 * nvtxs], &maxlnz, &ispace);
    delete [] mempool;
    delete [] xadj;
    delete [] adjncy;

    if (error)
        return error;

    /* perform symbolic factorization */
    error = funInSymbolic(matrix, nvtxs, aptrs, ainds);

    return (matrix.error = error);
}
/*************************************************************/
/* GENERAL SPARSE SYMMETRIC SCHEME, CLASSIC FUN-IN ALGORITHM */
/*************************************************************/
static int gsfct1(BCCS_Matrix& matrix, double tol, bool& aborted)
{
    int i, j, k, n, knew, isub, nvtxs,strt, stop, node, error, *link, *first;
    double Djj, Ljk, value, scalef, dmin, dmax, *temp, *lvals, *dvals;
    signed char *svals;
    const int *aptrs, *ainds, *perm, *invp, *xlvals, *xlinds, *linds;
    const double*   avals;

    dmin =  DBL_MAX;
    dmax = -DBL_MAX;

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    perm   = matrix.perm;
    invp   = matrix.invp;
    xlvals = matrix.xlvals;
    xlinds = matrix.xlinds;
    linds  = matrix.linds;
    svals  = matrix.svals;
    dvals  = matrix.dvals;
    lvals  = matrix.lvals;
    aptrs  = matrix.aptrs;
    ainds  = matrix.ainds;
    avals  = matrix.avals;

    /* one-linked list */
    link  = new int[unsigned(nvtxs)];
    /* first for updateing */
    first = new int [unsigned(nvtxs)];
    /* column accumulator */
    temp  = new double[unsigned(nvtxs)];

    /* set all links as empty and clear temp */
    for (i = 0; i < nvtxs; i++)
    {
        temp[i] = 0.0;
        link[i] = nvtxs;
    }

    for (error = j = 0; j < nvtxs; j++)
    {
        msg->addProgress();
        if (aborted)
            return 1;

        /* form A structure */
        node = perm[j];
        strt = aptrs[node];
        stop = aptrs[node+1];
        assert(strt < stop);
        for (i = strt; i < stop; i++)
        {
            node = invp[ainds[i]];
            if (node < j)
                continue;
            temp[node] = avals[i];
        }

        Djj = temp[j];

        /* merge with previous columns */
        for (k = link[j]; k < nvtxs; k = knew)
        {
            knew = link[k];
            strt = first[k];
            stop = xlvals[k+1];
            Ljk = lvals[strt];
            value = Ljk * double(svals[k]);
            Djj -= Ljk * value;

            strt++;
            first[k] = strt;
            if (strt >= stop)
                continue;

            i = xlinds[k] + strt - xlvals[k];
            isub = linds[i];
            link[k] = link[isub];
            link[isub] = k;
            if (value == 0.0)
                continue;

            for (n = strt; n < stop; n++)
                temp[linds[i++]] -= lvals[n] * value;
        }

        /* test stability */
        value = fabs(Djj);
        if (value < dmin)
            dmin = value;
        if (value > dmax)
            dmax = value;
        /* check numerical stability */
        if (value < tol)
        {
            error = 1;
            break;
        }

        /* factor diagonal value */
        if (Djj < 0.0)
        {
            Djj = sqrt(value);
            matrix.hyperbolic++;
            svals[j] = -1;
            scalef = -1.0 / Djj;
        }
        else
        {
            Djj = sqrt(value);
            svals[j] = 1;
            scalef = 1.0 / Djj;
        }

        /* save diagonal */
        dvals[j] = Djj;

        strt = xlvals[j];
        stop = xlvals[j+1];
        if (strt >= stop)
            continue;

        /* update links */
        first[j] = xlvals[j];
        i = xlinds[j];
        isub = linds[i];
        link[j] = link[isub];
        link[isub] = j;

        /* scale subdiagonal */
        for (n = strt; n < stop; n++)
        {
            node = linds[i++];
            lvals[n] = temp[node] * scalef;
            temp[node] = 0.0;
        }
    }

    delete [] temp;
    delete [] first;
    delete [] link;

    return(error);
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
static int gsfctb(BCCS_Matrix& matrix, int blksze, double tol, bool& aborted)
{
    int error, i, j, k, n, node, strt, stop, knew, isub, bi, bj, bk, denter, fenter, colsze, nvtxs, *link, *first;
    double **translate;
    const int *aptrs, *ainds, *perm, *invp, *xlvals, *xlinds, *linds;
    const double *avals, *anonz;
    signed char *svals, *ps;
    double *lnonz, *lvals, value, dmin, dmax, *dvals, *dcolptr, *lcolptr, *hcolptr, *fcolptr, *fblkptr, *pl, *ph, *FF, *LL,
           sval, rd[MAXBLKSZE], HH[MAXBLKSZE * MAXBLKSZE], DD[MAXBLKSZE * MAXBLKSZE];

    /* check possible HH,DD,rd overflow */
    assert(blksze < MAXBLKSZE);

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    perm   = matrix.perm;
    invp   = matrix.invp;
    xlinds = matrix.xlinds;
    xlvals = matrix.xlvals;
    linds  = matrix.linds;
    svals  = matrix.svals;
    dvals  = matrix.dvals;
    lvals  = matrix.lvals;
    aptrs  = matrix.aptrs;
    ainds  = matrix.ainds;
    avals  = matrix.avals;

    /* sizes of diagonal & subdiagonal blocks */
    fenter = blksze * blksze;
    denter = (blksze * (blksze + 1)) / 2;

    /* setup initial diagonal range */
    dmin =  DBL_MAX;
    dmax = -DBL_MAX;

    /* allocate work memory */
    link      = new int[unsigned(nvtxs)];
    first     = new int[unsigned(nvtxs)];
    translate = new double*[unsigned(nvtxs)];

    /* set all links as empty */
    fill_n(link, size_t(nvtxs), nvtxs);


    /* main routine */

    msg->setProcess(ProcessCode::FactorizationSystemEquation, 1, nvtxs);
    for (error = j = 0; j < nvtxs; j++)
    {
        msg->addProgress();
        if (aborted)
            return 1;

        strt = xlvals[j];
        stop = xlvals[j + 1];

        /* set entries into factored column */
        lnonz = &lvals[fenter * strt];
        isub = xlinds[j];
        for (i = strt; i < stop; i++)
        {
            translate[linds[isub++]] = lnonz;
            for (k = 0; k < fenter; k++)
                lnonz[k] = 0.0;
            lnonz += fenter;
        }

        /* form A structure */
        node = perm[j];
        strt = aptrs[node];
        stop = aptrs[node+1];
        anonz = &avals[fenter * strt];

        for (i = strt; i < stop; i++, anonz += fenter)
        {
            node = invp[ainds[i]];
            if (node < j)
                continue;
            if (node == j)
                lnonz = DD;
            else
                lnonz = translate[node];

            for (k = 0; k < fenter; k++)
                lnonz[k] = anonz[k];
        }

        /* merge with previous columns */
        for (k = link[j]; k < nvtxs; k = knew)
        {
            knew = link[k];
            strt = first[k];
            LL = &lvals[fenter * strt];

            /* calculate HH */
            ph = HH;
            pl = LL;
            ps = &svals[blksze * k];
            for (bi = 0; bi < blksze; bi++)
            {
                value = double(ps[bi]);
                for (bj = 0; bj < blksze; bj++)
                    ph[bj] = pl[bj] * value;
                ph += blksze;
                pl += blksze;
            }

            /* DD -= LL * HH */
            dcolptr = DD;
            for (bi = 0; bi < blksze; bi++)
            {
                ph = HH;
                pl = LL;
                for(bj = 0; bj < blksze; bj++)
                {
                    value = ph[bi];
                    for (bk = bi; bk < blksze; bk++)
                        dcolptr[bk] -= pl[bk] * value;
                    ph += blksze;
                    pl += blksze;
                }
                dcolptr += blksze;
            }

            strt++;
            stop = xlvals[k + 1];
            if (strt >= stop)
                continue;

            /* update links */
            first[k] = strt;
            i = xlinds[k] + strt - xlvals[k];
            isub = linds[i];
            link[k] = link[isub];
            link[isub] = k;

            /* merge subdiagonal */
            for (n = strt; n < stop; n++)
            {

                LL += fenter;
                FF = translate[linds[i++]];

                /* FF -= LL * HH */
                for (bi = 0; bi < blksze; bi++)
                {
                    ph = HH;
                    pl = LL;
                    for (bj = 0; bj < blksze; bj++)
                    {
                        value = ph[bi];
                        for (bk = 0; bk < blksze; bk++)
                            FF[bk] -= pl[bk] * value;
                        ph += blksze;
                        pl += blksze;
                    }
                    FF += blksze;
                }


            } /* next merged block */

        } /* next merged columns */

        ps = &svals[blksze*j];

        /* diagonal block factorization */
        fcolptr = DD;
        for (bi = 0; bi < blksze; bi++)
        {
            /* accumulating */
            lcolptr = DD;
            for (bj = 0; bj < bi; bj++)
            {
                value = lcolptr[bi] * double(ps[bj]);
                for (bk = bi; bk<blksze; bk++)
                    fcolptr[bk] -= lcolptr[bk] * value;
                lcolptr += blksze;
            }
            value = fcolptr[bi];

            /* check indefinity */
            if (value < 0.0)
            {
                value = -value;
                matrix.hyperbolic++;
                sval = -1.0;
                ps[bi] = -1;
            }
            else
            {
                sval = 1.0;
                ps[bi] = 1;
            }

            /* test diagonal value */
            if (value < dmin) dmin = value;
            if (value > dmax) dmax = value;
            /* check numerical stability */
            if (value < tol)
            {
                error = 1;
                delete [] translate;
                delete [] first;
                delete [] link;
                return error;
            }

            value = sqrt(value);
            fcolptr[bi] = value;
            value = sval / value;
            rd[bi] = value;

            /* scale subdiagonal */
            for (bk = bi + 1; bk < blksze; bk++)
                fcolptr[bk] *= value;
            fcolptr += blksze;
        }

        /* save factored diagonal block */
        fcolptr = DD;
        dcolptr = &dvals[denter*j];
        colsze = blksze;
        for (bi = 0; bi < blksze; bi++)
        {
            for (bj = bi; bj < blksze; bj++)
                dcolptr[bj] = fcolptr[bj];
            colsze--;
            dcolptr += colsze;
            fcolptr += blksze;
        }

        /* do we must factor subdiagonal? */
        strt = xlvals[j];
        stop = xlvals[j + 1];
        if (strt >= stop)
            continue;

        /* update links */
        first[j] = strt;
        i = xlinds[j];
        isub = linds[i];
        link[j] = link[isub];
        link[isub] = j;

        /* calculate HH */
        hcolptr = HH;
        dcolptr = DD;
        for (bi = 0; bi < blksze; bi++)
        {
            value = double(ps[bi]);
            for (bj = bi + 1; bj < blksze; bj++)
                hcolptr[bj] = dcolptr[bj] * value;
            hcolptr += blksze;
            dcolptr += blksze;
        }

        /* scale subdiagonal */
        fblkptr = &lvals[fenter * strt];

        for (n = strt; n < stop; n++)
        {
            fcolptr = fblkptr;
            for (bi = 0; bi<blksze; bi++)
            {
                lcolptr = fblkptr;
                hcolptr = HH;
                for (bj = 0; bj < bi; bj++)
                {
                    value = hcolptr[bi];
                    for (bk = 0; bk < blksze; bk++)
                        fcolptr[bk] -= lcolptr[bk] * value;
                    hcolptr += blksze;
                    lcolptr += blksze;
                }
                value = rd[bi];
                for (bk = 0; bk < blksze; bk++)
                    fcolptr[bk] *= value;
                fcolptr += blksze;
            }
            fblkptr += fenter;
        }

    } /* next factored columns */
    msg->stopProcess();

    delete [] translate;
    delete [] first;
    delete [] link;
    return error;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
int spFactor(BCCS_Matrix& matrix, double tol, bool& aborted)
{
    int error, nvtxs, blksze, memsze, denter;

    /* test arguments */
    if ((matrix.aptrs == nullptr) || (matrix.ainds == nullptr) || (matrix.avals == nullptr) || (matrix.blksze <= 0) ||
        (matrix.blksze >= MAXBLKSZE) || (matrix.error != 0))
        return 1;

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    blksze = matrix.blksze;

    /* set blksze */
    matrix.blksze = blksze;

    /* clear hyperbolic counter */
    matrix.hyperbolic = 0;

    /* free memory for reentrance */
    if (matrix.vpool)
        delete [] matrix.vpool;
    if (matrix.svals)
        delete [] matrix.svals;

    /* allocate main lvals array */
    denter = (blksze * (blksze + 1)) / 2;
    memsze = matrix.lspace * blksze * blksze + nvtxs * denter;
    ///////////////////////////
    if ((matrix.vpool = new double[unsigned(memsze)]) == nullptr)
    {
        throw ErrorCode::EAllocMemory;
        // cerr << unsigned(memsze) * sizeof(double) << endl;
        // return 1;
    }
    ///////////////////////////
    matrix.dvals = matrix.vpool;
    matrix.lvals = &matrix.vpool[denter * nvtxs];

    /* allocate diagonal flags array */
    matrix.svals = new signed char[unsigned(nvtxs * blksze)];

    if (blksze == 1)
        error = gsfct1(matrix, tol, aborted);
    else
        error = gsfctb(matrix, blksze, tol, aborted);

    memsze = memsze * int(sizeof(double)) + nvtxs * blksze * int(sizeof(signed char));

    return (matrix.error = error);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
void permrv(double rhs[], const int* order, int nvtxs, int blksze)
{
    int i, j, node;
    double  *ptri, *ptrn;

    if (blksze == 1)
        for (i = 0; i < nvtxs; i++)
        {
            node = order[i];
            if (node == i)
                continue;
            while (node < i)
                node = order[node];
            swap(rhs[node], rhs[i]);
        }
    else
        for (i = 0; i < nvtxs; i++)
        {
            node = order[i];
            if (node == i)
                continue;
            while (node < i)
                node = order[node];
            ptri = &rhs[blksze*i];
            ptrn = &rhs[blksze*node];
            for (j = 0; j < blksze; j++)
                swap(ptrn[j], ptri[j]);
        }
}
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*                                                                 */
/*             GENERAL SPARSE SYMMETRIC SYSTEM                     */
/*                                                                 */
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
static void gsslv1(BCCS_Matrix& matrix, double rhs[])
{
    int i, j, k, strt, stop, nvtxs;
    const int *xlvals, *xlinds, *linds;
    const double *lvals, *dvals;
    double value;

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    xlvals = matrix.xlvals;
    xlinds = matrix.xlinds;
    linds  = matrix.linds;
    dvals  = matrix.dvals;
    lvals  = matrix.lvals;

    /* forward substitution */
    for (j = 0; j < nvtxs; j++)
    {
        value = dvals[j];
        assert(value != 0.0);
        value = (rhs[j] /= value);
        strt = xlvals[j];
        stop = xlvals[j + 1];
        i = xlinds[j];
        for (k = strt; k < stop; k++)
            rhs[linds[i++]] -= lvals[k] * value;
    }

    /* special diagonal subsystem */
    if (matrix.hyperbolic > 0)
        dtrsd(nvtxs, matrix.svals, rhs);

    /* backward substitution */
    for (j = nvtxs - 1; j >= 0; j--)
    {
        strt = xlvals[j];
        stop = xlvals[j + 1];
        if (strt < stop)
        {
            value = 0.0;
            i = xlinds[j];
            for (k = strt; k < stop; k++)
                value += lvals[k] * rhs[linds[i++]];
            rhs[j] -= value;
        }
        value = dvals[j];
        assert(value != 0.0);
        rhs[j] /= value;
    }
}
/********************************************************************/
/********************************************************************/
/********************************************************************/
static void gsslvb(BCCS_Matrix& matrix, double rght[], int blksze)
{
    int i, j, k, nvtxs, brow, bcol, colsze, strt, stop, denter, fenter, run;
    const int *xlvals, *xlinds, *linds;
    const double *dvals, *lvals, *diagj, *curdiag, *nonzk, *lblk;
    double value, *rghtj, *rghtk;

    fenter = blksze * blksze;
    denter = (blksze * (blksze + 1)) / 2;

    nvtxs  = matrix.nvtxs;
    xlvals = matrix.xlvals;
    xlinds = matrix.xlinds;
    linds  = matrix.linds;
    dvals  = matrix.dvals;
    lvals  = matrix.lvals;

    /* === forward substitution === */
    diagj = dvals;
    rghtj = rght;
    for (j = 0; j < nvtxs; j++)
    {
        /* dense lover diagonal system */
        curdiag = diagj;
        colsze = blksze;
        for (run = bcol = 0; bcol < blksze; bcol++)
        {
            value = rghtj[bcol];
            if (value != 0.0)
            {
                rghtj[bcol] = value = (value / curdiag[bcol]);
                run = 1;
                for (brow = bcol + 1; brow < blksze; brow++)
                    rghtj[brow] -= curdiag[brow] * value;
            }
            colsze--;
            curdiag += colsze;
        }

        if (run)
        {
            strt = xlvals[j];
            stop = xlvals[j+1];
            if (strt < stop)
            {
                nonzk = &lvals[fenter * strt];
                i = xlinds[j];
                for (k = strt; k < stop; k++)
                {
                    lblk = nonzk;
                    rghtk = &rght[blksze * linds[i++]];
                    for (bcol = 0; bcol < blksze; bcol++)
                    {
                        value = rghtj[bcol];
                        for (brow = 0; brow < blksze; brow++)
                            rghtk[brow] -= lblk[brow] * value;
                        lblk += blksze;
                    }
                    nonzk += fenter;
                }
            }
        }
        diagj += denter;
        rghtj += blksze;
    }

    /* === diagonal substitution === */
    if (matrix.hyperbolic > 0)
        dtrsd(blksze * nvtxs, matrix.svals, rght);

    /* === backward substitution === */
    diagj = &dvals[nvtxs*denter - blksze];
    for (j = nvtxs - 1; j >= 0; j--)
    {
        strt = xlvals[j];
        stop = xlvals[j + 1];
        rghtj = &rght[j * blksze];

        if (strt < stop)
        {
            i = xlinds[j];
            nonzk = &lvals[fenter * strt];
            for(k=strt; k<stop; k++)
            {
                lblk = nonzk;
                rghtk = &rght[blksze * linds[i++]];
                for (brow = 0; brow < blksze; brow++)
                {
                    value = 0.0;
                    for (bcol = 0; bcol < blksze; bcol++)
                        value += lblk[bcol] * rghtk[bcol];
                    rghtj[brow] -= value;
                    lblk += blksze;
                }
                nonzk += fenter;
            }
        }

        /* dense upper diagonal system */
        curdiag = diagj;
        colsze = 0;
        for (brow = blksze-1; brow>=0; brow--)
        {
            value = 0.0;
            for (bcol = brow + 1; bcol < blksze; bcol++)
                value += rghtj[bcol] * curdiag[bcol];
            rghtj[brow] = (rghtj[brow] - value) / curdiag[brow];
            colsze++;
            curdiag -= colsze;
        }
        diagj -= denter;
    }
}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
int spSolve(BCCS_Matrix &matrix, double *rhs)
{
    int i, nvtxs, blksze, len = 0, ilim;
    double value, *temp1 = nullptr, *temp2 = nullptr, *cb;
    const double *ca;

    if ((rhs == nullptr) || (matrix.error != 0))
        return 1;

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    nvtxs  = matrix.nvtxs;
    blksze = matrix.blksze;
    len = nvtxs * blksze;
    temp1 = new double[unsigned(len)];

    ilim = len - 8;
    for (ca = rhs, cb = temp1, i = 0; i <= ilim; i += 8, ca += 8, cb += 8)
    {
        cb[0] = ca[0];
        cb[1] = ca[1];
        cb[2] = ca[2];
        cb[3] = ca[3];
        cb[4] = ca[4];
        cb[5] = ca[5];
        cb[6] = ca[6];
        cb[7] = ca[7];
    }

    for (; i < len; i++)
        temp1[i] = rhs[i];


    /* direct permutation */
    permrv(rhs, matrix.perm, nvtxs, blksze);

    /* choose optimised variant or universal */
    if (blksze == 1)
        gsslv1(matrix, rhs);
    else
        gsslvb(matrix, rhs, blksze);

    /* invert permutation */
    permrv(rhs, matrix.invp, nvtxs, blksze);

    /* calculate numerical precision */
    temp2 = new double[unsigned(len)];
    /* temp2 = M * right */
    matrix.spMulMatrix(rhs, temp2);
    value = 0.0;
    for (i = 0; i < len; i++)
        value += fabs(temp1[i] - temp2[i]);

    delete [] temp2;
    delete [] temp1;

    msg->stop();
    return 0;
}
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

