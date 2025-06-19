#include <mkl_pardiso.h>
#include "psolver.h"
#include "mesh/mesh.h"

extern TMessenger *msg;

void TPardisoSolver::setMatrix(TMesh *mesh, bool isDynamic)
{
    stiffness.setMatrix(mesh);
    if (isDynamic)
    {
        mass.setMatrix(mesh);
        damping.setMatrix(mesh);
    }
    load.resize(mesh->getNumVertex() * mesh->getFreedom(), 0);
    boundary_conditions.resize(mesh->getNumVertex() * mesh->getFreedom(), {false, 0});
}

int solvePardiso(int n, int num_procs, int *ia, int *ja, double *a, double *b)
{
    int nnz = ia[n];
    int mtype = -2; /* Real symmetric matrix */
    /* RHS and solution vectors. */
    double *x;
    int nrhs = 1; /* Number of right hand sides. */
    /* Internal solver memory pointer pt, */
    /* 32-bit: int pt[64]; 64-bit: long int pt[64] */
    /* or void *pt[64] should be OK on both architectures */
    void *pt[64];
    /* Pardiso control parameters. */
    int iparm[64], maxfct, mnum, phase, error, msglvl;
    /* Auxiliary variables. */
    int i;
    double ddum; /* Double dummy */
    int idum; /* Integer dummy. */

    /* -------------------------------------------------------------------- */
    /* .. Setup Pardiso control parameters. */
    /* -------------------------------------------------------------------- */
    pardisoinit(pt, &mtype, iparm);
    iparm[2] = num_procs;
    maxfct = 1; /* Maximum number of numerical factorizations. */
    mnum = 1; /* Which factorization to use. */
    //msglvl = 1; /* Print statistical information */
    msglvl = 0; /* Print statistical information */
    error = 0; /* Initialize error flag */
    /* -------------------------------------------------------------------- */
    /* .. Convert matrix from 0-based C-notation to Fortran 1-based */
    /* notation. */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < n+1; i++) {
        ia[i] += 1;
    }
    for (i = 0; i < nnz; i++) {
        ja[i] += 1;
    }
    /* -------------------------------------------------------------------- */
    /* .. Reordering and Symbolic Factorization. This step also allocates */
    /* all memory that is necessary for the factorization. */
    /* -------------------------------------------------------------------- */
    phase = 11;
    pardiso(pt, &maxfct, &mnum, &mtype, &phase,  &n, a, ia, ja, &idum, &nrhs, iparm, &msglvl, &ddum, &ddum, &error);
    if (error != 0) {
//        printf("\nERROR during symbolic factorization: %d", error);
        return 1;
    }
//    printf("\nReordering completed ... ");
//    printf("\nNumber of nonzeros in factors = %d", iparm[17]);
//    printf("\nNumber of factorization MFLOPS = %d", iparm[18]);
    /* -------------------------------------------------------------------- */
    /* .. Numerical factorization. */
    /* -------------------------------------------------------------------- */
    phase = 22;
    pardiso(pt, &maxfct, &mnum, &mtype, &phase, &n, a, ia, ja, &idum, &nrhs, iparm, &msglvl, &ddum, &ddum, &error);
    if (error != 0) {
//        printf("\nERROR during numerical factorization: %d", error);
        return 2;
    }
//    printf("\nFactorization completed ...\n ");
    /* -------------------------------------------------------------------- */
    /* .. Back substitution and iterative refinement. */
    /* -------------------------------------------------------------------- */
    phase = 33;
    iparm[7] = 1; /* Max numbers of iterative refinement steps. */
    /* Set right hand side to one. */
    if ((x = (double*)malloc(n * sizeof(double))) == NULL)
    {
        // Out of memory
        return 4;
    }
    pardiso(pt, &maxfct, &mnum, &mtype, &phase, &n, a, ia, ja, &idum, &nrhs, iparm, &msglvl, b, x, &error);
    if (error != 0) {
//        printf("\nERROR during solution: %d", error);
        free(x);
        return 3;
    }
//    printf("\nSolve completed ... ");
//    printf("\nThe solution of the system is: ");
    for (i = 0; i < n; i++) {
//        printf("\n x [%d] = % f", i, x[i] );
        b[i] = x[i];
    }
//    printf ("\n");
    free(x);
    /* -------------------------------------------------------------------- */
    /* .. Convert matrix back to 0-based C-notation. */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < n+1; i++) {
        ia[i] -= 1;
    }
    for (i = 0; i < nnz; i++) {
        ja[i] -= 1;
    }
    /* -------------------------------------------------------------------- */
    /* .. Termination and release of memory. */
    /* -------------------------------------------------------------------- */
    phase = -1; /* Release internal memory. */
    pardiso(pt, &maxfct, &mnum, &mtype, &phase, &n, &ddum, ia, ja, &idum, &nrhs, iparm, &msglvl, &ddum, &ddum, &error);
    return 0;
}

bool TPardisoSolver::solve(vector<double> &r, double, bool&)
{
    bool ret;

    msg->setProcess(ProcessCode::SolutionSystemEquation);
    ret = solvePardiso(stiffness.size, std::thread::hardware_concurrency(), stiffness.ia.data(), stiffness.ja.data(), stiffness.a.data(), load.data());
    msg->stop();
    if (ret != 0)
        return false;
    r = load;
    return true;
}
