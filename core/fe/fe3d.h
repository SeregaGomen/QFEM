#ifndef FE3D_H
#define FE3D_H

#include "fe.h"

//----------------------------------------------------
//             Трехмерный конечный элемент
//----------------------------------------------------
template <class T> class TFE3D : public TFE
{
protected:
    matrix<double> elastic_matrix(void)
    {
        return {{ e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), 0.0, 0.0, 0.0 },
            { m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), 0.0, 0.0, 0.0},
            { m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), m / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), 0.0, 0.0, 0.0},
            { 0.0, 0.0, 0.0, 0.5 * (1.0 - 2.0 * m) / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), 0.0, 0.0},
            { 0.0, 0.0, 0.0, 0.0, 0.5 * (1.0 - 2.0 * m) / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m), 0.0},
            { 0.0, 0.0, 0.0, 0.0, 0.0, 0.5 * (1.0 - 2.0 * m) / (1.0 - m) * e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m) }};
    }
public:
    TFE3D(void) : TFE()
    {
        freedom = 3;
        shape = new T();
    }
    virtual ~TFE3D() = default;
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> b(6, shape->size * freedom),
                       stress,
                       strain;

        for (unsigned i = 0; i < shape->size; i++)
        {
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(3, j * freedom + 1) = b(5, j * freedom + 2) = shape->shape_dx(i, j);
                b(1, j * freedom + 1) = b(3, j * freedom + 0) = b(4, j * freedom + 2) = shape->shape_dy(i, j);
                b(2, j * freedom + 2) = b(4, j * freedom + 1) = b(5, j * freedom + 0) = shape->shape_dz(i, j);
            }
            strain = b * u;
            stress = elastic_matrix() * strain;
            for (unsigned j = 0; j < 6; j++)
            {
                res(j, i) += strain(j, 0);
                res(j + 6, i) += stress(j, 0);
            }
        }
    }
    void generate(bool isStatic = true)
    {
        double jacobian;
        matrix<double> b(6, freedom * shape->size),
                       c(freedom, freedom * shape->size),
                       jacobi,
                       inverted_jacobi;

        K.resize(freedom * shape->size, freedom * shape->size);
        load.resize(freedom * shape->size, 1);
        if (!isStatic)
        {
            M.resize(freedom * shape->size, freedom * shape->size);
            D.resize(freedom * shape->size, freedom * shape->size);
        }
        // Интегрирование по по формуле Гаусса
        for (unsigned i = 0; i < shape->w.size(); i++)
        {
            // Матрица Якоби
            jacobi = shape->jacobi(i);

            // Якобиан
            jacobian = det(jacobi);

            // Обратная матрица Якоби
            inverted_jacobi = inv(jacobi);

            // Матрица градиентов
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(3, j * freedom + 1) = b(5, j * freedom + 2) = inverted_jacobi(0, 0) * shape->shape_dxi(i, j) + inverted_jacobi(0, 1) * shape->shape_deta(i, j) + inverted_jacobi(0, 2) * shape->shape_dpsi(i, j);
                b(1, j * freedom + 1) = b(3, j * freedom + 0) = b(4, j * freedom + 2) = inverted_jacobi(1, 0) * shape->shape_dxi(i, j) + inverted_jacobi(1, 1) * shape->shape_deta(i, j) + inverted_jacobi(1, 2) * shape->shape_dpsi(i, j);
                b(2, j * freedom + 2) = b(4, j * freedom + 1) = b(5, j * freedom + 0) = inverted_jacobi(2, 0) * shape->shape_dxi(i, j) + inverted_jacobi(2, 1) * shape->shape_deta(i, j) + inverted_jacobi(2, 2) * shape->shape_dpsi(i, j);
                if (!isStatic)
                    c(0, j * freedom + 0) = c(1, j * freedom + 1) = c(2, j * freedom + 2) = shape->shape(i, j);
            }
            K += (transpose(b) * elastic_matrix() * b) * shape->w[i] * abs(jacobian);
            // Вычисление температурной нагрузки
            if (temperature != 0.0 && alpha != 0.0)
                load += transpose(b) * elastic_matrix() * vector<double>{temperature * alpha, temperature * alpha, temperature * alpha, 0, 0, 0} * shape->w[i] * abs(jacobian);
            if (!isStatic)
            {
                M += (transpose(c) * c) * density * shape->w[i] * density * abs(jacobian);
                D += (transpose(c) * c) * damping * shape->w[i] * density * abs(jacobian);
            }
        }
    }
};


#endif // FE3D_H
