#include "shape.h"
#include "msg/msg.h"

//--------------------------------------------------------------
//                  Решение СЛАУ методом Гаусса
//--------------------------------------------------------------
bool TShape::solve(matrix<double>& matr, vector<double>& result, double eps)
{
    double koff1,
           koff2,
           sum;
    unsigned i;

    for (i = 0; i < matr.size1() - 1; i++)
    {
        if (abs(matr[i][i]) < eps)
            for (unsigned l = i + 1; l < matr.size1(); l++)
            {
                if (abs(matr[l][i]) < eps)
                    continue;
                for (unsigned j = 0; j < matr.size1() + 1; j++)
                    swap(matr[l][j], matr[i][j]);
            }
        koff1 = matr[i][i];
        for (unsigned j = i + 1; j < matr.size1(); j++)
        {
            if (abs(koff2 = matr[j][i]) < eps)
                continue;
            for (unsigned k = i; k < matr.size1() + 1; k++)
                matr[j][k] -= koff2*matr[i][k] / koff1;
        }
    }
    if (abs(matr[matr.size1() - 1][matr.size1() - 1]) < eps)
        return false;
    result[matr.size1() - 1] = matr[matr.size1() - 1][matr.size1()] / matr[matr.size1() - 1][matr.size1() - 1];
    for (unsigned k = 0; k < matr.size1() - 1; k++)
    {
         i = matr.size1() - k - 2;
         sum = matr[i][matr.size1()];
         for (unsigned j = i + 1; j < matr.size1(); j++)
             sum -= result[j] * matr[i][j];
         if (abs(matr[i][i]) < eps)
             return false;
         result[i] = sum / matr[i][i];
    }
    return true;
}
//--------------------------------------------------------------
void TShape::create(matrix<double>& px)
{
    matrix<double> A(size, size + 1);
    vector<double> res(size);

    x = px;
    c.resize(size, size);
    for (unsigned j = 0; j < size; j++)
    {
        for (unsigned i = 0; i < size; i++)
        {
            for (unsigned k = 0; k < size; k++)
                A[i][k] = shape_coeff(i)[k];
            A[i][size] = (i == j) ? 1.0 : 0.0;
        }
        if (!solve(A, res))
            throw INCORRECT_FE_ERR;
        for (unsigned i = 0; i < size; i++)
            c[i][j] = res[i];
    }
}
//--------------------------------------------------------------
