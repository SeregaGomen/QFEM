//---------------------------------------------------------------------------
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <vector>

#include "sparse32.h"
#include "../msg/msg.h"

extern TMessenger *msg;

/*********************************************************************/
//              функция сравнения элементов массива
/*********************************************************************/
int cmp(const void *x1, const void *x2)
{
    return *static_cast<const int*>(x1) - *static_cast<const int*>(x2);
}
/********************************************************************/
/*                                                                  */
/*   GENERAL SPARSE SYMMETRIC SYSTEM                                */
/*   ORDER METHOD: QMD                                              */
/*   FUNCTION: genqmd                                               */
/*                                                                  */
/********************************************************************/
int genqmd(const int nvtxs, const int *xadj, int *adjncy, int *perm, int *invp, int *degree, int *marker, int *reach, int *nbrhd, int *qsize,
            int *qlink, int *qhead, int *maxnonz, int *maxsub)
{
    int     MinDeg, thresh, Deg, nDeg,
            nxnode, search, num, ip, np, counter, Flag, SetPower,
            i, j, k, node, nabor, Root, tmpRoot, jstrt, jstop,
            kstrt, kstop, rchsze, nhdsze, srchsze, snhdsze, ovlsze,
            *sreach, *snbrhd, *ovrlp, mrgsze, lnode, head, *mask;

    /* initialisation */
    *maxnonz = *maxsub = num = search = 0;
    MinDeg = nvtxs;
    for(i=0; i<nvtxs; i++) {
        perm[i] = invp[i] = qhead[i] = i;
        qsize[i] = 1;
        qlink[i] = MAXINDEX;
        marker[i] = 0;
        degree[i] = nDeg = xadj[i+1] - xadj[i];
        if(nDeg < MinDeg) MinDeg = nDeg;
    }
    thresh = MinDeg;
    MinDeg = nvtxs;

    while(num < nvtxs) {

        /* threshold searching */
        for(;;) {
            if(num >= search) search = num;
            for(i=search; i<nvtxs; i++) {
                node = perm[i];
                if(marker[node] < 0) continue;
                nDeg = degree[node];
                if(nDeg <= thresh) {
                    search = i;
                    goto StopSearching;
                }
                if(nDeg <  MinDeg) MinDeg = nDeg;
            }
            thresh = MinDeg;
            MinDeg = nvtxs;
            search = 0;
        }
StopSearching:

        /* Root is pretendent! */
        Root = perm[search];
        marker[Root] = 1;
        Deg = degree[Root];
        *maxsub += Deg;

        /* calc reachable power (SetPower) */
        SetPower = rchsze = nhdsze = 0;
        jstrt = xadj[Root];
        jstop = xadj[Root+1];
        if(jstrt < jstop)
            for(j=jstrt; j<jstop; j++) {
                node = adjncy[j];
                assert(node < nvtxs);
                if(marker[node]) continue;
                if(degree[node] >= 0) {
                    reach[rchsze++] = node;
                    marker[node] = 1;
                    SetPower += qsize[node];
                    continue;
                }
                nbrhd[nhdsze++] = node;
                marker[node] = -1;
ReachSegment:
                kstrt = xadj[node];
                kstop = xadj[node+1];
                for(k=kstrt; k<kstop; k++) {
                    node = adjncy[k];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs) goto ReachSegment;
                        break;
                    }
                    if(marker[node]) continue;
                    reach[rchsze++] = node;
                    SetPower += qsize[node];
                    marker[node] = 1;
                } /* next k */
            } /* next j */

        /* numerate merged roots */
        for(nxnode=Root; nxnode<nvtxs; nxnode=qlink[nxnode]) {
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

        if(num >= nvtxs) break;
        if(rchsze == 0) { /* isolate root */
            marker[Root] = 0;
            continue; /* main loop */
        }
        sreach = &reach[rchsze];
        snbrhd = &nbrhd[nhdsze];
        counter = 0;

        /* find merged roots */
        for(snhdsze=i=0; i<rchsze; i++) {
            node = reach[i];
            jstrt = xadj[node];
            jstop = xadj[node+1];
            for(j=jstrt; j<jstop; j++) {
                nabor = adjncy[j];
                if(degree[nabor] >= 0) continue;
                if(marker[nabor]) continue;
                snbrhd[snhdsze++] = nabor;
                marker[nabor] = -1;
            } /* next j */
        } /* next i */
        if(snhdsze) {
            for(i=0; i<snhdsze; i++) {
                marker[snbrhd[i]] = 0;
            }
            ovrlp = &snbrhd[snhdsze]; /* overlapped */
            for(i=0; i<snhdsze; i++) {
                tmpRoot = snbrhd[i];
                marker[tmpRoot] = -1;
                Deg = ovlsze = srchsze = 0;
                node = tmpRoot;
MergeSegment:
                jstrt = xadj[node];
                jstop = xadj[node+1];
                for(j=jstrt; j<jstop; j++) {
                    node = adjncy[j];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs) goto MergeSegment;
                        break;
                    }
                    mask = &marker[node];
                    if(*mask < 0) continue;
                    if(*mask > 0) {
                        if(*mask > 1) continue;
                        *mask = 2; /* marker for overlapped */
                        ovrlp[ovlsze++] = node;
                        continue;
                    }
                    *mask = -2;
                    sreach[srchsze++] = node;
                    Deg += qsize[node];
                } /* next j */

                head = nvtxs;
                for(mrgsze=j=0; j<ovlsze; j++) {
                    node = ovrlp[j];
                    kstrt = xadj[node];
                    kstop = xadj[node+1];
                    Flag = 0;
                    for(k=kstrt; k<kstop; k++) {
                        if(marker[adjncy[k]]) continue;
                        Flag = 1;
                        break;
                    }
                    if(Flag) {
                        marker[node] = 1;
                    }
                    else {
                        mrgsze += qsize[node];
                        counter++;
                        marker[node] = -1;
                        lnode = qhead[node];
                        qlink[lnode] = head;
                        if(head < nvtxs) qhead[node] = qhead[head];
                        head = node;
                    }
                } /* next j */
                if(head < nvtxs) {
                    degree[head] = SetPower + Deg - 1;
                    marker[head] = 2;
                    qsize[head] = mrgsze;
                }
                marker[tmpRoot] = 0;
                if(srchsze)
                    for(j=0; j<srchsze; j++)
                        marker[sreach[j]] = 0;
            } /* next i */
        } /* if(snhdsze) */

        /* update root degree */
        for(i=0; i<rchsze; i++) {
            tmpRoot = reach[i];
            mask = &marker[tmpRoot];
            if(*mask > 1 || *mask < 0) continue;
            *mask = 2;
            Deg = srchsze = snhdsze = 0;
            jstrt = xadj[tmpRoot];
            jstop = xadj[tmpRoot+1];
            for(j=jstrt; j<jstop; j++) {
                node = adjncy[j];
                assert(node < nvtxs);
                if(marker[node]) continue;
                if(degree[node] >= 0) {
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
                for(k=kstrt; k<kstop; k++) {
                    node = adjncy[k];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs) goto UpdateSegment;
                        break;
                    }
                    if(marker[node]) continue;
                    sreach[srchsze++] = node;
                    Deg += qsize[node];
                    marker[node] = 1;
                } /* next k */
            } /* next j */

            degree[tmpRoot] = SetPower + Deg - 1;
            counter++;
            if(srchsze)
                for(j=0; j<srchsze; j++)
                    marker[sreach[j]] = 0;
            if(snhdsze)
                for(j=0; j<snhdsze; j++)
                    marker[snbrhd[j]] = 0;
        } /* next i */

        assert(counter == rchsze);

        /* update thresh */
        marker[Root] = 0;
        for(j=0; j<rchsze; j++) {
            nabor = reach[j];
            if(marker[nabor] < 0) continue;
            marker[nabor] = 0;
            nDeg = degree[nabor];
            if(nDeg < MinDeg) MinDeg = nDeg;
            if(nDeg > thresh) continue;
            MinDeg = thresh;
            thresh = nDeg;
            search = invp[nabor];
        } /* next j */

        /* transform graph */
        if(nhdsze) {
            nxnode = qhead[Root];
            for(counter=i=0; i<nhdsze; i++) {
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
            for(i=0; i<rchsze; i++) {
                nabor = reach[i];
                if(marker[nabor] < 0) continue;
                if(jstrt == jstop) {
                    lnode = jstrt;
                    do {
                        node = qlink[node];
                        assert(node < nvtxs);
                        jstrt = xadj[node];
                        jstop = xadj[node+1] - 1;
                    } while(jstrt >= jstop);
                    adjncy[lnode] = node + nvtxs;
                }
                adjncy[jstrt++] = nabor;
            } /* next i */
            adjncy[jstrt] = MAXINDEX;
            for(i=0; i<rchsze; i++) {
                node = reach[i];
                if(marker[node] < 0) continue;
                jstrt = xadj[node];
                jstop = xadj[node+1];
                for(j=jstrt; j<jstop; j++) {
                    if(marker[adjncy[j]] < 0) {
                        adjncy[j] = Root;
                        break;
                    }
                } /* next j */
            } /* next i */
        } /* end transforming */
    } /* main loop */

    return(0);
}
/*********************************/
/* solver for LDL' factorization */
/* diagonal system               */
/*********************************/
void dtrsd(int n, const signed char d[], double r[])
{
    int          i, ilim;
    double        *cr;
    const signed char  *cd;

    ilim = n - 8;
    for(cd=d, cr=r, i=0; i<=ilim; i+=8, cr+=8, cd+=8) {
        cr[0] *= (double) cd[0];
        cr[1] *= (double) cd[1];
        cr[2] *= (double) cd[2];
        cr[3] *= (double) cd[3];
        cr[4] *= (double) cd[4];
        cr[5] *= (double) cd[5];
        cr[6] *= (double) cd[6];
        cr[7] *= (double) cd[7];
    }

    for( ; i<n; i++) {
        r[i] *= (double) d[i];
    }

} /* end dtrsd */
/*****************************************************************/
int spSetMatrix(BCCS_Matrix &matrix, const int* mesh, int nelmnts, int elmsze, int nvtxs, int blksz)
{
    int   i, j, m, k, n, nedges, node,
            *nptr, *mark, *nind, *aptrs, *ainds,
            nnz, len;


    matrix.nvtxs = nvtxs;

    /* allocate & set all element adjacency lenghts to null */
    nptr = (int *) malloc ( (nvtxs+1) * sizeof(int));
    fill_n(nptr, size_t(nvtxs), 0);

    /* calculate sizes for list of all references to adjacency elements */
    j = elmsze * nelmnts;

    for(i=0; i<j; i++) {
        assert(mesh[i] >= 0);
        assert(mesh[i] < nvtxs);
        nptr[mesh[i]]++;
    }

    /* calculate references from lenghts */
    for(node=i=0; i<nvtxs; i++) {
        j = nptr[i];
        nptr[i] = node;
        node += j;
    }
    nptr[nvtxs] = node;
    nind = (int *) malloc (node * sizeof(int));

    /* save into nind all references to adjacency elements */
    for(k=i=0; i<nelmnts; i++) {
        for(j=0; j<elmsze; j++) {
            nind[nptr[mesh[k++]]++] = i;
        }
    }

    /* restore nptr */
    for(i=nvtxs; i>0; i--) {
        nptr[i] = nptr[i-1];
    }
    nptr[0] = 0;

    mark = (int *) malloc(nvtxs * sizeof(int));
    matrix.aptrs = aptrs = (int *) malloc((nvtxs + 1) * sizeof(int));

    /* calculate memory for adjacency structure */
    fill_n(mark, size_t(nvtxs), -1);
    for(nedges=i=0; i<nvtxs; i++) {
        aptrs[i] = nedges;
        node = nptr[i+1];
        for(j=nptr[i]; j<node; j++) {
            m = elmsze*nind[j];
            for(k=0; k<elmsze; k++) {
                n = mesh[m++];
                if(mark[n] != i) {
                    mark[n] = i;
                    nedges++;
                }
            }
        }
    }
    aptrs[nvtxs] = nedges;
    matrix.ainds = ainds = (int *) malloc(nedges * sizeof(int));

    /* save adjacency structure */
    fill_n(mark, size_t(nvtxs), -1);
    for(nedges=i=0; i<nvtxs; i++) {
        node = nptr[i+1];
        for(j=nptr[i]; j<node; j++) {
            m = elmsze*nind[j];
            for(k=0; k<elmsze; k++) {
                n = mesh[m++];
                if(mark[n] != i) {
                    mark[n] = i;
                    ainds[nedges++] = n;
                }
            }
        }
        assert(nedges-aptrs[i] > 0);
        //     iqsort(nedges-aptrs[i], &ainds[aptrs[i]]);
        qsort(&ainds[aptrs[i]], size_t(nedges - aptrs[i]), sizeof(int), cmp);
    }

    /* free work memory */
    free(mark);
    free(nind);
    free(nptr);

    nnz = matrix.aptrs[matrix.nvtxs];
    matrix.blksze = blksz;
    len = nnz * matrix.blksze * matrix.blksze;
    matrix.avals = (double *) malloc(len * sizeof(double));
    fill_n(matrix.avals, size_t(len), 0);


    return(0);
}
/*****************************************************************/
int spMulMatrix(BCCS_Matrix &matrix, const double* x, double *y)
{
    int i, j, bi, bj, strt, stop, node, sqrblk, ilim;
    const double *cx, *a, *ca;
    double *cy, *cv;

    sqrblk = matrix.blksze * matrix.blksze;
    for (ilim = matrix.nvtxs * matrix.blksze - 8, cv = y, i = 0; i <= ilim; i += 8, cv += 8)
        cv[0] = cv[1] = cv[2] = cv[3] = cv[4] = cv[5] = cv[6] = cv[7] = 0;
    for(; i < matrix.nvtxs * matrix.blksze; i++)
        y[i] = 0;

    cx = x;

    for (i = 0; i < matrix.nvtxs; i++, cx += matrix.blksze)
    {

        strt = matrix.aptrs[i];
        stop = matrix.aptrs[i+1];
        a = &matrix.avals[sqrblk*strt];

        for (j = strt; j < stop; j++, a += sqrblk)
        {

            node = matrix.ainds[j];
            cy = &y[matrix.blksze*node];
            ca = a;

            for (bi = 0; bi < matrix.blksze; bi++, ca += matrix.blksze)
                for (bj = 0; bj < matrix.blksze; bj++)
                    cy[bj] += ca[bj] * cx[bi];

        }
    }
    return 0;
}
/*****************************************************************/
int spMulMatrix(BCCS_Matrix &matrix, double K)
{
    int len = matrix.aptrs[matrix.nvtxs] * matrix.blksze * matrix.blksze,
            i;

    for (i = 0; i < len; i++)
        matrix.avals[i] *= K;
    return(0);
}
/***********************************************************************/
int merge(int *a, int alen, int *b, int blen, int *t)
{
    int   i, j, k;

    i=j=k=0;

    while( (i < alen) && (j < blen) ) {
        if(a[i] < b[j]) {
            t[k] = a[i++];
        } else if(a[i] == b[j]) {
            t[k] = a[i++];
            j++;
        } else {
            t[k] = b[j++];
        }
        k++;
    }

    while(i < alen) {
        t[k++] = a[i++];
    }

    while(j < blen) {
        t[k++] = b[j++];
    }

#ifndef NDEBUG
    for(i=1; i<k; i++) {
        assert(t[i-1] < t[i]);
    }
#endif

    return(k);
}
/********************************************************************/
static int funInSymbolic(BCCS_Factor &factor, const int* aptrs, const int* ainds)
{
    int i, j, k, node, lsize, setsze, strt, stop, nvtxs, *xlvals, *xlinds, *linds, *pool, *mrglnk, *stack, *nodeset;
    const int *perm, *invp;

    perm = factor.perm;
    invp = factor.invp;
    nvtxs = factor.nvtxs;
    factor.xlvals = xlvals = (int *) malloc((nvtxs + 1) * sizeof(int));
    factor.xlinds = xlinds = (int *) malloc(nvtxs * sizeof(int));
    factor.opcount = 0.0;

    /* allocate work memory */
    pool = (int *) malloc((3 * nvtxs) * sizeof(int));
    mrglnk  = pool;
    stack   = pool + nvtxs;
    nodeset = pool + 2*nvtxs;

    /* allocate initial buffer for compressed row indices */
    lsize = 20 * nvtxs;
    linds = (int *) malloc(lsize * sizeof(int));

    /* must be */
    xlvals[0] = 0;

    /* set all lists as empty */
    for(i=0; i<nvtxs; i++) {
        mrglnk[i] = nvtxs;
    }

    for(k=0; k<nvtxs; k++) {
        node = perm[k];
        assert(node < nvtxs);
        strt = aptrs[node];
        stop = aptrs[node+1];

        /* isolated root */
        if(strt == stop) {
            xlinds[k] = factor.ispace;
            xlvals[k+1] = factor.lspace;
            continue;
        }

        /* consolidate A(*,k) */
        setsze = 0;
        for(j=strt; j<stop; j++) {
            node = invp[ainds[j]];
            assert(node < nvtxs);
            if(node <= k) continue;
            nodeset[setsze++] = node;
        }
        //     iqsort(setsze, nodeset);
        qsort(nodeset, size_t(setsze), size_t(sizeof(int)), cmp);

        /* inspect kids list and consolidate L(*,k) */
        for(i=mrglnk[k]; i<nvtxs; i=mrglnk[i]) {
            /* i must be kid of k */
            assert(linds[xlinds[i]] == k);
            /* merge two sets */
            setsze = merge(nodeset, setsze,
                           &linds[xlinds[i]+1], xlvals[i+1]-xlvals[i]-1, stack);
            /* now set is in stack */
            swap(stack, nodeset);
            /* now set is in nodeset */
        }

        /* may be compression? */
        if(k > 0 && linds[xlinds[k-1]] == k &&
                setsze == (xlvals[k]-xlvals[k-1]-1))
        {
            /* it's indistinguishable node */
            xlinds[k] = xlinds[k-1] + 1;
            factor.lspace += setsze;
            xlvals[k+1] = factor.lspace;

        } else {

            if(factor.ispace + setsze > lsize) {
                lsize = 2*lsize;
                linds = (int *) realloc (linds, lsize * sizeof(int));
            }

            j = factor.ispace;
            for(i=0; i<setsze; i++) {
                linds[j++] = nodeset[i];
            }

            xlinds[k] = factor.ispace;
            factor.ispace += setsze;
            factor.lspace += setsze;
            xlvals[k+1] = factor.lspace;

        }

        factor.opcount += (double)(setsze * setsze);

        if(setsze > 1) {
            i = linds[xlinds[k]];
            mrglnk[k] = mrglnk[i];
            mrglnk[i] = k;
        }

    } /* next k */

    assert(xlvals[nvtxs] == factor.lspace);
    free(pool);

    /* truncate row indices array */
    factor.linds = (int *) realloc (linds, factor.ispace * sizeof(int));

    return(0);
}
/********************************************************************/
int spOrder(BCCS_Factor &factor, BCCS_Matrix &matrix, bool &aborted)
{

    int i, j, k, l, nvtxs, error, nedges, strt, stop, *perm, *invp, *mempool, *xadj, *adjncy, maxlnz, ispace;
    const int *aptrs, *ainds;

    nvtxs = matrix.nvtxs;
    aptrs = matrix.aptrs;
    ainds = matrix.ainds;


    /* allocate standart graph structure without diagonal indices */
    nedges = aptrs[nvtxs] - nvtxs;
    xadj = (int *) malloc((nvtxs + 1) * sizeof(int));
    adjncy = (int *) malloc(nedges * sizeof(int));

    /* form (xadj,adjncy) from (aptrs,ainds) */

    msg->setProcess(ProcessCode::PreparingSystemEquation, 0, nvtxs - 1, 10);
    for(l=i=0; i<nvtxs; i++)
    {

        msg->addProgress();
        if (aborted)
        {
            //          msg->setError(ABORT_ERR);
            free(xadj);
            free(adjncy);
            return 1;
        }

        xadj[i] = l;
        strt = aptrs[i];
        stop = aptrs[i+1];
        for(j=strt; j<stop; j++) {
            k = ainds[j];
            if(k != i) {
                assert(l < nedges);
                adjncy[l++] = k;
            }
        }
    }
    msg->stopProcess();
    xadj[nvtxs] = l;
    assert(l == nedges);

    factor.nvtxs = nvtxs;

    /* now compute permutation */
    factor.perm = perm = (int *) malloc(nvtxs * sizeof(int));
    factor.invp = invp = (int *) malloc(nvtxs * sizeof(int));


    mempool = (int *) malloc(7 * nvtxs * sizeof(int));
    error = genqmd(nvtxs, xadj, adjncy, perm, invp, &mempool[0*nvtxs], &mempool[1*nvtxs], &mempool[2*nvtxs], &mempool[3*nvtxs],
                   &mempool[4*nvtxs], &mempool[5*nvtxs], &mempool[6*nvtxs], &maxlnz, &ispace);

    free(mempool);
    free(xadj);
    free(adjncy);
    if (error != 0)
        return (error);

    /* perform symbolic factorization */
    error = funInSymbolic(factor, aptrs, ainds);

    return factor.error = error;
}
/*************************************************************/
/* GENERAL SPARSE SYMMETRIC SCHEME, CLASSIC FUN-IN ALGORITHM */
/*************************************************************/
int gsfctb(BCCS_Factor &factor, BCCS_Matrix &matrix, double eps, bool &aborted)
{
    int      error, i, j, k, n, node, strt, stop, knew, isub,
            bi, bj, bk, denter, fenter, colsze, nvtxs, blksze,
            *link, *first;
    double    **translate;
    const int *aptrs, *ainds, *perm, *invp, *xlvals, *xlinds, *linds;
    const double *avals, *anonz;
    signed char    *svals, *ps;
    double    *lnonz, *lvals, value, dmin, dmax, *dvals,
            *dcolptr, *lcolptr, *hcolptr, *fcolptr, *fblkptr;

    double    *pl, *ph, *FF, *LL;
    double    sval,
            rd[MAXBLKSZE],
            HH[MAXBLKSZE * MAXBLKSZE],
            DD[MAXBLKSZE * MAXBLKSZE];


    /* load data from structure */
    nvtxs  = factor.nvtxs;
    perm   = factor.perm;
    invp   = factor.invp;
    xlinds = factor.xlinds;
    xlvals = factor.xlvals;
    linds  = factor.linds;
    svals  = factor.svals;
    dvals  = factor.dvals;
    lvals  = factor.lvals;
    aptrs  = matrix.aptrs;
    ainds  = matrix.ainds;
    avals  = matrix.avals;
    blksze = matrix.blksze;

    /* check possible HH,DD,rd overflow */
    assert(blksze < MAXBLKSZE);

    /* switch to even-odd */
    //  odd = blksze % 2;

    /* single work for last odd number*/
    //  blast = blksze - 1;
    //  blklim = blksze - 2;

    /* sizes of diagonal & subdiagonal blocks */
    fenter = blksze * blksze;
    denter = (blksze * (blksze + 1)) / 2;

    /* setup initial diagonal range */
    dmin =  DBL_MAX;
    dmax = -DBL_MAX;

    /* allocate work memory */
    link      = (int *) malloc(nvtxs * sizeof(int));
    first     = (int *) malloc(nvtxs * sizeof(int));
    translate = (double**) malloc(nvtxs * sizeof(double*));

    /* set all links as empty */
    fill_n(link, size_t(nvtxs), nvtxs);


    /* main routine */

    msg->setProcess(ProcessCode::FactorizationSystemEquation, 0, nvtxs - 1, 10);
    for (error = j = 0; j<nvtxs; j++, msg->addProgress())
    {
        if (aborted)
        {
            //         msg->setError(ABORT_ERR);
            free(link);
            free(first);
            free(translate);
            return 1;
        }

        strt = xlvals[j];
        stop = xlvals[j+1];

        /* set entries into factored column */
        lnonz = &lvals[fenter * strt];
        isub = xlinds[j];
        for(i=strt; i<stop; i++) {
            translate[linds[isub++]] = lnonz;
            for(k=0; k<fenter; k++) {
                lnonz[k] = 0.0;
            }
            lnonz += fenter;
        }

        /* form A structure */
        node = perm[j];
        strt = aptrs[node];
        stop = aptrs[node+1];
        anonz = &avals[fenter * strt];

        for(i=strt; i<stop; i++, anonz+=fenter) {
            node = invp[ainds[i]];
            if(node < j) continue;
            if(node == j) {
                lnonz = DD;
            } else {
                lnonz = translate[node];
            }
            for(k=0; k<fenter; k++) {
                lnonz[k] = anonz[k];
            }
        }

        /* merge with previous columns */
        for(k=link[j]; k<nvtxs; k=knew) {

            knew = link[k];
            strt = first[k];
            LL = &lvals[fenter * strt];

            /* calculate HH */
            ph = HH;
            pl = LL;
            ps = &svals[blksze*k];
            for(bi=0; bi<blksze; bi++) {
                value = (double) ps[bi];
                for(bj=0; bj<blksze; bj++) {
                    ph[bj] = pl[bj] * value;
                }
                ph += blksze;
                pl += blksze;
            }

            /* DD -= LL * HH */
            dcolptr = DD;
            for(bi=0; bi<blksze; bi++) {
                ph = HH;
                pl = LL;
                for(bj=0; bj<blksze; bj++) {
                    value = ph[bi];
                    for(bk=bi; bk<blksze; bk++) {
                        dcolptr[bk] -= pl[bk] * value;
                    }
                    ph += blksze;
                    pl += blksze;
                }
                dcolptr += blksze;
            }

            strt++;
            stop = xlvals[k+1];
            if(strt >= stop) continue;

            /* update links */
            first[k] = strt;
            i = xlinds[k] + strt - xlvals[k];
            isub = linds[i];
            link[k] = link[isub];
            link[isub] = k;

            /* merge subdiagonal */
            for(n=strt; n<stop; n++) {

                LL += fenter;
                FF = translate[linds[i++]];

                /* FF -= LL * HH */
                for(bi=0; bi<blksze; bi++) {
                    ph = HH;
                    pl = LL;
                    for(bj=0; bj<blksze; bj++) {
                        value = ph[bi];
                        for(bk=0; bk<blksze; bk++) {
                            FF[bk] -= pl[bk] * value;
                        }
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
        for(bi=0; bi<blksze; bi++) {
            /* accumulating */
            lcolptr = DD;
            for(bj=0; bj<bi; bj++) {
                value = lcolptr[bi] * (double) ps[bj];
                for(bk=bi; bk<blksze; bk++) {
                    fcolptr[bk] -= lcolptr[bk] * value;
                }
                lcolptr += blksze;
            }

            value = fcolptr[bi];

            /* check indefinity */
            if(value < 0.0) {
                value = -value;
                factor.hyperbolic++;
                sval = -1.0;
                ps[bi] = -1;
            } else {
                sval = 1.0;
                ps[bi] = 1;
            }

            /* test diagonal value */
            if(value < dmin) dmin = value;
            if(value > dmax) dmax = value;
            if(value < eps) {
                error = 1;
                goto before_return;
            }

            value = sqrt(value);
            fcolptr[bi] = value;
            value = sval / value;
            rd[bi] = value;

            /* scale subdiagonal */
            for(bk=bi+1; bk<blksze; bk++) {
                fcolptr[bk] *= value;
            }

            fcolptr += blksze;
        }

        /* save factored diagonal block */
        fcolptr = DD;
        dcolptr = &dvals[denter*j];
        colsze = blksze;
        for(bi=0; bi<blksze; bi++) {
            for(bj=bi; bj<blksze; bj++) {
                dcolptr[bj] = fcolptr[bj];
            }
            colsze--;
            dcolptr += colsze;
            fcolptr += blksze;
        }

        /* do we must factor subdiagonal? */
        strt = xlvals[j];
        stop = xlvals[j+1];
        if(strt >= stop) continue;

        /* update links */
        first[j] = strt;
        i = xlinds[j];
        isub = linds[i];
        link[j] = link[isub];
        link[isub] = j;

        /* calculate HH */
        hcolptr = HH;
        dcolptr = DD;
        for(bi=0; bi<blksze; bi++) {
            value = (double) ps[bi];
            for(bj=bi+1; bj<blksze; bj++) {
                hcolptr[bj] = dcolptr[bj] * value;
            }
            hcolptr += blksze;
            dcolptr += blksze;
        }

        /* scale subdiagonal */
        fblkptr = &lvals[fenter * strt];

        for(n=strt; n<stop; n++) {

            fcolptr = fblkptr;
            for(bi=0; bi<blksze; bi++) {
                lcolptr = fblkptr;
                hcolptr = HH;
                for(bj=0; bj<bi; bj++) {
                    value = hcolptr[bi];
                    for(bk=0; bk<blksze; bk++) {
                        fcolptr[bk] -= lcolptr[bk] * value;
                    }
                    hcolptr += blksze;
                    lcolptr += blksze;
                }
                value = rd[bi];
                for(bk=0; bk<blksze; bk++) {
                    fcolptr[bk] *= value;
                }
                fcolptr += blksze;
            }

            fblkptr += fenter;
        }

    } /* next factored columns */
    msg->stopProcess();

before_return:
    free(translate);
    free(first);
    free(link);

    return error;
}
/*********************************************************************/
int spFactor(BCCS_Factor &factor, BCCS_Matrix &matrix, double eps, bool &aborted)
{
    int       error, nvtxs, blksze, memsze, denter;

    /* test arguments */
    if((matrix.aptrs == NULL) ||
       (matrix.ainds == NULL) ||
       (matrix.avals == NULL) ||
       (matrix.blksze <= 0) ||
       (matrix.blksze >= MAXBLKSZE) ||
       (matrix.nvtxs != factor.nvtxs) ||
       (factor.error != 0) )
        return 1;

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    factor.blksze = blksze = matrix.blksze;

    /* clear hyperbolic counter */
    factor.hyperbolic = 0;

    /* free memory for reentrance */
    if (factor.vpool)
        free(factor.vpool);
    if (factor.svals)
        free(factor.svals);

    /* allocate main lvals array */
    denter = (blksze * (blksze + 1)) / 2;
    memsze = factor.lspace * blksze * blksze + nvtxs * denter;
    ///////////////////////////
    if ((factor.vpool = (double*) malloc(memsze * sizeof(double))) == NULL)
        throw ErrorCode::EAllocMemory;
    ///////////////////////////
    factor.dvals = factor.vpool;
    factor.lvals = &factor.vpool[denter * nvtxs];

    /* allocate diagonal flags array */
    factor.svals = (signed char *) malloc(nvtxs * blksze * sizeof(signed char));

    error = gsfctb(factor, matrix, eps, aborted);

    return factor.error = error;
}
/********************************************************************/
void permrv(double *rhs, const int* order, int nvtxs, int blksze)
{
    int i, j, node;
    double  *ptri, *ptrn;

    for(i=0; i<nvtxs; i++)
    {
        node = order[i];
        if(node == i)
            continue;
        while(node < i)
            node = order[node];

        ptri = &rhs[blksze*i];
        ptrn = &rhs[blksze*node];
        for(j=0; j<blksze; j++)
            swap(ptrn[j], ptri[j]);
    }

} /* end permrv */
/*******************************************************************/
/*                                                                 */
/*             GENERAL SPARSE SYMMETRIC SYSTEM                     */
/*                                                                 */
/*******************************************************************/
void gsslvb(BCCS_Factor &factor, double* rght)
{
    int i, j, k, nvtxs, brow, bcol, colsze, strt, stop, denter, fenter, run, blksze;
    const int *xlvals, *xlinds, *linds;
    const double *dvals, *lvals, *diagj, *curdiag, *nonzk, *lblk;
    double value, *rghtj, *rghtk;

    blksze = factor.blksze;
    nvtxs  = factor.nvtxs;
    xlvals = factor.xlvals;
    xlinds = factor.xlinds;
    linds  = factor.linds;
    dvals  = factor.dvals;
    lvals  = factor.lvals;

    fenter = blksze * blksze;
    denter = (blksze * (blksze + 1)) / 2;


    /* === forward substitution === */
    diagj = dvals;
    rghtj = rght;
    for(j=0; j<nvtxs; j++) {

        /* dense lover diagonal system */
        curdiag = diagj;
        colsze = blksze;
        for(run=bcol=0; bcol<blksze; bcol++) {
            value = rghtj[bcol];
            if(value != 0.0) {
                rghtj[bcol] = value = (value / curdiag[bcol]);
                run = 1;
                for(brow=bcol+1; brow<blksze; brow++) {
                    rghtj[brow] -= curdiag[brow] * value;
                }
            }
            colsze--;
            curdiag += colsze;
        }

        if(run) {
            strt = xlvals[j];
            stop = xlvals[j+1];
            if(strt < stop) {
                nonzk = &lvals[fenter*strt];
                i = xlinds[j];
                for(k=strt; k<stop; k++) {
                    lblk = nonzk;
                    rghtk = &rght[blksze*linds[i++]];
                    for(bcol=0; bcol<blksze; bcol++) {
                        value = rghtj[bcol];
                        for(brow=0; brow<blksze; brow++) {
                            rghtk[brow] -= lblk[brow] * value;
                        }
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
    if(factor.hyperbolic > 0)
        dtrsd(blksze*nvtxs, factor.svals, rght);

    /* === backward substitution === */
    diagj = &dvals[nvtxs*denter - blksze];
    for(j=nvtxs-1; j>=0; j--) {
        strt = xlvals[j];
        stop = xlvals[j+1];
        rghtj = &rght[j*blksze];

        if(strt < stop) {
            i = xlinds[j];
            nonzk = &lvals[fenter*strt];
            for(k=strt; k<stop; k++) {
                lblk = nonzk;
                rghtk = &rght[blksze*linds[i++]];
                for(brow=0; brow<blksze; brow++) {
                    value = 0.0;
                    for(bcol=0; bcol<blksze; bcol++) {
                        value += lblk[bcol] * rghtk[bcol];
                    }
                    rghtj[brow] -= value;
                    lblk += blksze;
                }
                nonzk += fenter;
            }
        }

        /* dense upper diagonal system */
        curdiag = diagj;
        colsze = 0;
        for(brow=blksze-1; brow>=0; brow--) {
            value = 0.0;
            for(bcol=brow+1; bcol<blksze; bcol++) {
                value += rghtj[bcol] * curdiag[bcol];
            }
            rghtj[brow] = (rghtj[brow] - value) / curdiag[brow];
            colsze++;
            curdiag -= colsze;
        }

        diagj -= denter;
    }
}
/*********************************************************************/
int spSolve(BCCS_Factor &factor, double *rhs)
{
    msg->setProcess(ProcessCode::SolutionSystemEquation);
    /* direct permutation */
    permrv(rhs, factor.perm, factor.nvtxs, factor.blksze);
    /* choose optimised variant or universal */
    gsslvb(factor, rhs);
    /* invert permutation */
    permrv(rhs, factor.invp, factor.nvtxs, factor.blksze);
    msg->stop();


    return(0);
}
/*********************************************************************/
double spGetElem(BCCS_Matrix &matrix, int i, int j)
{
    int k, strt, stop, col, row, ofst;


    col  = i / matrix.blksze;
    row  = j / matrix.blksze;
    ofst = matrix.blksze * (i % matrix.blksze) + (j % matrix.blksze);

    strt = matrix.aptrs[col];
    stop = matrix.aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (matrix.ainds[k] == row)
            return matrix.avals[matrix.blksze * matrix.blksze * k + ofst];
    return 0;
}
/*********************************************************************/
void spSetElem(BCCS_Matrix &matrix, int i, int j, double val)
{
    int k, strt, stop, col, row, ofst;


    col  = i / matrix.blksze;
    row  = j / matrix.blksze;
    ofst = matrix.blksze * (i % matrix.blksze) + (j % matrix.blksze);

    strt = matrix.aptrs[col];
    stop = matrix.aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (matrix.ainds[k] == row)
        {
            matrix.avals[matrix.blksze * matrix.blksze * k + ofst] = val;
            break;
        }
}
/*********************************************************************/
void spAddElem(BCCS_Matrix &matrix, int i, int j, double val)
{
    int k, strt, stop, col, row, ofst;


    col  = i / matrix.blksze;
    row  = j / matrix.blksze;
    ofst = matrix.blksze * (i % matrix.blksze) + (j % matrix.blksze);

    strt = matrix.aptrs[col];
    stop = matrix.aptrs[col+1];
    for (k = strt; k < stop; k++)
        if (matrix.ainds[k] == row)
        {
            matrix.avals[matrix.blksze * matrix.blksze * k + ofst] += val;
            break;
        }
}
/*********************************************************************/


