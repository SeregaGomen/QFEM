#ifndef SHAPE_H
#define SHAPE_H

//#include <Eigen/Dense>
#include <array>
#include "msg/msg.h"
#include "util/matrix.h"

//-----------------------------------------------------------------------
// Абстрактный изопараметрический класс функции формы конечного элемента
// вида N(x,y,z) = c0 + c1 * x + c2 * y + ...
//-----------------------------------------------------------------------
class TShape
{
protected:
    // Коэффициенты функций форм
    matrix<double> c;
    // Структура изопараметрической функции формы
    virtual double shape_coeff(int, int) = 0;
public:
    // Количество узлов КЭ
    unsigned size = 0;
    // Координаты вершин
    matrix<double> x;
    // Параметры квадратур для численного интегрирования
    vector<double> xi;
    vector<double> psi;
    vector<double> eta;
    vector<double> w;
    TShape(void) noexcept {}
    virtual ~TShape() noexcept {}
    // Вычисление коэффициентов функций формы
//    void create(matrix<double> &px)
//    {
//        using namespace Eigen;

//        MatrixXd A(size, size);
//        VectorXd b(size);

//        x = px;
//        c.resize(size, size);
//        for (unsigned i = 0; i < size; i++)
//        {
//            for (unsigned j = 0; j < size; j++)
//            {
//                for (unsigned k = 0; k < size; k++)
//                    A(j, k) = shape_coeff(j, k);
//                b(j) = (i == j) ? 1.0 : 0.0;
//            }
//            if (A.determinant() == 0.0)
//                throw ErrorCode::EIncorrectFE;
////            b = A.colPivHouseholderQr().solve(b);
//            b = A.ldlt().solve(b);
//            for (unsigned j = 0; j < size; j++)
//                c(j, i) = b(j);
//        }
//    }
    void create(matrix<double> &px)
    {
        matrix<double> a(size, size);
        vector<double> b(size);

        x = px;
        c.resize(size, size);
        for (unsigned i = 0; i < size; i++)
        {
            for (unsigned j = 0; j < size; j++)
            {
                for (unsigned k = 0; k < size; k++)
                    a(j, k) = shape_coeff(j, k);
                b[j] = (i == j) ? 1.0 : 0.0;
            }
            if (!gauss_solve(a, b))
                throw ErrorCode::EIncorrectFE;
            for (unsigned j = 0; j < size; j++)
                c(j, i) = b[j];
        }
    }
    // Якобиан
    virtual matrix<double> jacobi(int = 0) = 0;
    // Значения функции формы
    virtual double shape(int, int) = 0;
    // ... производных локальных функций формы
    virtual double shape_dxi(int, int) = 0;
    virtual double shape_dpsi(int, int) = 0;
    virtual double shape_deta(int, int) = 0;
    // ... производных глобальных функций формы
    virtual double shape_dx(int, int) = 0;
    virtual double shape_dy(int, int) = 0;
    virtual double shape_dz(int, int) = 0;
};
//-----------------------------------------------------------------------
//          Линейные функции формы одномерного двухузлового КЭ
//-----------------------------------------------------------------------
class TShape1D2 : public TShape
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 2>{ 1.0, x(i, 0) }[j];
    }
public:
    TShape1D2(void) : TShape()
    {
        size = 2;
        xi = { -0.57735026919, 0.0, 0.57735026919 };
        w = { 0.55555555556, 0.88888888889, 0.55555555556 };
    }
    ~TShape1D2(void) {}
    double shape(int i, int j)
    {
        return array<double, 2>{ (1.0 - xi[i]) * 0.5, (1.0 + xi[i]) * 0.5 }[j];
    }
    double shape_dxi(int, int j)
    {
        return array<double, 2>{ -0.5, 0.5 }[j];
    }
    double shape_deta(int, int)
    {
        return 0;
    }
    double shape_dpsi(int, int)
    {
        return 0;
    }
    double shape_dx(int, int j)
    {
        return c(1, j);
    }
    double shape_dy(int, int)
    {
        return 0;
    }
    double shape_dz(int, int)
    {
        return 0;
    }
    double edge_volume(const matrix<double>&)
    {
        return 1.0;
    }
    vector<double> volumeLoadShare(void)
    {
        return { 0.5, 0.5 };
    }
    vector<double> surfaceLoadShare(void)
    {
        return { 1.0 };
    }
    matrix<double> jacobi(int)
    {
        return { { (x(1, 0) - x(0, 0)) * 0.5 } };
    }
};
//-----------------------------------------------------------------------
//                  Линейные функции формы треугольного КЭ
//-----------------------------------------------------------------------
class TShape2D3 : public TShape
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 3>{ 1.0, x(i, 0), x(i, 1) }[j];
    }
