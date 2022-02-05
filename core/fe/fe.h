#ifndef FE_H
#define FE_H

#include "shape.h"

using namespace std;

// Типы конечных элементов
enum class FEType { undefined  = 0, fe1d2, fe2d3, fe2d4, fe2d6, fe2d3p, fe2d4p, fe2d6p, fe3d4, fe3d8, fe3d10, fe3d3s, fe3d4s, fe3d6s };

//---------------------------------------------------------
// Абстрактный изопараметрический класс конечного элемента
//---------------------------------------------------------
class TFE
{
protected:
    // Функции формы КЭ
    TShape *shape = nullptr;
    // Количество степеней свободы конечного элемента
    unsigned freedom = 0;
    // Модуль Юнга
    double e = 0;
    // Коэффициент Пуассона
    double m = 0;
    // Плотность
    double density = 0;
    // Параметр демпфирования
    double damping = 0;
    // Толщина (для плоских и оболочечных элементов) или площадь сечение (для одномерных КЭ)
    double thickness;
    // Температура
    double temperature = 0;
    // Коэффициент температурного расширения
    double alpha = 0;
    // Погрешность сравнения с нулем (для определения "плохих" КЭ)
    double eps = 1.0E-10;
    // Локальные матрицы жесткости, массы и сопротивления (демпфирования)
    matrix<double> K;
    matrix<double> M;
    matrix<double> D;
    // Вектор-столбец нагрузки
    matrix<double> load;
public:
    TFE(void) = default;
    virtual ~TFE(void)
    {
        delete shape;
    }
    virtual void setCoord(matrix<double> &x)
    {
        shape->create(x);
    }
    void setYoungModulus(double e)
    {
        this->e = e;
    }
    void setPoissonRatio(double m)
    {
        this->m = m;
    }
    void setEps(double eps)
    {
        this->eps = eps;
    }
    void setThickness(double thickness)
    {
        this->thickness = thickness;
    }
    void setDensity(double density)
    {
        this->density = density;
    }
    void setDamping(double damping)
    {
        this->damping = damping;
    }
    void setAlpha(double alpha)
    {
        this->alpha = alpha;
    }
    void setTemperature(double temperature)
    {
        this->temperature = temperature;
    }
    unsigned getSize(void)
    {
        return shape->size;
    }
    unsigned getFreedom(void)
    {
        return freedom;
    }
    double getStiffnessMatrix(unsigned i, unsigned j)
    {
        return K(i, j);
    }
    double getMassMatrix(unsigned i, unsigned j)
    {
        return M(i, j);
    }
    double getDampingMatrix(unsigned i, unsigned j)
    {
        return D(i, j);
    }
    double getLoad(unsigned i)
    {
        return load(i, 0);
    }
    double getYoungModulus(void)
    {
        return e;
    }
    double getPoissonRatio(void)
    {
        return m;
    }
    // Вычисление стандартных результатов КЭ (деформаций, напряжений, etc)
    virtual void calc(matrix<double>&, vector<double>&) = 0;
    // Процедура генерации локальных матриц жесткости, масс, демпфирования
    virtual void generate(bool = true) = 0;
};
//---------------------------------------------------------

#endif // FE_H_
