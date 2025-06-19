//---------------------------------------------------------------------------
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <vector>

#include "sparse_bccs.h"
#include "../msg/msg.h"

extern TMessenger *msg;

/********************************************************************/
/*                                                                  */
/*   GENERAL SPARSE SYMMETRIC SYSTEM                                */
/*   ORDER METHOD: QMD                                              */
/*   FUNCTION: genqmd                                               */
/*                                                                  */
/********************************************************************/
int genqmd(const int nvtxs, const vector<int> &xadj, vector<int> &adjncy, vector<int> &perm, vector<int> &invp)
{
    int MinDeg, thresh, Deg, nDeg,
        nxnode, search, num, ip, np, counter, Flag, SetPower,
        i, j, k, node, nabor, Root, tmpRoot, jstrt, jstop,
        kstrt, kstop, rchsze, nhdsze, srchsze, snhdsze, ovlsze,
        mrgsze, lnode, head;

    vector<int> degree(7 * nvtxs), marker(7 * nvtxs), reach(7 * nvtxs), nbrhd(7 * nvtxs), qsize(7 * nvtxs), qlink(7 * nvtxs), qhead(7 * nvtxs);


    /* initialisation */
    num = search = 0;
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
            Flag = 1;
            for(i=search; i<nvtxs; i++) {
                node = perm[i];
                if(marker[node] < 0) continue;
                nDeg = degree[node];
                if(nDeg <= thresh) {
                    search = i;
                    Flag = 0;
                    break;
                }
                if (nDeg <  MinDeg) MinDeg = nDeg;
            }
            if (Flag == 0)
                break;
            thresh = MinDeg;
            MinDeg = nvtxs;
            search = 0;
        }
        /* Root is pretendent! */
        Root = perm[search];
        marker[Root] = 1;
        Deg = degree[Root];

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
                k = xadj[node];
                kstop = xadj[node+1];
                while (k < kstop) {
                    node = adjncy[k++];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs)
                        {
                            k = xadj[node];
                            kstop = xadj[node + 1];
                            continue;
                        }
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
            Deg--;
            num++;
        }

        if(num >= nvtxs) break;
        if(rchsze == 0) { /* isolate root */
            marker[Root] = 0;
            continue; /* main loop */
        }
        //snbrhd = &nbrhd[nhdsze];
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
                nbrhd[nhdsze + snhdsze++] = nabor;
                marker[nabor] = -1;
            } /* next j */
        } /* next i */
        if(snhdsze) {
            for(i=0; i<snhdsze; i++) {
                marker[nbrhd[nhdsze + i]] = 0;
            }
            for(i=0; i<snhdsze; i++) {
                tmpRoot = nbrhd[nhdsze + i];
                marker[tmpRoot] = -1;
                Deg = ovlsze = srchsze = 0;
                node = tmpRoot;
                j = xadj[node];
                jstop = xadj[node+1];
                while (j < jstop) {
                    node = adjncy[j++];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs)
                        {
                            j = xadj[node];
                            jstop = xadj[node+1];
                            continue;
                        }
                        break;
                    }
                    if (marker[node] < 0) continue;
                    if (marker[node] > 0) {
                        if (marker[node] > 1) continue;
                        marker[node] = 2; /* marker for overlapped */
                        nbrhd[nhdsze + snhdsze + ovlsze++] = node;
                        continue;
                    }
                    marker[node] = -2;
                    reach[rchsze + srchsze++] = node;
                    Deg += qsize[node];
                } /* next j */

                head = nvtxs;
                for(mrgsze=j=0; j<ovlsze; j++) {
                    node = nbrhd[nhdsze + snhdsze + j];
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
                        marker[reach[rchsze + j]] = 0;
            } /* next i */
        } /* if(snhdsze) */

        /* update root degree */
        for(i=0; i<rchsze; i++) {
            tmpRoot = reach[i];
            if(marker[tmpRoot] > 1 || marker[tmpRoot] < 0) continue;
            marker[tmpRoot] = 2;
            Deg = srchsze = snhdsze = 0;
            jstrt = xadj[tmpRoot];
            jstop = xadj[tmpRoot+1];
            for(j=jstrt; j<jstop; j++) {
                node = adjncy[j];
                assert(node < nvtxs);
                if(marker[node]) continue;
                if(degree[node] >= 0) {
                    reach[rchsze + srchsze++] = node;
                    Deg += qsize[node];
                    marker[node] = 1;
                    continue;
                }
                nbrhd[nhdsze + snhdsze++] = node;
                marker[node] = -1;
                k = xadj[node];
                kstop = xadj[node+1];
                while (k < kstop) {
                    node = adjncy[k++];
                    if(node >= nvtxs) {
                        node -= nvtxs;
                        if(node < nvtxs)
                        {
                            k = xadj[node];
                            kstop = xadj[node+1];
                            continue;
                        }
                        break;
                    }
                    if(marker[node]) continue;
                    reach[rchsze + srchsze++] = node;
                    Deg += qsize[node];
                    marker[node] = 1;
                } /* next k */
            } /* next j */

            degree[tmpRoot] = SetPower + Deg - 1;
            counter++;
            if(srchsze)
                for(j=0; j<srchsze; j++)
                    marker[reach[rchsze + j]] = 0;
            if(snhdsze)
                for(j=0; j<snhdsze; j++)
                    marker[nbrhd[nhdsze + j]] = 0;
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

    return 0;
}
/*****************************************************************/
int spSetMatrix(TBCCSMatrix &matrix, const int* mesh, int nelmnts, int elmsze, int nvtxs, int blksz)
{
    int i, j, m, k, n, nedges, node, nnz, len;
    vector<int> nptr, mark, nind;

    matrix.nvtxs = nvtxs;

    /* allocate & set all element adjacency lenghts to null */
    nptr.resize(nvtxs + 1, 0);

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
    nind.resize(node);

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

    mark.resize(nvtxs, -1);
    matrix.aptrs.resize(nvtxs + 1);

    /* calculate memory for adjacency structure */
    for(nedges=i=0; i<nvtxs; i++) {
        matrix.aptrs[i] = nedges;
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
    matrix.aptrs[nvtxs] = nedges;
    matrix.ainds.resize(nedges);

    /* save adjacency structure */
    fill_n(mark.begin(), size_t(nvtxs), -1);
    for(nedges=i=0; i<nvtxs; i++) {
        node = nptr[i+1];
        for(j=nptr[i]; j<node; j++) {
            m = elmsze*nind[j];
            for(k=0; k<elmsze; k++) {
                n = mesh[m++];
                if(mark[n] != i) {
                    mark[n] = i;
                    matrix.ainds[nedges++] = n;
                }
            }
        }
        assert(nedges - matrix.aptrs[i] > 0);
        //     iqsort(nedges-aptrs[i], &ainds[aptrs[i]]);
        qsort(&matrix.ainds[matrix.aptrs[i]], size_t(nedges - matrix.aptrs[i]), sizeof(int), [](const void *x1, const void *x2) { return *(const int*)x1 - *(const int*)x2; });
    }
    nnz = matrix.aptrs[matrix.nvtxs];
    matrix.blksze = blksz;
    len = nnz * matrix.blksze * matrix.blksze;
    matrix.avals.resize(len, 0.);
    return 0;
}
/*****************************************************************/
int spMulMatrix(TBCCSMatrix &matrix, const vector<double> &x, vector<double> &y)
{
    int i, j, bi, bj, strt, stop, node, sqrblk, ilim;
    const double *cx, *a, *ca;
    double *cy, *cv;

    sqrblk = matrix.blksze * matrix.blksze;
    for (ilim = matrix.nvtxs * matrix.blksze - 8, cv = y.data(), i = 0; i <= ilim; i += 8, cv += 8)
        cv[0] = cv[1] = cv[2] = cv[3] = cv[4] = cv[5] = cv[6] = cv[7] = 0;
    for(; i < matrix.nvtxs * matrix.blksze; i++)
        y[i] = 0;

    cx = x.data();

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
int spMulMatrix(TBCCSMatrix &matrix, double K)
{
    int len = matrix.aptrs[matrix.nvtxs] * matrix.blksze * matrix.blksze;

    for (auto i = 0; i < len; i++)
        matrix.avals[i] *= K;
    return 0;
}
/***********************************************************************/
int merge(vector<int> &a, int alen, vector<int> &b, int offset, int blen, vector<int> &t)
{
    int   i, j, k;

    i=j=k=0;

    while( (i < alen) && (j < blen) ) {
        if(a[i] < b[offset + j]) {
            t[k] = a[i++];
        } else if(a[i] == b[offset + j]) {
            t[k] = a[i++];
            j++;
        } else {
            t[k] = b[offset + j++];
        }
        k++;
    }

    while(i < alen) {
        t[k++] = a[i++];
    }

    while(j < blen) {
        t[k++] = b[offset + j++];
    }
    return k;
}
/********************************************************************/
static int funInSymbolic(TBCCSFactor &factor, const vector<int> &aptrs, const vector<int> &ainds)
{
    int i, j, k, node, lsize, setsze, strt, stop, nvtxs, ispace = 0;
    vector<int> linds, mrglnk, stack, nodeset;

    nvtxs = factor.nvtxs;
    factor.xlvals.resize(nvtxs + 1);
    factor.xlinds.resize(nvtxs);

    /* allocate work memory */
    mrglnk.resize(nvtxs);
    stack.resize(nvtxs);
    nodeset.resize(nvtxs);

    /* allocate initial buffer for compressed row indices */
    lsize = 20 * nvtxs;
    linds.resize(lsize);

    /* must be */
    factor.xlvals[0] = 0;

    /* set all lists as empty */
    for(i=0; i<nvtxs; i++) {
        mrglnk[i] = nvtxs;
    }

    for(k=0; k<nvtxs; k++) {
        node = factor.perm[k];
        assert(node < nvtxs);
        strt = aptrs[node];
        stop = aptrs[node+1];

        /* isolated root */
        if(strt == stop) {
            factor.xlinds[k] = ispace;
            factor.xlvals[k+1] = factor.lspace;
            continue;
        }

        /* consolidate A(*,k) */
        setsze = 0;
        for(j=strt; j<stop; j++) {
            node = factor.invp[ainds[j]];
            assert(node < nvtxs);
            if(node <= k) continue;
            nodeset[setsze++] = node;
        }
        //     iqsort(setsze, nodeset);
        qsort(nodeset.data(), size_t(setsze), size_t(sizeof(int)), [](const void *x1, const void *x2) { return *(const int*)x1 - *(const int*)x2; });

        /* inspect kids list and consolidate L(*,k) */
        for(i=mrglnk[k]; i<nvtxs; i=mrglnk[i]) {
            /* i must be kid of k */
            assert(linds[factor.xlinds[i]] == k);
            /* merge two sets */
            setsze = merge(nodeset, setsze, linds, factor.xlinds[i]+1, factor.xlvals[i+1] - factor.xlvals[i] - 1, stack);
            /* now set is in stack */
            swap(stack, nodeset);
            /* now set is in nodeset */
        }

        /* may be compression? */
        if(k > 0 && linds[factor.xlinds[k-1]] == k &&
            setsze == (factor.xlvals[k] - factor.xlvals[k-1]-1))
        {
            /* it's indistinguishable node */
            factor.xlinds[k] = factor.xlinds[k-1] + 1;
            factor.lspace += setsze;
            factor.xlvals[k+1] = factor.lspace;

        } else {

            if(ispace + setsze > lsize) {
                lsize = 2*lsize;
                linds.resize(lsize);
            }

            j = ispace;
            for(i=0; i<setsze; i++) {
                linds[j++] = nodeset[i];
            }

            factor.xlinds[k] = ispace;
            ispace += setsze;
            factor.lspace += setsze;
            factor.xlvals[k+1] = factor.lspace;

        }

        if(setsze > 1) {
            i = linds[factor.xlinds[k]];
            mrglnk[k] = mrglnk[i];
            mrglnk[i] = k;
        }

    } /* next k */

    assert(factor.xlvals[nvtxs] == factor.lspace);

    /* truncate row indices array */
    factor.linds = linds;

    return 0;
}
/********************************************************************/
int spOrder(TBCCSFactor &factor, TBCCSMatrix &matrix, bool &aborted)
{
    int i, j, k, l, nvtxs, error, nedges, strt, stop;
    vector<int> xadj, adjncy;

    nvtxs = matrix.nvtxs;

    /* allocate standart graph structure without diagonal indices */
    nedges = matrix.aptrs[nvtxs] - nvtxs;
    xadj.resize(nvtxs + 1);
    adjncy.resize(nedges);

    /* form (xadj,adjncy) from (aptrs,ainds) */

    msg->setProcess(ProcessCode::PreparingSystemEquation, 0, nvtxs - 1, 10);
    for(l=i=0; i<nvtxs; i++)
    {

        msg->addProgress();
        if (aborted)
            return 1;

        xadj[i] = l;
        strt = matrix.aptrs[i];
        stop = matrix.aptrs[i+1];
        for(j=strt; j<stop; j++) {
            k = matrix.ainds[j];
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
    factor.perm.resize(nvtxs);
    factor.invp.resize(nvtxs);


    error = genqmd(nvtxs, xadj, adjncy, factor.perm, factor.invp);

    if (error != 0)
        return error;

    /* perform symbolic factorization */
    error = funInSymbolic(factor, matrix.aptrs, matrix.ainds);

    return factor.error = error;
}
/*************************************************************/
/* GENERAL SPARSE SYMMETRIC SCHEME, CLASSIC FUN-IN ALGORITHM */
/*************************************************************/
int gsfctb(TBCCSFactor &factor, TBCCSMatrix &matrix, double tol, bool &aborted)
{
    int error, i, j, k, n, node, strt, stop, knew, isub, bi, bj, bk, denter, fenter, colsze, blksze = matrix.blksze, offset, offset1, offset2, offset3;
    double value, dmin, dmax, *FF, *LL, sval;
    vector<double*> translate(matrix.nvtxs);
    vector<int> first(matrix.nvtxs), link(matrix.nvtxs, matrix.nvtxs);
    vector<double> DD(MAXBLKSZE * MAXBLKSZE), rd(MAXBLKSZE), HH(MAXBLKSZE * MAXBLKSZE);

    /* check possible HH,DD,rd overflow */
    assert(blksze < MAXBLKSZE);


    /* sizes of diagonal & subdiagonal blocks */
    fenter = blksze * blksze;
    denter = (blksze * (blksze + 1)) / 2;

    /* setup initial diagonal range */
    dmin =  DBL_MAX;
    dmax = -DBL_MAX;

    /* main routine */
    msg->setProcess(ProcessCode::FactorizationSystemEquation, 0, matrix.nvtxs - 1, 10);
    for (error = j = 0; j < matrix.nvtxs; j++)
    {
        msg->addProgress();
        if (aborted)
        {
            error = 1;
            break;
        }

        strt = factor.xlvals[j];
        stop = factor.xlvals[j + 1];

        /* set entries into factored column */
        isub = factor.xlinds[j];
        for (i = strt, offset = fenter * strt; i < stop; i++, offset += fenter)
        {
            translate[factor.linds[isub++]] = &factor.lvals[offset];
            for (k = 0; k < fenter; k++)
                factor.lvals[offset + k] = 0.0;
        }

        /* form A structure */
        node = factor.perm[j];
        strt = matrix.aptrs[node];
        stop = matrix.aptrs[node+1];

        for (i = strt, offset = fenter * strt; i < stop; i++, offset += fenter)
        {
            node = factor.invp[matrix.ainds[i]];
            if (node < j)
                continue;

            for (k = 0; k < fenter; k++)
                if (node == j)
                    DD[k] = matrix.avals[offset + k];
                else
                    translate[node][k] = matrix.avals[offset + k];
        }

        /* merge with previous columns */
        for (k = link[j]; k < matrix.nvtxs; k = knew)
        {
            knew = link[k];
            strt = first[k];
            LL = &factor.lvals[fenter * strt];

            /* calculate HH */
            for (bi = 0, offset = 0; bi < blksze; bi++, offset += blksze)
            {
                value = double(factor.svals[blksze * k + bi]);
                for (bj = 0; bj < blksze; bj++)
                    HH[offset + bj] = LL[offset + bj] * value;
            }

            /* DD -= LL * HH */
            for (bi = 0, offset = 0; bi < blksze; bi++, offset += blksze)
            {
                for(bj = 0, offset1 = 0; bj < blksze; bj++, offset1 += blksze)
                {
                    value = HH[offset1 + bi];
                    for (bk = bi; bk < blksze; bk++)
                        DD[offset + bk] -= LL[offset1 + bk] * value;
                }
            }

            strt++;
            stop = factor.xlvals[k + 1];
            if (strt >= stop)
                continue;

            /* update links */
            first[k] = strt;
            i = factor.xlinds[k] + strt - factor.xlvals[k];
            isub = factor.linds[i];
            link[k] = link[isub];
            link[isub] = k;

            /* merge subdiagonal */
            for (n = strt; n < stop; n++)
            {

                LL += fenter;
                FF = translate[factor.linds[i++]];

                /* FF -= LL * HH */
                for (bi = 0; bi < blksze; bi++)
                {
                    // ph = HH;
                    // pl = LL;
                    for (bj = 0, offset1 = 0; bj < blksze; bj++, offset1 += blksze)
                    {
                        value = HH[offset1 + bi];
                        for (bk = 0; bk < blksze; bk++)
                            FF[bk] -= LL[offset1 + bk] * value;
                    }
                    FF += blksze;
                }


            } /* next merged block */

        } /* next merged columns */


        /* diagonal block factorization */
        for (bi = 0, offset = 0; bi < blksze; bi++, offset += blksze)
        {
            /* accumulating */
            for (bj = 0, offset1 = 0; bj < bi; bj++, offset1 += blksze)
            {
                value = DD[offset1 + bi] * double(factor.svals[blksze*j + bj]);
                for (bk = bi; bk<blksze; bk++)
                    DD[offset + bk] -= DD[offset1 + bk] * value;
            }
            value = DD[offset + bi];

            /* check indefinity */
            if (value < 0.0)
            {
                value = -value;
                //matrix.hyperbolic++;
                sval = -1.0;
                factor.svals[blksze*j + bi] = -1;
            }
            else
            {
                sval = 1.0;
                factor.svals[blksze*j + bi] = 1;
            }

            /* test diagonal value */
            if (value < dmin) dmin = value;
            if (value > dmax) dmax = value;
            /* check numerical stability */
            if (value < tol)
            {
                error = 1;
                break;
            }

            value = sqrt(value);
            DD[offset + bi] = value;
            value = sval / value;
            rd[bi] = value;

            /* scale subdiagonal */
            for (bk = bi + 1; bk < blksze; bk++)
                DD[offset + bk] *= value;
        }

        /* save factored diagonal block */
        //fcolptr = DD;
        colsze = blksze;
        for (bi = 0, offset = denter*j, offset1 = 0; bi < blksze; bi++, offset += colsze, offset1 += blksze)
        {
            for (bj = bi; bj < blksze; bj++)
                factor.dvals[offset + bj] = DD[offset1 + bj];
            colsze--;
            //fcolptr += blksze;
        }

        /* do we must factor subdiagonal? */
        strt = factor.xlvals[j];
        stop = factor.xlvals[j + 1];
        if (strt >= stop)
            continue;

        /* update links */
        first[j] = strt;
        i = factor.xlinds[j];
        isub = factor.linds[i];
        link[j] = link[isub];
        link[isub] = j;

        /* calculate HH */
        for (bi = 0, offset = 0, offset1 = 0; bi < blksze; bi++, offset += blksze, offset1 += blksze)
        {
            value = double(factor.svals[blksze*j + bi]);
            for (bj = bi + 1; bj < blksze; bj++)
                HH[offset1 + bj] = DD[offset + bj] * value;
        }

        /* scale subdiagonal */
        for (n = strt, offset = fenter * strt; n < stop; n++, offset += fenter)
        {
            for (bi = 0, offset1 = offset; bi<blksze; bi++, offset1 += blksze)
            {
                for (bj = 0, offset2 = 0, offset3 = 0; bj < bi; bj++, offset2 += blksze, offset3 += blksze)
                {
                    value = HH[offset2 + bi];
                    for (bk = 0; bk < blksze; bk++)
                        factor.lvals[offset1 + bk] -= factor.lvals[offset + offset3 + bk] * value;
                }
                value = rd[bi];
                for (bk = 0; bk < blksze; bk++)
                    factor.lvals[offset1 + bk] *= value;
            }
        }

    } /* next factored columns */
    msg->stopProcess();

    return error;
}
/*********************************************************************/
int spFactor(TBCCSFactor &factor, TBCCSMatrix &matrix, double eps, bool &aborted)
{
    int nvtxs, blksze, memsze, denter;

    /* test arguments */
    if((matrix.aptrs.size() == 0) ||
        (matrix.ainds.size() == 0) ||
        (matrix.avals.size() == 0) ||
        (matrix.blksze <= 0) ||
        (matrix.blksze >= MAXBLKSZE) ||
        (matrix.nvtxs != factor.nvtxs) ||
        (factor.error != 0) )
        return 1;

    /* load data from structure */
    nvtxs  = matrix.nvtxs;
    factor.blksze = blksze = matrix.blksze;

    /* allocate main lvals array */
    denter = (blksze * (blksze + 1)) / 2;
    memsze = factor.lspace * blksze * blksze + nvtxs * denter;
    ///////////////////////////
    //factor.vpool.resize(memsze);
    ///////////////////////////
    factor.dvals.resize(denter * nvtxs); // =  &factor.vpool[0];
    factor.lvals.resize(memsze - denter * nvtxs); // = &factor.vpool[denter * nvtxs];

    /* allocate diagonal flags array */
    factor.svals.resize(nvtxs * blksze);

    factor.error = gsfctb(factor, matrix, eps, aborted);

    return factor.error;
}
/********************************************************************/
void permrv(vector<double> &rhs, const vector<int> &order, int nvtxs, int blksze)
{
    int node;

    for(auto i = 0; i < nvtxs; i++)
    {
        node = order[i];
        if(node == i)
            continue;
        while(node < i)
            node = order[node];

        for (auto j = 0; j < blksze; j++)
            swap(rhs[blksze*node + j], rhs[blksze*i + j]);
    }

} /* end permrv */
/*******************************************************************/
/*                                                                 */
/*             GENERAL SPARSE SYMMETRIC SYSTEM                     */
/*                                                                 */
/*******************************************************************/
void gsslvb(TBCCSFactor &factor, vector<double> &rght)
{
    int i, j, k, brow, bcol, colsze, strt, stop, denter, fenter, run, offset, offset1, offset2;
    double value;


    fenter = factor.blksze * factor.blksze;
    denter = (factor.blksze * (factor.blksze + 1)) / 2;


    /* === forward substitution === */
    for (j=offset=0; j<factor.nvtxs; j++, offset +=denter) {

        /* dense lover diagonal system */
        colsze = factor.blksze;
        for (run=bcol=offset1 = 0; bcol<factor.blksze; bcol++, offset1 += colsze) {
            value = rght[bcol + j * factor.blksze];
            if (value != 0.0) {
                rght[bcol + j * factor.blksze] = value = (value / factor.dvals[offset + offset1 + bcol]);
                run = 1;
                for (brow=bcol+1; brow<factor.blksze; brow++) {
                    rght[brow + j * factor.blksze] -= factor.dvals[offset + offset1 + brow] * value;
                }
            }
            colsze--;
        }

        if (run) {
            strt = factor.xlvals[j];
            stop = factor.xlvals[j+1];
            if (strt < stop) {
                i = factor.xlinds[j];
                for (k=strt, offset1 = fenter*strt; k<stop; k++, offset1 += fenter) {
                    for (bcol=0, offset2 = offset1; bcol<factor.blksze; bcol++, offset2 += factor.blksze) {
                        value = rght[bcol + j * factor.blksze];
                        for (brow=0; brow<factor.blksze; brow++) {
                            rght[brow + factor.blksze * factor.linds[i]] -= factor.lvals[offset2 + brow] * value;
                        }
                    }
                    i++;
                }
            }
        }
    }

    /* === backward substitution === */
    for (j=factor.nvtxs-1, offset = factor.nvtxs*denter - factor.blksze; j>=0; j--, offset -= denter) {
        strt = factor.xlvals[j];
        stop = factor.xlvals[j+1];

        if (strt < stop) {
            i = factor.xlinds[j];
            for (k=strt, offset1 = fenter*strt; k<stop; k++, offset1 += fenter) {
                for (brow=0, offset2 = offset1; brow<factor.blksze; brow++, offset2 += factor.blksze) {
                    value = 0.0;
                    for (bcol=0; bcol<factor.blksze; bcol++) {
                        value += factor.lvals[offset2 + bcol] * rght[bcol + factor.blksze * factor.linds[i]];
                    }
                    rght[brow + j*factor.blksze] -= value;
                }
                i++;
            }
        }

        /* dense upper diagonal system */
        colsze = 0;
        for (brow=factor.blksze-1, offset1 = 0; brow>=0; brow--, offset1 -= colsze) {
            value = 0.0;
            for(bcol=brow+1; bcol<factor.blksze; bcol++) {
                value += rght[bcol + j*factor.blksze] * factor.dvals[offset + offset1 + bcol];
            }
            rght[brow + j*factor.blksze] = (rght[brow + j*factor.blksze] - value) /  factor.dvals[offset + offset1 + brow];
            colsze++;
        }
    }
}
/*********************************************************************/
int spSolve(TBCCSFactor &factor, vector<double> &rhs)
{
    msg->setProcess(ProcessCode::SolutionSystemEquation);
    /* direct permutation */
    permrv(rhs, factor.perm, factor.nvtxs, factor.blksze);
    /* choose optimised variant or universal */
    gsslvb(factor, rhs);
    /* invert permutation */
    permrv(rhs, factor.invp, factor.nvtxs, factor.blksze);
    msg->stop();
    return 0;
}
/*********************************************************************/
double spGetElem(TBCCSMatrix &matrix, int i, int j)
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
void spSetElem(TBCCSMatrix &matrix, int i, int j, double val)
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
void spAddElem(TBCCSMatrix &matrix, int i, int j, double val)
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


