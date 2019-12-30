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
        matrix<double> d(6, 6);

        d(0, 0) = 1.0; d(0, 1) = m / (1.0 - m); d(0, 2) = m / (1.0 - m); d(0, 3) = 0.0; d(0, 4) = 0.0; d(0, 5) = 0.0;
        d(1, 0) = m / (1.0 - m); d(1, 1) = 1.0; d(1, 2) = m / (1.0 - m); d(1, 3) = 0.0; d(1, 4) = 0.0; d(1, 5) = 0.0;
        d(2, 0) = m / (1.0 - m); d(2, 1) = m / (1.0 - m); d(2, 2) = 1.0; d(2, 3) = 0.0; d(2, 4) = 0.0; d(2, 5) = 0.0;
        d(3, 0) = 0.0; d(3, 1) = 0.0; d(3, 2) = 0.0; d(3, 3) = 0.5 * (1.0 - 2.0 * m) / (1.0 - m); d(3, 4) = 0.0; d(3, 5) = 0.0;
        d(4, 0) = 0.0; d(4, 1) = 0.0; d(4, 2) = 0.0; d(4, 3) = 0.0; d(4, 4) = 0.5 * (1.0 - 2.0 * m) / (1.0 - m); d(4, 5) = 0.0;
        d(5, 0) = 0.0; d(5, 1) = 0.0; d(5, 2) = 0.0; d(5, 3) = 0.0; d(5, 4) = 0.0; d(5, 5) = 0.5 * (1.0 - 2.0 * m) / (1.0 - m);
        return d * ((e * (1.0 - m) / (1.0 + m) / (1.0 - 2.0 * m)));
    }
    void generate(bool isStatic = true)
    {
        double jacobian;
        matrix<double> b(6, freedom * shape->size),
                       c(freedom, freedom * shape->size),
                       d = elastic_matrix(),
                       jacobi(3, 3),
                       inverted_jacobi;
        vector<double> dx,
                       dy,
                       dz;

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
            jacobi.fill(0);
            for (unsigned j = 0; j < 3; j++)
                for (unsigned k = 0; k < shape->size; k++)
                {
                    jacobi(0, j) += dynamic_cast<T*>(shape)->shape_dxi(i)[k] * shape->x(k, j);
                    jacobi(1, j) += dynamic_cast<T*>(shape)->shape_deta(i)[k] * shape->x(k, j);
                    jacobi(2, j) += dynamic_cast<T*>(shape)->shape_dpsi(i)[k] * shape->x(k, j);
                }

            // Якобиан
            jacobian = det3x3(jacobi);

            // Обратная матрица Якоби
            inverted_jacobi = inv3x3(jacobi);

            // Производные функций формы
            dx = inverted_jacobi(0, 0) * dynamic_cast<T*>(shape)->shape_dxi(i) + inverted_jacobi(0, 1) * dynamic_cast<T*>(shape)->shape_deta(i) + inverted_jacobi(0, 2) * dynamic_cast<T*>(shape)->shape_dpsi(i);
            dy = inverted_jacobi(1, 0) * dynamic_cast<T*>(shape)->shape_dxi(i) + inverted_jacobi(1, 1) * dynamic_cast<T*>(shape)->shape_deta(i) + inverted_jacobi(1, 2) * dynamic_cast<T*>(shape)->shape_dpsi(i);
            dz = inverted_jacobi(2, 0) * dynamic_cast<T*>(shape)->shape_dxi(i) + inverted_jacobi(2, 1) * dynamic_cast<T*>(shape)->shape_deta(i) + inverted_jacobi(2, 2) * dynamic_cast<T*>(shape)->shape_dpsi(i);

            // Матрица градиентов
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(3, j * freedom + 1) = b(5, j * freedom + 2) = dx[j];
                b(1, j * freedom + 1) = b(3, j * freedom + 0) = b(4, j * freedom + 2) = dy[j];
                b(2, j * freedom + 2) = b(4, j * freedom + 1) = b(5, j * freedom + 0) = dz[j];
                if (!isStatic)
                    c(0, j * freedom + 0) = c(1, j * freedom + 1) = c(2, j * freedom + 2) = dynamic_cast<T*>(shape)->shape(i, j);
            }
            K += (transpose(b) * d * b) * shape->w[i] * abs(jacobian);
            // Вычисление температурной нагрузки
            if (dT != 0.0 && alpha != 0.0)
                load += transpose(b) * d * vector<double>{dT * alpha, dT * alpha, dT * alpha, 0, 0, 0} * shape->w[i] * abs(jacobian);
            if (!isStatic)
            {
                M += (transpose(c) * c) * density * shape->w[i] * density * abs(jacobian);
                D += (transpose(c) * c) * damping * shape->w[i] * density * abs(jacobian);
            }
        }
    }

public:
    TFE3D(void) : TFE()
    {
        freedom = 3;
        shape = new T();
    }
    virtual ~TFE3D()
    {
        delete shape;
    }
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> b(6, shape->size * freedom),
                       d = elastic_matrix(),
                       stress,
                       strain;

        for (unsigned i = 0; i < shape->size; i++)
        {
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(3, j * freedom + 1) = b(5, j * freedom + 2) = dynamic_cast<T*>(shape)->shape_dx(i, j);
                b(1, j * freedom + 1) = b(3, j * freedom + 0) = b(4, j * freedom + 2) = dynamic_cast<T*>(shape)->shape_dy(i, j);
                b(2, j * freedom + 2) = b(4, j * freedom + 1) = b(5, j * freedom + 0) = dynamic_cast<T*>(shape)->shape_dz(i, j);
            }
            strain = b * u;
            stress = d * strain;
            for (unsigned j = 0; j < 6; j++)
            {
                res(j, i) += strain(j, 0);
                res(j + 6, i) += stress(j, 0);
            }
        }
    }
};


#endif // FE3D_H
