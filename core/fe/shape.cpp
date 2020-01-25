#include <Eigen/Dense>
#include "shape.h"
#include "msg/msg.h"

//--------------------------------------------------------------
void TShape::create(matrix<double>& px)
{
    using namespace Eigen;

    MatrixXd A(size, size);
    VectorXd b(size);

    x = px;
    c.resize(size, size);
    for (unsigned i = 0; i < size; i++)
    {
        for (unsigned j = 0; j < size; j++)
        {
            for (unsigned k = 0; k < size; k++)
                A(j, k) = shape_coeff(j, k);
            b(j) = (i == j) ? 1.0 : 0.0;
        }
        if (A.determinant() == 0.0)
            throw INCORRECT_FE_ERR;
        b = A.colPivHouseholderQr().solve(b);
        for (unsigned j = 0; j < size; j++)
            c(j, i) = b(j);
    }
}
//--------------------------------------------------------------
