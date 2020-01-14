#ifndef SHAPE_H
#define SHAPE_H

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
    virtual double shape_coeff(unsigned, unsigned) = 0;
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
    TShape(void) {}
    virtual ~TShape() {}
    // Вычисление коэффициентов функций формы
    void create(matrix<double>&);
};
//-----------------------------------------------------------------------
//          Линейные функции формы одномерного двухузлового КЭ
//-----------------------------------------------------------------------
class TShape1D2 : public TShape
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0) };

        return s[j];
    }
public:
    TShape1D2(void) : TShape()
    {
        size = 2;
        xi = { -0.57735026919, 0.0, 0.57735026919 };
        w = { 0.55555555556, 0.88888888889, 0.55555555556 };
    }
    ~TShape1D2(void) {}
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{ (1.0 - xi[i]) * 0.5, (1.0 + xi[i]) * 0.5 };

        return v[j];
    }
    vector<double> shape_dxi(unsigned)
    {
        return { -0.5, 0.5 };
    }
    double shape_dx(unsigned, unsigned j)
    {
        return c(1, j);
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
};
//-----------------------------------------------------------------------
//                  Линейные функции формы треугольного КЭ
//-----------------------------------------------------------------------
class TShape2D3 : public TShape
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1) };

        return s[j];
    }
public:
    TShape2D3(void) : TShape()
    {
        size = 3;
        xi = { 0.0, 0.5, 0.5 };
        eta = { 0.5, 0.0, 0.5 };
        w = { 0.166666666667, 0.166666666667, 0.166666666667 };
    }
    ~TShape2D3(void) {}
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{ 1.0 - xi[i] - eta[i], xi[i], eta[i] };

        return v[j];
    }
    vector<double> shape_dxi(unsigned)
    {
        return { -1.0, 1.0, 0.0 };
    }
    vector<double> shape_deta(unsigned)
    {
        return { -1.0, 0.0, 1.0 };
    }
    double shape_dx(unsigned, unsigned j)
    {
        return c(1, j);
    }
    double shape_dy(unsigned, unsigned j)
    {
        return c(2, j);
    }
};
//-----------------------------------------------------------------------
//              Билинейные функции формы четырехугольного КЭ
//-----------------------------------------------------------------------
class TShape2D4 : public TShape
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1), x(i, 0) * x(i, 1) };

        return s[j];
    }
