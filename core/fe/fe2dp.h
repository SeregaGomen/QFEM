#ifndef FE2DP_H
#define FE2DP_H

#include <cmath>
#include "fe2d.h"

//----------------------------------------------------
//       Двумерный конечный элемент пластины
//----------------------------------------------------
template <class T> class TFE2DP : public TFE2D<T>
{
protected:
    matrix<double> extra_elastic_matrix(void)
    {
        return { { TFE::e / (2.0 + 2.0 * TFE::m),  0.0 },
            { 0.0,  TFE::e / (2.0 + 2.0 * TFE::m) } };
    }
public:
    TFE2DP(void) : TFE2D<T>()
    {
        TFE::freedom = 3;
    }
    virtual ~TFE2DP() = default;
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> bm(3, TFE::shape->size * TFE::freedom),
                       bp(2, TFE::shape->size * TFE::freedom),
                       stressm,
                       stressp,
                       strainm,
                       strainp;

        for (unsigned i = 0; i < TFE::shape->size; i++)
        {
            for (unsigned j = 0; j < TFE::shape->size; j++)
            {
                bm(0, TFE::freedom * j + 2) = bm(2, TFE::freedom * j + 1) = bp(0, TFE::freedom * j + 0) = dynamic_cast<T*>(TFE::shape)->shape_dx(i, j);
                bm(1, TFE::freedom * j + 1) = bm(2, TFE::freedom * j + 2) = bp(1, TFE::freedom * j + 0) = dynamic_cast<T*>(TFE::shape)->shape_dy(i, j);
                bp(0, TFE::freedom * j + 2) = bp(1, TFE::freedom * j + 1) = (i == j) ? 1.0 : 0.0;
            }
            strainm = bm * u;
            strainp = bp * u;
            stressm = TFE2D<T>::elastic_matrix() * strainm * TFE::thickness * 0.5;
            stressp = extra_elastic_matrix() * strainp;

            res(0, i) += strainm(0, 0);   // Exx
            res(1, i) += strainm(1, 0);   // Eyy
            res(3, i) += strainm(2, 0);   // Exy
            res(4, i) += strainp(0, 0);   // Exz
            res(5, i) += strainp(1, 0);   // Eyz
            res(6, i) += stressm(0, 0);   // Sxx
            res(7, i) += stressm(1, 0);   // Syy
            res(9, i) += stressm(2, 0);   // Sxy
            res(10, i) += stressp(0, 0);  // Sxz
            res(11, i) += stressp(1, 0);  // Syz
        }


//        double k = TFE::e / (1.0 - TFE::m * TFE::m),
//               g = TFE::e / (2.0 + 2.0 * TFE::m),
//               Tx,
//               Ty,
//               w,
//               dx,
//               dy,
//               n;

//        for (unsigned i = 0; i < TFE::shape->size; i++)
//            for (unsigned j = 0; j < TFE::shape->size; j++)
//            {
//                w  = u[TFE::freedom * j + 0];
//                Tx = u[TFE::freedom * j + 1];
//                Ty = u[TFE::freedom * j + 2];
//                dx = dynamic_cast<T*>(TFE::shape)->shape_dx(i, j);
//                dy = dynamic_cast<T*>(TFE::shape)->shape_dy(i, j);
//                n  = (i == j) ? 1.0 : 0.0;
//                // Exx
//                res(0, i) += Tx * dx;
//                // Eyy
//                res(1, i) += Ty * dy;
//                // Exy
//                res(3, i) += Tx * dy + Ty * dx;
//                // Exz
//                res(4, i) += Ty * n + w * dx;
//                // Eyz
//                res(5, i) += Tx * n + w * dy;
//                // Sxx
//                res(6, i) += k * ((Tx * dx) + TFE::m * (Ty * dy));
//                // Syy
//                res(7, i) += k * (TFE::m * (Tx * dx) + (Ty * dy));
//                // Sxy
//                res(9, i) += g * (Tx * dy + Ty * dx);
//                // Sxz
//                res(10, i) += g * (Ty * n + w * dx);
//                // Syz
//                res(11, i) += g * (Tx * n + w * dy);
//            }

    }
    void generate(bool isStatic = true)
    {
        double jacobian;
        matrix<double> bm(3, TFE::freedom * TFE::shape->size),
                       bp(2, TFE::freedom * TFE::shape->size),
                       c(TFE::freedom, TFE::freedom * TFE::shape->size),
                       jacobi,
                       inverted_jacobi;

        TFE::K.resize(TFE::freedom * TFE::shape->size, TFE::freedom * TFE::shape->size);
        TFE::load.resize(TFE::freedom * TFE::shape->size, 1);
        if (not isStatic)
        {
            TFE::M.resize(TFE::freedom * TFE::shape->size, TFE::freedom * TFE::shape->size);
            TFE::D.resize(TFE::freedom * TFE::shape->size, TFE::freedom * TFE::shape->size);
        }
        // Интегрирование по формуле Гаусса
        for (unsigned i = 0; i < TFE::shape->w.size(); i++)
        {
            // Матрица Якоби
            jacobi = TFE::shape->jacobi(i);

            // Якобиан
            jacobian = det(jacobi);

            // Обратная матрица Якоби
            inverted_jacobi = inv(jacobi);

            // Матрица градиентов
            for (unsigned j = 0; j < TFE::shape->size; j++)
            {
                bm(0, TFE::freedom * j + 2) = bm(2, TFE::freedom * j + 1) = bp(0, TFE::freedom * j + 0) = inverted_jacobi(0, 0) * TFE::shape->shape_dxi(i, j) + inverted_jacobi(0, 1) * TFE::shape->shape_deta(i, j);
                bm(1, TFE::freedom * j + 1) = bm(2, TFE::freedom * j + 2) = bp(1, TFE::freedom * j + 0) = inverted_jacobi(1, 0) * TFE::shape->shape_dxi(i, j) + inverted_jacobi(1, 1) * TFE::shape->shape_deta(i, j);
                bp(0, TFE::freedom * j + 2) = bp(1, TFE::freedom * j + 1) = TFE::shape->shape(i, j);
                if (not isStatic)
                    c(0, TFE::freedom * j + 0) = c(1, TFE::freedom * j + 1) = c(2, TFE::freedom * j + 2) = dynamic_cast<T*>(TFE::shape)->shape(i, j);
            }

            // Вычисление локальной матрицы жесткости
            TFE::K += ((transpose(bm) * TFE2D<T>::elastic_matrix() * bm) * (pow(TFE::thickness, 3) / 12.0) +
                       (transpose(bp) * extra_elastic_matrix() * bp) * (TFE::thickness * 5.0 / 6.0)) * TFE::shape->w[i] * abs(jacobian);
            // Вычисление температурной нагрузки
            if (TFE::temperature != 0.0 && TFE::alpha != 0.0)
                for (unsigned j = 0; j < TFE::getSize(); j++)
                    TFE::load[j * TFE::freedom][0] += TFE::e * TFE::alpha * TFE::temperature * TFE::shape->w[i] * abs(jacobian);
                // TFE::load += (transpose(bm) * TFE2D<T>::elastic_matrix() * vector<double>{ 1.0, 0.0, 0.0 }) * TFE::alpha * TFE::dT * TFE::shape->w[i] * abs(jacobian);
                // TFE::load += (transpose(bp) * extra_elastic_matrix() * vector<double>{ 1.0, 0.0 }) * TFE::alpha * TFE::dT * TFE::shape->w[i] * abs(jacobian);
            if (not isStatic)
            {
                TFE::M += (transpose(c) * c) * TFE::density * TFE::shape->w[i] * TFE::thickness * TFE::density * abs(jacobian);
                TFE::D += (transpose(c) * c) * TFE::damping * TFE::shape->w[i] * TFE::thickness * TFE::density * abs(jacobian);
            }
        }
    }
};

#endif // FE2DP_H