public:
    TShape2D3(void) : TShape()
    {
        size = 3;
        xi = { 0.0, 0.5, 0.5 };
        eta = { 0.5, 0.0, 0.5 };
        w = { 0.166666666667, 0.166666666667, 0.166666666667 };
    }
    virtual ~TShape2D3(void) {}
    double shape(int i, int j)
    {
        return array<double, 3>{ 1.0 - xi[i] - eta[i], xi[i], eta[i] }[j];
    }
    double shape_dxi(int, int j)
    {
        return array<double, 3>{ -1.0, 1.0, 0.0 }[j];
    }
    double shape_deta(int, int j)
    {
        return array<double, 3>{ -1.0, 0.0, 1.0 }[j];
    }
    double shape_dpsi(int, int)
    {
        return 0;
    }
    double shape_dx(int, int j)
    {
        return c(1, j);
    }
    double shape_dy(int, int j)
    {
        return c(2, j);
    }
    double shape_dz(int, int)
    {
        return 0;
    }
    matrix<double> jacobi(int i)
    {
        matrix<double> jacobi(2, 2);

        for (unsigned j = 0; j < 2; j++)
            for (unsigned k = 0; k < size; k++)
            {
                jacobi(0, j) += shape_dxi(i, k) * x(k, j);
                jacobi(1, j) += shape_deta(i, k) * x(k, j);
            }
        return jacobi;
    }
};
//-----------------------------------------------------------------------
//              Билинейные функции формы четырехугольного КЭ
//-----------------------------------------------------------------------
class TShape2D4 : public TShape2D3
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 4>{ 1.0, x(i, 0), x(i, 1), x(i, 0) * x(i, 1) }[j];
    }
public:
    TShape2D4(void) : TShape2D3()
    {
        size = 4;
        xi = { -0.57735027, -0.57735027, 0.57735027, 0.57735027 };
        eta = { -0.57735027, 0.57735027, -0.57735027, 0.57735027 };
        w = { 1.0, 1.0, 1.0, 1.0 };
    }
    ~TShape2D4(void) {}
    double shape(int i, int j)
    {
        return array<double, 4>{ 0.25 * (1.0 - xi[i]) * (1.0 - eta[i]), 0.25 * (1.0 + xi[i]) * (1.0 - eta[i]), 0.25 * (1.0 + xi[i]) * (1.0 + eta[i]), 0.25 * (1.0 - xi[i]) * (1.0 + eta[i]) }[j];
    }
    double shape_dxi(int i, int j)
    {
        return array<double, 4>{ -0.25 * (1.0 - eta[i]), 0.25 * (1.0 - eta[i]), 0.25 * (1.0 + eta[i]), -0.25 * (1.0 + eta[i]) }[j];
    }
    double shape_deta(int i, int j)
    {
        return array<double, 4>{ -0.25 * (1.0 - xi[i]), -0.25 * (1.0 + xi[i]), 0.25 * (1.0 + xi[i]), 0.25 * (1.0 - xi[i]) }[j];
    }
    double shape_dpsi(int, int)
    {
        return 0;
    }
    double shape_dx(int i, int j)
    {
        return c(1, j) + c(3, j) * x(i, 1);
    }
    double shape_dy(int i, int j)
    {
        return c(2, j) + c(3, j) * x(i, 0);
    }
    double shape_dz(int, int)
    {
        return 0;
    }
};
//-----------------------------------------------------------------------
//               Квадратичные функции формы треугольного КЭ
//-----------------------------------------------------------------------
class TShape2D6 : public TShape2D3
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 6>{ 1.0, x(i, 0), x(i, 1), x(i, 0) * x(i, 1), x(i, 0) * x(i, 0), x(i, 1) * x(i, 1) }[j];
    }