public:
    TShape2D4(void) : TShape()
    {
        size = 4;
        xi = { -0.57735027, -0.57735027, 0.57735027, 0.57735027 };
        eta = { -0.57735027, 0.57735027, -0.57735027, 0.57735027 };
        w = { 1.0, 1.0, 1.0, 1.0 };
    }
    ~TShape2D4(void) {}
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{ 0.25*(1.0 - xi[i])*(1.0 - eta[i]), 0.25*(1.0 + xi[i])*(1.0 - eta[i]), 0.25*(1.0 + xi[i])*(1.0 + eta[i]), 0.25*(1.0 - xi[i])*(1.0 + eta[i]) };

        return v[j];
    }
    vector<double> shape_dxi(unsigned i)
    {
        return {-0.25*(1.0 - eta[i]), 0.25*(1.0 - eta[i]), 0.25*(1.0 + eta[i]), -0.25*(1.0 + eta[i]) };
    }
    vector<double> shape_deta(unsigned i)
    {
        return { -0.25*(1.0 - xi[i]), -0.25*(1.0 + xi[i]), 0.25*(1.0 + xi[i]), 0.25*(1.0 - xi[i]) };
    }
    double shape_dx(unsigned i, unsigned j)
    {
        return c(1, j) + c(3, j) * x(i, 1);
    }
    double shape_dy(unsigned i, unsigned j)
    {
        return c(2, j) + c(3, j) * x(i, 0);
    }
};
//-----------------------------------------------------------------------
//               Квадратичные функции формы треугольного КЭ
//-----------------------------------------------------------------------
class TShape2D6 : public TShape2D3
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1), x(i, 0) * x(i, 1), x(i, 0) * x(i, 0), x(i, 1) * x(i, 1) };

        return s[j];
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
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{ (1.0 - xi[i] - eta[i]) * (2.0 * (1.0 - xi[i] - eta[i]) - 1.0), xi[i] * (2.0 * xi[i] - 1.0), eta[i] * (2.0 * eta[i] - 1.0), 4.0 * (1.0 - xi[i] - eta[i]) * xi[i],  4.0 * xi[i] * eta[i], 4.0 * (1.0 - xi[i] - eta[i]) * eta[i] };

        return v[j];
    }
    vector<double> shape_dxi(unsigned i)
    {
        return { -3.0 + 4.0 * xi[i] + 4.0 * eta[i], 4.0 * xi[i] - 1.0, 0.0, -8.0 * xi[i] + 4.0 - 4.0 * eta[i], 4.0 * eta[i], -4.0 * eta[i] };
    }
    vector<double> shape_deta(unsigned i)
    {
        return { -3.0 + 4.0 * xi[i] + 4.0 * eta[i], 0.0, 4.0 * eta[i] - 1.0, -4.0 * xi[i], 4.0 * xi[i], -8.0 * eta[i] + 4.0 - 4.0 * xi[i] };
    }
    double shape_dx(unsigned i, unsigned j)
    {
        return c(1, j) + c(3, j) * x(i, 1) + 2.0 * c(4, j) * x(i, 0);
    }
    double shape_dy(unsigned i, unsigned j)
    {
        return c(2, j) + c(3, j) * x(i, 0) + 2.0 * c(5, j) * x(i, 1);
    }
};
//-----------------------------------------------------------------------
//              Линейные функции формы тетраэдрального КЭ
//-----------------------------------------------------------------------
class TShape3D4 : public TShape
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1), x(i, 2) };

        return s[j];
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
    ~TShape3D4() {}
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{ 1.0 - xi[i] - eta[i] - psi[i], xi[i], eta[i], psi[i] };

        return v[j];
    }
    vector<double> shape_dxi(unsigned)
    {
        return { -1.0, 1.0, 0.0, 0.0 };
    }
    vector<double> shape_deta(unsigned)
    {
        return { -1.0, 0.0, 1.0, 0.0 };
    }
    vector<double> shape_dpsi(unsigned)
    {
        return { -1.0, 0.0, 0.0, 1.0 };
    }
    double shape_dx(unsigned, unsigned j)
    {
        return c(1, j);
    }
    double shape_dy(unsigned, unsigned j)
    {
        return c(2, j);
    }
    double shape_dz(unsigned, unsigned j)
    {
        return c(3, j);
    }
};
//-----------------------------------------------------------------------
//    Билинейные функции формы шестигранной четырехугольной призмы
//-----------------------------------------------------------------------
class TShape3D8 : public TShape
{
protected:
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1), x(i, 2), x(i, 0) * x(i, 1), x(i, 0) * x(i, 2), x(i, 1) * x(i, 2), x(i, 0) * x(i, 1) * x(i, 2) };

        return s[j];
    }
