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
        matrix<double> d(3, 3);

        d(0, 0) = 1.0;  d(0, 1) = m;    d(0, 2) = 0;
        d(1, 0) = m;    d(1, 1) = 1.0;  d(1, 2) = 0;
        d(2, 0) = 0;    d(2, 1) = 0;    d(2, 2) = 0.5 * (1.0 - m);
        return d * (e / (1.0 - m * m));
    }
    void generate(bool isStatic = true)
    {
        double jacobian;
        matrix<double> b(3, freedom * shape->size),
                       c(freedom, freedom * shape->size),
                       d = elastic_matrix(),
                       jacobi(2, 2),
                       inverted_jacobi;
        vector<double> dx,
                       dy;

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
            jacobi.fill(0);
            for (unsigned j = 0; j < 2; j++)
                for (unsigned k = 0; k < shape->size; k++)
                {
                    jacobi(0, j) += dynamic_cast<T*>(shape)->shape_dxi(i)[k] * shape->x(k, j);
                    jacobi(1, j) += dynamic_cast<T*>(shape)->shape_deta(i)[k] * shape->x(k, j);
                }

            // Якобиан
            jacobian = det2x2(jacobi);

            // Обратная матрица Якоби
            inverted_jacobi = inv2x2(jacobi);

            // Производные функций формы
            dx = inverted_jacobi(0, 0) * dynamic_cast<T*>(shape)->shape_dxi(i) + inverted_jacobi(0, 1) * dynamic_cast<T*>(shape)->shape_deta(i);
            dy = inverted_jacobi(1, 0) * dynamic_cast<T*>(shape)->shape_dxi(i) + inverted_jacobi(1, 1) * dynamic_cast<T*>(shape)->shape_deta(i);

            // Матрица градиентов
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(2, j * freedom + 1) = dx[j];
                b(1, j * freedom + 1) = b(2, j * freedom + 0) = dy[j];
                if (!isStatic)
                    c(0, j * freedom + 0) = c(0, j * freedom + 1) = dynamic_cast<T*>(shape)->shape(i, j);
            }

            // Вычисление локальной матрицы жесткости
            K += (transpose(b) * d * b) * shape->w[i] * thickness * abs(jacobian);
            // Вычисление температурной нагрузки
            if (dT != 0.0 && alpha != 0.0)
                load += transpose(b) * d * vector<double>{dT * alpha, dT * alpha, 0} * shape->w[i] * abs(jacobian);
            if (!isStatic)
            {
                M += (transpose(c) * c) * density * shape->w[i] * thickness * abs(jacobian);
                D += (transpose(c) * c) * damping * shape->w[i] * thickness * abs(jacobian);
            }
        }
    }
public:
    TFE2D(void) : TFE()
    {
        freedom = 2;
        shape = new T();
    }
    virtual ~TFE2D()
    {
        delete shape;
    }
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> b(3, shape->size * freedom),
                       d = elastic_matrix(),
                       stress,
                       strain;

        for (unsigned i = 0; i < shape->size; i++)
        {
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, j * freedom + 0) = b(2, j * freedom + 1) = dynamic_cast<T*>(shape)->shape_dx(i, j);
                b(1, j * freedom + 1) = b(2, j * freedom + 0) = dynamic_cast<T*>(shape)->shape_dy(i, j);
            }
            strain = b * u;
            stress = d * strain;
            for (unsigned j = 0; j < 3; j++)
            {
                res(j, i) += strain(j, 0);
                res(j + 3, i) += stress(j, 0);
            }
        }
    }
};


#endif // FE2D_H