public:
    TShape2D6(void) : TShape2D3()
    {
        size = 6;
        xi = { 0.333333333333, 0.0, 0.5, 0.5, 1.0, 0.0, 0.0 };
        eta = { 0.333333333333, 0.5, 0.0, 0.5, 0.0, 1.0, 0 };
        w = { 0.225, 0.0666666666667, 0.0666666666667, 0.0666666666667, 0.025, 0.025, 0.025 };
    }
    ~TShape2D6(void) {}
    double shape(int i, int j)
    {
        return array<double, 6>{ (1.0 - xi[i] - eta[i]) * (2.0 * (1.0 - xi[i] - eta[i]) - 1.0),
                    xi[i] * (2.0 * xi[i] - 1.0), eta[i] * (2.0 * eta[i] - 1.0),
                    4.0 * (1.0 - xi[i] - eta[i]) * xi[i],
                    4.0 * xi[i] * eta[i],
                    4.0 * (1.0 - xi[i] - eta[i]) * eta[i] }[j];
    }
    double shape_dxi(int i, int j)
    {
        return array<double, 6>{ -3.0 + 4.0 * xi[i] + 4.0 * eta[i], 4.0 * xi[i] - 1.0, 0.0, -8.0 * xi[i] + 4.0 - 4.0 * eta[i], 4.0 * eta[i], -4.0 * eta[i] }[j];
    }
    double shape_deta(int i, int j)
    {
        return array<double, 6>{ -3.0 + 4.0 * xi[i] + 4.0 * eta[i], 0.0, 4.0 * eta[i] - 1.0, -4.0 * xi[i], 4.0 * xi[i], -8.0 * eta[i] + 4.0 - 4.0 * xi[i] }[j];
    }
    double shape_dpsi(int, int)
    {
        return 0;
    }
    double shape_dx(int i, int j)
    {
        return c(1, j) + c(3, j) * x(i, 1) + 2.0 * c(4, j) * x(i, 0);
    }
    double shape_dy(int i, int j)
    {
        return c(2, j) + c(3, j) * x(i, 0) + 2.0 * c(5, j) * x(i, 1);
    }
    double shape_dz(int, int)
    {
        return 0;
    }
};
//-----------------------------------------------------------------------
//              Линейные функции формы тетраэдрального КЭ
//-----------------------------------------------------------------------
class TShape3D4 : public TShape
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 4>{ 1.0, x(i, 0), x(i, 1), x(i, 2) }[j];
    }
