#ifndef FE2D_H
#define FE2D_H

#include "fe.h"

//----------------------------------------------------
//             Двумерный конечный элемент
//----------------------------------------------------
template <class T> class TFE2D : public TFE
{
protected:
    matrix<double> elastic_matrix(void)
    {
        return { { e / (1.0 - m * m),  m * e / (1.0 - m * m),    0 },
                 { m * e / (1.0 - m * m),    e / (1.0 - m * m),  0 },
                 { 0,    0,    0.5 * (1.0 - m) * e / (1.0 - m * m) } };
    }
public:
    TFE2D(void) : TFE()
    {
        freedom = 2;
        shape = new T();
    }
    virtual ~TFE2D() = default;
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> b(3, shape->size * freedom),
                       stress,
                       strain;

        for (unsigned i = 0; i < shape->size; i++)
        {
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(2, j * freedom + 1) = shape->shape_dx(i, j);
                b(1, j * freedom + 1) = b(2, j * freedom + 0) = shape->shape_dy(i, j);
            }
            strain = b * u;
            stress = elastic_matrix() * strain;
            for (unsigned j = 0; j < 3; j++)
            {
                res(j, i) += strain(j, 0);
                res(j + 3, i) += stress(j, 0);
            }
        }
    }
    void generate(bool isStatic = true)
    {
        double jacobian;
        matrix<double> b(3, freedom * shape->size),
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
        // Интегрирование по формуле Гаусса
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
                b(0, j * freedom + 0) = b(2, j * freedom + 1) = inverted_jacobi(0, 0) * shape->shape_dxi(i, j) + inverted_jacobi(0, 1) * shape->shape_deta(i, j);
                b(1, j * freedom + 1) = b(2, j * freedom + 0) = inverted_jacobi(1, 0) * shape->shape_dxi(i, j) + inverted_jacobi(1, 1) * shape->shape_deta(i, j);
                if (!isStatic)
                    c(0, j * freedom + 0) = c(0, j * freedom + 1) = shape->shape(i, j);
            }

            // Вычисление локальной матрицы жесткости
            K += (transpose(b) * elastic_matrix() * b) * shape->w[i] * thickness * abs(jacobian);
            // Вычисление температурной нагрузки
            if (temperature != 0.0 && alpha != 0.0)
                load += transpose(b) * elastic_matrix() * vector<double>{temperature * alpha, temperature * alpha, 0} * shape->w[i] * abs(jacobian);
            if (!isStatic)
            {
                M += (transpose(c) * c) * density * shape->w[i] * thickness * abs(jacobian);
                D += (transpose(c) * c) * damping * shape->w[i] * thickness * abs(jacobian);
            }
        }
    }
};


#endif // FE2D_H
