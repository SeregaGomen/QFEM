#ifndef FE3DS_H
#define FE3DS_H

#include "fe2dp.h"

//----------------------------------------------------
//        Трехмерный конечный элемент оболочки
//----------------------------------------------------
template <class T> class TFE3DS : public TFE2DP<T>
{
private:
    // Матрица преобразования глобальных координат в локальные
    matrix<double> TransformMatrix;
    // Построение матрицц преобразования для текущего КЭ
    void createTransformMatrix(matrix<double>& px)
    {
        auto norm3 = [](vector<double> &res) {
            double norm{sqrt(res[0] * res[0] + res[1] * res[1] + res[2] * res[2])};

            for (auto k = 0; k < 3; k++)
                res[k] /= norm;
        };
        auto createVector3 = [norm3](double *xi, double *xj) -> vector<double> {
            vector<double> res(3);

            for (auto k = 0; k < 3; k++)
                res[k] = xj[k] - xi[k];
            norm3(res);
            return res;
        };
        auto crossProduct3 = [norm3](vector<double> &a, vector<double> &b) -> vector<double> {
            vector<double> res(3);

            res[0] = a[1] * b[2] - a[2] * b[1];
            res[1] = a[2] * b[0] - a[0] * b[2];
            res[2] = a[0] * b[1] - a[1] * b[0];
            norm3(res);
            return res;
        };
        vector<double> tmp = createVector3(px[2], px[0]),
                       vx = createVector3(px[1], px[0]),
                       vz = crossProduct3(vx, tmp),
                       vy = crossProduct3(vz, vx);

        TransformMatrix.resize(3, 3);
        for (unsigned i = 0; i < 3; i++)
        {
            TransformMatrix(0, i) = vx[i];
            TransformMatrix(1, i) = vy[i];
            TransformMatrix(2, i) = vz[i];
        }
    }
    // Матрица преобразования размерности (freedom * size) x (freedom * size)
    matrix<double> prepareTransformMatrix(void)
    {
        matrix<double> m(TFE::freedom * TFE::shape->size, TFE::freedom * TFE::shape->size);

        for (unsigned i = 0; i < 3; i++)
            for (unsigned j = 0; j < 3; j++)
                for (unsigned k = 0; k < TFE::freedom * TFE::shape->size; k +=3)
                    m(i + k, j + k) = TransformMatrix(i, j);
        return m;
    }
public:
    TFE3DS(void) : TFE2DP<T>()
    {
        TFE::freedom = 6;
    }
    virtual ~TFE3DS(void) = default;
    void setCoord(matrix<double>& px)
    {
        matrix<double> nx;

        createTransformMatrix(px);
        nx = transpose((TransformMatrix * transpose(px)));
        TFE::shape->create(nx);
    }
//     void calc(matrix<double>& res, vector<double>& u)
//     {
//         int index[][2]{{0, 0}, {1, 1}, {2, 2}, {0, 1}, {0, 2}, {1, 2}};
//         matrix<double> bm(3, TFE::shape->size * TFE::freedom),
//                        bp(3, TFE::shape->size * TFE::freedom),
//                        bc(2, TFE::shape->size * TFE::freedom),
//                        m = prepareTransformMatrix(), // Подготовка матрицы преобразования
//                        global_strain,
//                        global_stress,
//                        strainm,
//                        strainp,
//                        stressm,
//                        stressp,
//                        strainc,
//                        stressc,
//                        lu,
//                        local_strain(3, 3),
//                        local_stress(3, 3);

//         // Преобразование перемещений
//         lu = m * u;

//         for (unsigned i = 0; i < TFE::shape->size; i++)
//         {
//             for (unsigned j = 0; j < TFE::shape->size; j++)
//             {
//                 bm(0, TFE::freedom * j + 0) = bm(2, TFE::freedom * j + 1) = bp(2, TFE::freedom * j + 4) = bp(0, TFE::freedom * j + 3) = bc(0, TFE::freedom * j + 2) = dynamic_cast<T*>(TFE::shape)->shape_dx(i, j);
//                 bm(1, TFE::freedom * j + 1) = bm(2, TFE::freedom * j + 0) = bp(1, TFE::freedom * j + 4) = bp(2, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 2) = dynamic_cast<T*>(TFE::shape)->shape_dy(i, j);
//                 bc(0, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 4) = (i == j) ? 1.0 : 0.0;
//             }
//             strainm = bm * lu;
//             strainp = bp * lu;
//             strainc = bc * lu;
//             stressm = TFE2D<T>::elastic_matrix() * strainm;
//             stressp = TFE2D<T>::elastic_matrix() * strainp * TFE::thickness * 0.5;
//             stressc = TFE2DP<T>::extra_elastic_matrix() * strainc;

//             local_strain(0, 0) = strainm(0, 0) + strainp(0, 0);  local_strain(0, 1) = strainm(0, 2) + strainp(0, 2);  local_strain(0, 2) = strainc(0, 0);
//             local_strain(1, 0) = strainm(0, 2) + strainp(0, 2);  local_strain(1, 1) = strainm(0, 1) + strainp(0, 1);  local_strain(1, 2) = strainc(0, 1);
//             local_strain(2, 0) = strainc(0, 0); local_strain(2, 1) = strainc(0, 1); local_strain(2, 2) = 0.0;

//             local_stress(0, 0) = stressm(0, 0) + stressp(0, 0);  local_stress(0, 1) = stressm(0, 2) + stressp(0, 2);  local_stress(0, 2) = stressc(0, 0);
//             local_stress(1, 0) = stressm(0, 2) + stressp(0, 2);  local_stress(1, 1) = stressm(0, 1) + stressp(0, 1);  local_stress(1, 2) = stressc(0, 1);
//             local_stress(2, 0) = stressc(0, 0); local_stress(2, 1) = stressc(0, 1); local_stress(2, 2) = 0.0;

//             global_strain = transpose(TransformMatrix) * local_strain * TransformMatrix;
//             global_stress = transpose(TransformMatrix) * local_stress * TransformMatrix;

//             for (auto j = 0; j < 6; j++)
//             {
//                 res(j, i) += global_strain(index[j][0], index[j][1]);
//                 res(j+6, i) += global_stress(index[j][0], index[j][1]);
//             }

// //            res(0, i) += global_strain(0, 0);    // Exx
// //            res(1, i) += global_strain(1, 1);    // Eyy
// //            res(2, i) += global_strain(2, 2);    // Ezz
// //            res(3, i) += global_strain(0, 1);    // Exy
// //            res(4, i) += global_strain(0, 2);    // Exz
// //            res(5, i) += global_strain(1, 2);    // Eyz
// //            res(6, i) += global_stress(0, 0);    // Sxx
// //            res(7, i) += global_stress(1, 1);    // Syy
// //            res(8, i) += global_stress(2, 2);    // Szz
// //            res(9, i) += global_stress(0, 1);    // Sxy
// //            res(10, i) += global_stress(0, 2);   // Sxz
// //            res(11, i) += global_stress(1, 2);   // Syz
//         }
//     }
    void calc(matrix<double>& res, vector<double>& u)
    {
        matrix<double> bm(3, TFE::shape->size * TFE::freedom),
            bp(3, TFE::shape->size * TFE::freedom),
            bc(2, TFE::shape->size * TFE::freedom),
            m = prepareTransformMatrix(), // Подготовка матрицы преобразования
            global_strain,
            global_stress,
            strainm,
            strainp,
            stressm,
            stressp,
            strainc,
            stressc,
            lu,
            local_strain(3, 3),
            local_stress(3, 3);

        // Преобразование перемещений
        lu = m * u;

        for (unsigned i = 0; i < TFE::shape->size; i++)
        {
            for (unsigned j = 0; j < TFE::shape->size; j++)
            {
                bm(0, TFE::freedom * j + 0) = bm(2, TFE::freedom * j + 1) = bp(2, TFE::freedom * j + 4) = bp(0, TFE::freedom * j + 3) = bc(0, TFE::freedom * j + 2) = dynamic_cast<T*>(TFE::shape)->shape_dx(i, j);
                bm(1, TFE::freedom * j + 1) = bm(2, TFE::freedom * j + 0) = bp(1, TFE::freedom * j + 4) = bp(2, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 2) = dynamic_cast<T*>(TFE::shape)->shape_dy(i, j);
                bc(0, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 4) = (i == j) ? 1.0 : 0.0;
            }
            strainm = bm * lu;
            strainp = bp * lu;
            strainc = bc * lu;
            stressm = TFE2D<T>::elastic_matrix() * strainm;
            stressp = TFE2D<T>::elastic_matrix() * strainp * TFE::thickness * 0.5;
            stressc = TFE2DP<T>::extra_elastic_matrix() * strainc;

            local_strain(0, 0) = strainm(0, 0) + strainp(0, 0);  local_strain(0, 1) = strainm(0, 2) + strainp(0, 2);  local_strain(0, 2) = strainc(0, 0);
            local_strain(1, 0) = strainm(0, 2) + strainp(0, 2);  local_strain(1, 1) = strainm(0, 1) + strainp(0, 1);  local_strain(1, 2) = strainc(0, 1);
            local_strain(2, 0) = strainc(0, 0); local_strain(2, 1) = strainc(0, 1); local_strain(2, 2) = 0.0;

            local_stress(0, 0) = stressm(0, 0) + stressp(0, 0);  local_stress(0, 1) = stressm(0, 2) + stressp(0, 2);  local_stress(0, 2) = stressc(0, 0);
            local_stress(1, 0) = stressm(0, 2) + stressp(0, 2);  local_stress(1, 1) = stressm(0, 1) + stressp(0, 1);  local_stress(1, 2) = stressc(0, 1);
            local_stress(2, 0) = stressc(0, 0); local_stress(2, 1) = stressc(0, 1); local_stress(2, 2) = 0.0;

            global_strain = transpose(TransformMatrix) * local_strain * TransformMatrix;
            global_stress = transpose(TransformMatrix) * local_stress * TransformMatrix;

            res(0, i) += global_strain(0, 0);    // Exx
            res(1, i) += global_strain(1, 1);    // Eyy
            res(2, i) += global_strain(2, 2);    // Ezz
            res(3, i) += global_strain(0, 1);    // Exy
            res(4, i) += global_strain(0, 2);    // Exz
            res(5, i) += global_strain(1, 2);    // Eyz
            res(6, i) += global_stress(0, 0);    // Sxx
            res(7, i) += global_stress(1, 1);    // Syy
            res(8, i) += global_stress(2, 2);    // Szz
            res(9, i) += global_stress(0, 1);    // Sxy
            res(10, i) += global_stress(0, 2);   // Sxz
            res(11, i) += global_stress(1, 2);   // Syz
        }
    }
    void generate(bool isStatic = true)
    {
        double jacobian,
               singular = 0;
        matrix<double> bm(3, TFE::freedom * TFE::shape->size),
                       bp(3, TFE::freedom * TFE::shape->size),
                       bc(2, TFE::freedom * TFE::shape->size),
                       c(TFE::freedom, TFE::freedom * TFE::shape->size),
                       jacobi,
                       inverted_jacobi,
                       m = prepareTransformMatrix(); // Подготовка матрицы преобразования

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

            // Матрицы градиентов
            for (unsigned j = 0; j < TFE::shape->size; j++)
            {
                bm(0, TFE::freedom * j + 0) = bm(2, TFE::freedom * j + 1) = bp(0, TFE::freedom * j + 3) = bp(2, TFE::freedom * j + 4) = bc(0, TFE::freedom * j + 2) = inverted_jacobi(0, 0) * TFE::shape->shape_dxi(i, j) + inverted_jacobi(0, 1) * TFE::shape->shape_deta(i, j);
                bm(1, TFE::freedom * j + 1) = bm(2, TFE::freedom * j + 0) = bp(1, TFE::freedom * j + 4) = bp(2, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 2) = inverted_jacobi(1, 0) * TFE::shape->shape_dxi(i, j) + inverted_jacobi(1, 1) * TFE::shape->shape_deta(i, j);
                bc(0, TFE::freedom * j + 3) = bc(1, TFE::freedom * j + 4) = TFE::shape->shape(i, j);
                if (not isStatic)
                    c(0, TFE::freedom * j + 0) = c(1, TFE::freedom * j + 1) = c(2, TFE::freedom * j + 2) = c(3, TFE::freedom * j + 3) = c(4, TFE::freedom * j + 4) = c(5, TFE::freedom * j + 5) = dynamic_cast<T*>(TFE::shape)->shape(i, j);
            }

            // Вычисление локальной матрицы жесткости
            TFE::K += ((transpose(bm) * TFE2D<T>::elastic_matrix() * bm) * TFE::thickness +
                       (transpose(bp) * TFE2D<T>::elastic_matrix() * bp) * (pow(TFE::thickness, 3) / 12.0) +
                       (transpose(bc) * TFE2DP<T>::extra_elastic_matrix() * bc) * (TFE::thickness * 5.0 / 6.0)) * TFE::shape->w[i] * abs(jacobian);
            // Вычисление температурной нагрузки
            if (TFE::temperature != 0.0 && TFE::alpha != 0.0)
                TFE::load += (transpose(bm) * TFE2D<T>::elastic_matrix() * vector<double>{ 1.0, 1.0, 0.0 }) * TFE::alpha * TFE::temperature * TFE::shape->w[i] * abs(jacobian);
            if (not isStatic)
            {
                TFE::M += (transpose(c) * c) * TFE::density * TFE::shape->w[i] * TFE::thickness * TFE::density * abs(jacobian);
                TFE::D += (transpose(c) * c) * TFE::damping * TFE::shape->w[i] * TFE::thickness * TFE::density * abs(jacobian);
            }
        }
        // Поиск максимального диагонального элемента
        for (unsigned i = 0; i < TFE::K.size1(); i++)
            if (TFE::K(i, i) > singular)
                singular = TFE::K(i, i);
        singular *= 1.0E-3;

        // Устранение сингулярности
        for (unsigned i = 0; i < TFE::shape->size; i++)
        {
            TFE::K(TFE::freedom * (i + 1) - 1, TFE::freedom * (i + 1) - 1) = singular;
            if (not isStatic)
                TFE::M(TFE::freedom * (i + 1) - 1, TFE::freedom * (i + 1) - 1) = TFE::D(TFE::freedom * (i + 1) - 1, TFE::freedom * (i + 1) - 1) = singular;
        }

        // Преобразование из локальных координат в глобальные
        TFE::K = transpose(m) * TFE::K * m;
        TFE::load = transpose(m) * TFE::load;
        if (not isStatic)
        {
            TFE::M = transpose(m) * TFE::M * m;
            TFE::D = transpose(m) * TFE::D * m;
        }
    }
};


#endif // FE3DS_H
