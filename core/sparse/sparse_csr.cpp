#include "sparse_csr.h"
#include "mesh/mesh.h"

void TCSRMatrix::setMatrix(TMesh *mesh)
{
    auto freedom = mesh->getFreedom(),
         nnz = 0u;

    a.clear();
    ia.clear();
    ja.clear();

    size = mesh->getNumVertex() * freedom;
    ia.resize(size + 1);
    ja.reserve(size);

    ia[0] = 0;
    for (auto i = 0; i < size; i++)
    {
        for (auto j = i%freedom; j < mesh->getMeshMap()[i/freedom].size()*freedom; j++)
        {
            ja.push_back(mesh->getMeshMap()[i/freedom][j/freedom]*freedom + j%freedom);
            nnz++;
        }
        ia[i+1] = nnz;
    }
    a.resize(ja.size(), 0);
}

int TCSRMatrix::findPos(int i, int j)
{
    if (i >= 0 && i < size)
    {
        int pos = ia[i];
        for (auto k = 0; k < ia[i+1] - ia[i]; k++)
            if (ja[pos + k] == j)
                return pos + k;
    }
    return -1;
}

void TCSRMatrix::setElem(int i, int j, double value)
{
    int pos;

    if (i <= j)
    {
        if ((pos = findPos(i, j)) == -1)
            throw ErrorCode::EIndex;
        else
            a[pos] = value;
    }
}

void TCSRMatrix::addElem(int i, int j, double value)
{
    int pos;

    if (i <= j)
    {
        if ((pos = findPos(i, j)) == -1)
            throw ErrorCode::EIndex;
        else
            a[pos] += value;
    }
}


double TCSRMatrix::getElem(int i, int j)
{
    int pos;

    if (i <= j)
    {
        if ((pos = findPos(i, j)) == -1)
            return 0;
        else
            return a[pos];
    }
    return getElem(j, i);
}

void TCSRMatrix::clearRow(int index)
{
    auto pos = findPos(index, index);

    if (pos != -1)
        for (auto k = 0; k < ia[index+1] - ia[index]; k++)
            a[pos+k] = 0;

}

void TCSRMatrix::clearCol(int index)
{
    for (auto i = 0; i <= index; i++)
        if (getElem(i, index) != 0)
            setElem(i, index, 0);
}