public:
    TShape3D4(void) : TShape()
    {
        size = 4;
        xi = { 0.25, 0.5, 0.16666666667, 0.16666666667, 0.16666666667 };
        eta = { 0.25, 0.16666666667, 0.5, 0.16666666667, 0.16666666667 };
        psi = { 0.25, 0.16666666667, 0.16666666667, 0.5, 0.16666666667 };
        w = { -0.13333333333, 0.075, 0.075, 0.075, 0.075 };
    }
    virtual ~TShape3D4() {}
    double shape(int i, int j)
    {
        return array<double, 4>{ 1.0 - xi[i] - eta[i] - psi[i], xi[i], eta[i], psi[i] }[j];
    }
    double shape_dxi(int, int j)
    {
        return array<double, 4>{ -1.0, 1.0, 0.0, 0.0 }[j];
    }
    double shape_deta(int, int j)
    {
        return array<double, 4>{ -1.0, 0.0, 1.0, 0.0 }[j];
    }
    double shape_dpsi(int, int j)
    {
        return array<double, 4>{ -1.0, 0.0, 0.0, 1.0 }[j];
    }
    double shape_dx(int, int j)
    {
        return c(1, j);
    }
    double shape_dy(int, int j)
    {
        return c(2, j);
    }
    double shape_dz(int, int j)
    {
        return c(3, j);
    }
    matrix<double> jacobi(int i)
    {
        matrix<double> jacobi(3, 3);

        for (unsigned j = 0; j < 3; j++)
            for (unsigned k = 0; k < size; k++)
            {
                jacobi(0, j) += shape_dxi(i, k) * x(k, j);
                jacobi(1, j) += shape_deta(i, k) * x(k, j);
                jacobi(2, j) += shape_dpsi(i, k) * x(k, j);
            }
        return jacobi;
    }
};
//-----------------------------------------------------------------------
//    Билинейные функции формы шестигранной четырехугольной призмы
//-----------------------------------------------------------------------
class TShape3D8 : public TShape3D4
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 8>{ 1.0, x(i, 0), x(i, 1), x(i, 2), x(i, 0) * x(i, 1), x(i, 0) * x(i, 2), x(i, 1) * x(i, 2), x(i, 0) * x(i, 1) * x(i, 2) }[j];
    }
