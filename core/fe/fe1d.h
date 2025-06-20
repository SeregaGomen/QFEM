#ifndef FE1D_H
#define FE1D_H

#include "fe.h"

//----------------------------------------------------
//             Одномерный конечный элемент
//----------------------------------------------------
template <class T> class TFE1D : public TFE
{
public:
    TFE1D(void) : TFE()
    {
        freedom = 1;
        shape = new T();
    }
    virtual ~TFE1D() = default;
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> stress,
                       strain,
                       b(1, shape->size);

        for (unsigned i = 0; i < shape->size; i++)
        {
            for (unsigned j = 0; j < shape->size; j++)
                b(0, freedom * j) = shape->shape_dx(i, j);
            strain = b * u;
            stress = e * strain;
            res(0, i) += strain(0, 0);
            res(1, i) += stress(0, 0);
        }
    }
    void generate(bool isStatic = true)
    {
        double jacobian,
               inverted_jacobi;
        matrix<double> b(1, shape->size),
                       c(freedom, shape->size);

        K.resize(freedom * shape->size, freedom * shape->size);
        load.resize(freedom * shape->size, 1);
        if (not isStatic)
        {
            M.resize(freedom * shape->size, freedom * shape->size);
            D.resize(freedom * shape->size, freedom * shape->size);
        }

        // Якобиан и обратная матрица Якоби
        jacobian = (shape->x(1, 0) - shape->x(0, 0)) * 0.5;
        inverted_jacobi = 1.0 / jacobian;

        // Численное интегрирование по формуле Гаусса на отрезке [-0,5; 0,5]
        for (unsigned i = 0; i < shape->w.size(); i++)
        {
            // Матрица градиентов
            for (unsigned j = 0; j < shape->size; j++)
            {
                b(0, freedom * j) = inverted_jacobi * shape->shape_dxi(i, j);
                if (not isStatic)
                    c(0, freedom * j) = dynamic_cast<T*>(shape)->shape(i, j);
            }
            // Вычисление локальной матрицы жесткости
            K += (((transpose(b) * e) * b) * shape->w[i] * thickness * abs(jacobian));
            // Вычисление температурной нагрузки
            if (temperature != 0.0 && alpha != 0.0)
                load += (transpose(b) * e * temperature * alpha * shape->w[i] * abs(jacobian));
            if (not isStatic)
            {
                M += ((transpose(c) * c) * (density * shape->w[i] * thickness * abs(jacobian)));
                D += ((transpose(c) * c) * (damping * shape->w[i] * thickness * abs(jacobian)));
            }
        }
    }
};

#endif // FE1D_H