public:
    TShape3D8(void) : TShape()
    {
        size = 8;
        xi = { -0.57735026919, -0.57735026919, -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919, 0.57735026919, 0.57735026919 };
        eta = { -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919, -0.57735026919, -0.57735026919, 0.57735026919, 0.57735026919 };
        psi = { -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919, -0.57735026919, 0.57735026919 };
        w = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
    }
    ~TShape3D8(void) {}
    double shape(unsigned i, unsigned j)
    {
        vector<double> v{
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
    vector<double> shape_dxi(unsigned i)
    {
        return { -0.125 * (1.0 - eta[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 - eta[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 + eta[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 + eta[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 - eta[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 - eta[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 + eta[i]) * (1.0 + psi[i]),
                 -0.125 * (1.0 + eta[i]) * (1.0 + psi[i]) };
    }
    vector<double> shape_deta(unsigned i)
    {
        return { -0.125 * (1.0 - xi[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 - psi[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 - psi[i]),
                 -0.125 * (1.0 - xi[i]) * (1.0 + psi[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 + psi[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 + psi[i]) };
    }
    vector<double> shape_dpsi(unsigned i)
    {
        return { -0.125 * (1.0 - xi[i]) * (1.0 - eta[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 - eta[i]),
                 -0.125 * (1.0 + xi[i]) * (1.0 + eta[i]),
                 -0.125 * (1.0 - xi[i]) * (1.0 + eta[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 - eta[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 - eta[i]),
                  0.125 * (1.0 + xi[i]) * (1.0 + eta[i]),
                  0.125 * (1.0 - xi[i]) * (1.0 + eta[i]) };
    }
    double shape_dx(unsigned i, unsigned j)
    {
        return c(1, j) + c(4, j) * x(i, 1) + c(5, j) * x(i, 2) + c(7, j) * x(i, 1) * x(i, 2);
    }
    double shape_dy(unsigned i, unsigned j)
    {
        return c(2, j) + c(4, j) * x(i, 0) + c(6, j) * x(i, 2) + c(7, j) * x(i, 0) * x(i, 2);
    }
    double shape_dz(unsigned i, unsigned j)
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
    double shape_coeff(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0, x(i, 0), x(i, 1), x(i, 2), x(i, 0) * x(i, 1), x(i, 0) * x(i, 2), x(i, 1) * x(i, 2), x(i, 0) * x(i, 0), x(i, 1) * x(i, 1), x(i, 2) * x(i, 2) };

        return s[j];
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
    double shape(unsigned i, unsigned j)
    {
        vector<double> s = { 1.0 - xi[i] - eta[i] - psi[i], xi[i], eta[i], psi[i] },
                       v{ s[0] * (2.0 * s[0] - 1.0), s[1] * (2.0 * s[1] - 1.0), s[2] * (2.0 * s[2] - 1.0), s[3] * (2.0 * s[3] - 1.0),
                          4.0 * s[0] * s[1], 4.0 * s[1] * s[2], 4.0 * s[0] * s[2], 4.0 * s[2] * s[3], 4.0 * s[1] * s[3], 4.0 * s[0] * s[3] };

        return v[j];
    }
    vector<double> shape_dxi(unsigned i)
    {
        return { -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 4.0 * xi[i] - 1.0,
                  0.0, 0.0, -8.0 * xi[i] + 4.0 - 4.0 * eta[i] - 4.0 * psi[i],
                  4.0 * eta[i], -4.0 * eta[i], 0.0, 4.0 * psi[i], -4.0 * psi[i] };
    }
    vector<double> shape_deta(unsigned i)
    {
        return { -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 0.0, 4.0 * eta[i] - 1.0, 0.0,
                 -4.0 * xi[i], 4.0 * xi[i], -8.0 * eta[i] + 4.0 - 4.0 * xi[i] - 4.0 * psi[i],
                  4.0 * psi[i], 0.0, -4.0 * psi[i] };
    }
    vector<double> shape_dpsi(unsigned i)
    {
        return { -3.0 + 4.0 * xi[i] + 4.0 * eta[i] + 4.0 * psi[i], 0.0, 0.0, 4.0 * psi[i] - 1.0,
                 -4.0 * xi[i], 0.0, -4.0 * eta[i], 4.0 * eta[i], 4.0 * xi[i],
                 -8.0 * psi[i] + 4.0 - 4.0 * xi[i] - 4.0 * eta[i] };
    }
    double shape_dx(unsigned i, unsigned j)
    {
        return c(1, j) + 2.0 * c(7, j) * x(i, 0) + c(4, j) * x(i, 1) + c(5, j) * x(i, 2);
    }
    double shape_dy(unsigned i, unsigned j)
    {
        return c(2, j) + c(4, j) * x(i, 0) + 2.0 * c(8, j) * x(i, 1) + c(6, j) * x(i, 2);
    }
    double shape_dz(unsigned i, unsigned j)
    {
        return c(3, j) + c(5, j) * x(i, 0) + c(6, j) * x(i, 1) + 2.0 * c(9, j) * x(i, 2);
    }
};

#endif // SHAPE_H