public:
    TShape3D8(void) : TShape3D4()
    {
        size = 8;
        xi = { -0.57735026919, -0.57735026919, -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919, 0.57735026919, 0.57735026919 };
        eta = { -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919, -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919 };
        psi = { -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919 };
        w = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
    }
    ~TShape3D8(void) {}
    double shape(int i, int j)
    {
        array<double, 8> v{
                          0.125 * (1.0 - xi[i]) * (1.0 - eta[i]) * (1.0 - psi[i]),
                          0.125 * (1.0 + xi[i]) * (1.0 - eta[i]) * (1.0 - psi[i]),
                          0.125 * (1.0 + xi[i]) * (1.0 + eta[i]) * (1.0 - psi[i]),
                          0.125 * (1.0 - xi[i]) * (1.0 + eta[i]) * (1.0 - psi[i]),
                          0.125 * (1.0 - xi[i]) * (1.0 - eta[i]) * (1.0 + psi[i]),
                          0.125 * (1.0 + xi[i]) * (1.0 - eta[i]) * (1.0 + psi[i]),
                          0.125 * (1.0 + xi[i]) * (1.0 + eta[i]) * (1.0 + psi[i]),
                          0.125 * (1.0 - xi[i]) * (1.0 + eta[i]) * (1.0 + psi[i])
                        };

        return v[j];
    }
    double shape_dxi(int i, int j)
    {
        return array<double, 8>{ -0.125 * (1.0 - eta[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 - eta[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 + eta[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 + eta[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 - eta[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 - eta[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 + eta[i]) * (1.0 + psi[i]),
                 -0.125 * (1.0 + eta[i]) * (1.0 + psi[i]) }[j];
    }
    double shape_deta(int i, int j)
    {
        return array<double, 8>{ -0.125 * (1.0 - xi[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 - xi[i]) * (1.0 + psi[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 + psi[i]) }[j];
    }
    double shape_dpsi(int i, int j)
    {
        return array<double, 8>{ -0.125 * (1.0 - xi[i]) * (1.0 - eta[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 - eta[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 + eta[i]),
                 -0.125 * (1.0 - xi[i]) * (1.0 + eta[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 - eta[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 - eta[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 + eta[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 + eta[i]) }[j];
    }
    double shape_dx(int i, int j)
    {
        return c(1, j) + c(4, j) * x(i, 1) + c(5, j) * x(i, 2) + c(7, j) * x(i, 1) * x(i, 2);
    }
    double shape_dy(int i, int j)
    {
        return c(2, j) + c(4, j) * x(i, 0) + c(6, j) * x(i, 2) + c(7, j) * x(i, 0) * x(i, 2);
    }
    double shape_dz(int i, int j)
    {
        return c(3, j) + c(5, j) * x(i, 0) + c(6, j) * x(i, 1) + c(7, j) * x(i, 0)* x(i, 1);
    }
};
//-----------------------------------------------------------------------
//           Квадратичные функции формы тетраэдрального КЭ
//-----------------------------------------------------------------------
class TShape3D10 : public TShape3D4
{
protected:
    double shape_coeff(int i, int j)
    {
        return array<double, 10>{ 1.0, x(i, 0), x(i, 1), x(i, 2), x(i, 0) * x(i, 1), x(i, 0) * x(i, 2), x(i, 1) * x(i, 2), x(i, 0) * x(i, 0), x(i, 1) * x(i, 1), x(i, 2) * x(i, 2) }[j];
    }
public:
    TShape3D10(void) : TShape3D4()
    {
        size = 10;
        xi = { 0.25, 0.5, 0.16666666667, 0.16666666667, 0.16666666667 };
        eta = { 0.25, 0.16666666667, 0.5, 0.16666666667, 0.16666666667 };
        psi = { 0.25, 0.16666666667, 0.16666666667, 0.5, 0.16666666667 };
        w = { -0.13333333333, 0.075, 0.075, 0.075, 0.075 };
    }
    ~TShape3D10(void) {}
    double shape(int i, int j)
    {
        return array<double, 10>{ (1.0 - xi[i] - eta[i] - psi[i]) * (2.0 * (1.0 - xi[i] - eta[i] - psi[i]) - 1.0), xi[i] * (2.0 * xi[i] - 1.0), eta[i] * (2.0 * eta[i] - 1.0),
                    psi[i] * (2.0 * psi[i] - 1.0), 4.0 * (1.0 - xi[i] - eta[i] - psi[i]) * xi[i], 4.0 * xi[i] * eta[i], 4.0 * (1.0 - xi[i] - eta[i] - psi[i]) * eta[i],
                    4.0 * eta[i] * psi[i], 4.0 * xi[i] * psi[i], 4.0 * (1.0 - xi[i] - eta[i] - psi[i]) * psi[i] }[j];
    }
    double shape_dxi(int i, int j)
    {
        return array<double, 10>{ -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 4.0 * xi[i] - 1.0,
                  0.0, 0.0, -8.0 * xi[i] + 4.0 - 4.0 * eta[i] - 4.0 * psi[i],
                  4.0 * eta[i], -4.0 * eta[i], 0.0, 4.0 * psi[i], -4.0 * psi[i] }[j];
    }
    double shape_deta(int i, int j)
    {
        return array<double, 10>{ -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 0.0, 4.0 * eta[i] - 1.0, 0.0,
                 -4.0 * xi[i], 4.0 * xi[i], -8.0 * eta[i] + 4.0 - 4.0 * xi[i] - 4.0 * psi[i],
                  4.0 * psi[i], 0.0, -4.0 * psi[i] }[j];
    }
    double shape_dpsi(int i, int j)
    {
        return array<double, 10>{ -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 0.0, 0.0, 4.0 * psi[i] - 1.0,
                 -4.0 * xi[i], 0.0, -4.0 * eta[i], 4.0 * eta[i], 4.0 * xi[i],
                 -8.0 * psi[i] + 4.0 - 4.0 * xi[i] - 4.0 * eta[i] }[j];
    }
    double shape_dx(int i, int j)
    {
        return c(1, j) + 2.0 * c(7, j) * x(i, 0) + c(4, j) * x(i, 1) + c(5, j) * x(i, 2);
    }
    double shape_dy(int i, int j)
    {
        return c(2, j) + c(4, j) * x(i, 0) + 2.0 * c(8, j) * x(i, 1) + c(6, j) * x(i, 2);
    }
    double shape_dz(int i, int j)
    {
        return c(3, j) + c(5, j) * x(i, 0) + c(6, j) * x(i, 1) + 2.0 * c(9, j) * x(i, 2);
    }
};

#endif // SHAPE_H
