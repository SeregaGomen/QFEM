#ifndef LIST_H
#define LIST_H

#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include "msg/msg.h"
#include "util/matrix.h"

// Коды направлений вдоль осей координат
#define DIR_X     1 // 0000000000000001
#define DIR_Y     2 // 0000000000000010
#define DIR_Z     4 // 0000000000000100

// Коды начальных условий
#define FUN_U     1 // 0000000000000001
#define FUN_V     2 // 0000000000000010
#define FUN_W     4 // 0000000000000100
#define FUN_UT    8 // 0000000000001000
#define FUN_VT   16 // 0000000000010000
#define FUN_WT   32 // 0000000000100000
#define FUN_UTT  64 // 0000000001000000
#define FUN_VTT 128 // 0000000010000000
#define FUN_WTT 256 // 0000000100000000

using namespace std;

// Типы параметров расчета:
enum {
    EMPTY_PARAMETER = 0,
    INITIAL_CONDITION_PARAMETER = 1,        // Начальные условия
    BOUNDARY_CONDITION_PARAMETER = 2,       // Граничное условие, заданноe выражением
    VOLUME_LOAD_PARAMETER = 4,              // Объемная нагрузка
    SURFACE_LOAD_PARAMETER = 8,             // Поверхностная ...
    CONCENTRATED_LOAD_PARAMETER = 16,       // Сосредоточенная ...
    PRESSURE_LOAD_PARAMETER = 32,           // Нагрузка давлением
    YOUNG_MODULUS_PARAMETER = 64,           // Модуль Юнга
    POISSON_RATIO_PARAMETER = 128,          // Коэффициент Пуассона
    THICKNESS_PARAMETER = 256,              // Толщина элемента
    TEMPERATURE_PARAMETER = 512,            // Разность температур
    ALPHA_PARAMETER = 1024,                 // Коэффициент теплового расширения
    DENSITY_PARAMETER = 2048,               // Плотность
    DAMPING_PARAMETER = 4096,               // Параметр демпфирования
    STRESS_STRAIN_CURVE_PARAMETER = 8192    // Диаграмма деформирования
};

extern TMessenger* msg;

// Названия параметров
inline string paramName(int type)
{
    string ret;

    switch (type)
    {
        case INITIAL_CONDITION_PARAMETER:
            ret = S_INITIAL_CONDITION_PARAMETER;
            break;
        case BOUNDARY_CONDITION_PARAMETER:
            ret = S_BOUNDARY_CONDITION_PARAMETER;
            break;
        case VOLUME_LOAD_PARAMETER:
            ret = S_VOLUME_LOAD_PARAMETER;
            break;
        case SURFACE_LOAD_PARAMETER:
            ret = S_SURFACE_LOAD_PARAMETER;
            break;
        case CONCENTRATED_LOAD_PARAMETER:
            ret = S_CONCENTRATED_LOAD_PARAMETER;
            break;
        case PRESSURE_LOAD_PARAMETER:
            ret = S_PRESSURE_LOAD_PARAMETER;
            break;
        case YOUNG_MODULUS_PARAMETER:
            ret = S_YOUNG_MODULUS_PARAMETER;
            break;
        case POISSON_RATIO_PARAMETER:
            ret = S_POISSON_RATIO_PARAMETER;
            break;
        case THICKNESS_PARAMETER:
            ret = S_THICKNESS_PARAMETER;
            break;
        case TEMPERATURE_PARAMETER:
            ret = S_TEMPERATURE_PARAMETER;
            break;
        case ALPHA_PARAMETER:
            ret = S_ALPHA_PARAMETER;
            break;
        case DENSITY_PARAMETER:
            ret = S_DENSITY_PARAMETER;
            break;
        case DAMPING_PARAMETER:
            ret = S_DAMPING_PARAMETER;
            break;
        case STRESS_STRAIN_CURVE_PARAMETER:
            ret = S_STRESS_STRAIN_CURVE_PARAMETER;
    }
    return ret;
}

//--------------------------------------------------------------
//  Представление параметра в виде строки, функции или матрицы
//--------------------------------------------------------------
struct ParamValue
{
    string s_value;
    function<double (double, double, double, double)> f_value;
    matrix<double> m_value;
    void operator = (string val)
    {
        s_value = val;
    }
    void operator = (function<double (double, double, double, double)> val)
    {
        f_value = val;
    }
    void operator = (const matrix<double>& val)
    {
        m_value = val;
    }
};

//------------------------------------------------------
//              Описание параметра расчета
//------------------------------------------------------
class TParameter
{
private:
    // Тип параметра
    int type;
    // Направление (для краевого условия )
    int direct = 0;
    // Значение параметра в виде числовой константы
    double d_value = 0;
    // Значение параметра в виде функционального выражения или матрицы
    ParamValue e_value;
    ParamValue predicate;
public:
    TParameter(void)
    {
        type = EMPTY_PARAMETER;
    }
    TParameter(int t, double v, string p, int d)
    {
        type = t;
        d_value = v;
        predicate = p;
        direct = d;
    }
    TParameter(int t, string e, string p, int d)
    {
        type = t;
        e_value = e;
        predicate = p;
        direct = d;
    }
    TParameter(int t, double v, function<double (double, double, double, double)> p, int d)
    {
        type = t;
        d_value = v;
        predicate = p;
        direct = d;
    }
    TParameter(int t, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d)
    {
        type = t;
        e_value = e;
        predicate = p;
        direct = d;
    }
    TParameter(matrix<double>& m, string p)
    {
        type = STRESS_STRAIN_CURVE_PARAMETER;
        e_value = m;
        predicate = p;
    }
    TParameter(matrix<double>& m, function<double (double, double, double, double)> p)
    {
        type = STRESS_STRAIN_CURVE_PARAMETER;
        e_value = m;
        predicate = p;
    }
    TParameter(const TParameter& r)
    {
        direct = r.direct;
        type = r.type;
        d_value = r.d_value;
        e_value = r.e_value;
        predicate = r.predicate;
    }
    ~TParameter(void) {}
    TParameter operator = (const TParameter& r)
    {
        direct = r.direct;
        type = r.type;
        d_value = r.d_value;
        e_value = r.e_value;
        predicate = r.predicate;
        return *this;
    }
    int getDirect(void) const
    {
        return direct;
    }
    double getValue(void) const
    {
        return d_value;
    }
    string getPredicate(void) const
    {
        return predicate.s_value;
    }
    string getExpression(void) const
    {
        return e_value.s_value;
    }
    int getType(void) const
    {
        return type;
    }
    matrix<double>& getStressStrainCurve(void)
    {
        return e_value.m_value;
    }
    double getStressStrainCurve(unsigned i, unsigned j)
    {
        return e_value.m_value[i][j];
    }
    bool isFuncPredicate(void)
    {
        return (predicate.f_value) ? true : false;
    }
    bool isFuncExpression(void)
    {
        return (e_value.f_value) ? true : false;
    }
    double getFuncExpression(double x, double y, double z, double t = 0)
    {
        return e_value.f_value(x, y, z, t);
    }
    double getFuncPredicate(double x, double y, double z, double t = 0)
    {
        return predicate.f_value(x, y, z, t);
    }
};
//------------------------------------------------------
//     Класс, реализующий список параметров расчета
//------------------------------------------------------
class TParameterList : public list<TParameter>
{
public:
    TParameterList(void) {}
    ~TParameterList(void) {}
    void addParameter(int t, double v, string p, int d = 0)
    {
        TParameter c(t, v, p, d);

        push_back(c);
    }
    void addParameter(int t, string e, string p, int d = 0)
    {
        TParameter c(t, e, p, d);

        push_back(c);
    }
    void addParameter(int t, double v, function<double (double, double, double, double)> p, int d = 0)
    {
        TParameter c(t, v, p, d);

        push_back(c);
    }
    void addParameter(int t, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d = 0)
    {
        TParameter c(t, e, p, d);

        push_back(c);
    }
    void addStressStrainCurve(matrix<double> ssc, string p)
    {
        TParameter c(ssc, p);

        push_back(c);
    }
    void addStressStrainCurve(matrix<double> ssc, function<double (double, double, double, double)> p)
    {
        TParameter c(ssc, p);

        push_back(c);
    }
    void addThickness(double v, string p)
    {
        addParameter(THICKNESS_PARAMETER, v, p);
    }
    void addThickness(string e, string p)
    {
        addParameter(THICKNESS_PARAMETER, e, p);
    }
    void addThickness(double v, function<double (double, double, double, double)> p)
    {
        addParameter(THICKNESS_PARAMETER, v, p);
    }
    void addThickness(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(THICKNESS_PARAMETER, e, p);
    }
    void addTemperature(double v, string p)
    {
        addParameter(TEMPERATURE_PARAMETER, v, p);
    }
    void addTemperature(string e, string p)
    {
        addParameter(TEMPERATURE_PARAMETER, e, p);
    }
    void addTemperature(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(TEMPERATURE_PARAMETER, e, p);
    }
    void addTemperature(double v, function<double (double, double, double, double)> p)
    {
        addParameter(TEMPERATURE_PARAMETER, v, p);
    }
    void addAlpha(double v, string p)
    {
        addParameter(ALPHA_PARAMETER, v, p);
    }
    void addAlpha(string e, string p)
    {
        addParameter(ALPHA_PARAMETER, e, p);
    }
    void addAlpha(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ALPHA_PARAMETER, e, p);
    }
    void addAlpha(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ALPHA_PARAMETER, v, p);
    }
    void addDensity(double v, string p)
    {
        addParameter(DENSITY_PARAMETER, v, p);
    }
    void addDensity(string e, string p)
    {
        addParameter(DENSITY_PARAMETER, e, p);
    }
    void addDensity(double v, function<double (double, double, double, double)> p)
    {
        addParameter(DENSITY_PARAMETER, v, p);
    }
    void addDensity(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(DENSITY_PARAMETER, e, p);
    }
    void addDamping(double v, string p)
    {
        addParameter(DAMPING_PARAMETER, v, p);
    }
    void addDamping(string e, string p)
    {
        addParameter(DAMPING_PARAMETER, e, p);
    }
    void addDamping(double v, function<double (double, double, double, double)> p)
    {
        addParameter(DAMPING_PARAMETER, v, p);
    }
    void addDamping(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(DAMPING_PARAMETER, e, p);
    }
    void addYoungModulus(double v, string p)
    {
        addParameter(YOUNG_MODULUS_PARAMETER, v, p);
    }
    void addYoungModulus(string e, string p)
    {
        addParameter(YOUNG_MODULUS_PARAMETER, e, p);
    }
    void addYoungModulus(double v, function<double (double, double, double, double)> p)
    {
        addParameter(YOUNG_MODULUS_PARAMETER, v, p);
    }
    void addYoungModulus(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(YOUNG_MODULUS_PARAMETER, e, p);
    }
    void addPoissonRatio(double v, string p)
    {
        addParameter(POISSON_RATIO_PARAMETER, v, p);
    }
    void addPoissonRatio(string e, string p)
    {
        addParameter(POISSON_RATIO_PARAMETER, e, p);
    }
    void addPoissonRatio(double v, function<double (double, double, double, double)> p)
    {
        addParameter(POISSON_RATIO_PARAMETER, v, p);
    }
    void addPoissonRatio(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(POISSON_RATIO_PARAMETER, e, p);
    }
    void addBoundaryCondition(double v, string p, int d)
    {
        addParameter(BOUNDARY_CONDITION_PARAMETER, v, p, d);
    }
    void addBoundaryCondition(string e, string p, int d)
    {
        addParameter(BOUNDARY_CONDITION_PARAMETER, e, p, d);
    }
    void addBoundaryCondition(double v, function<double (double, double, double, double)> p, int d)
    {
        addParameter(BOUNDARY_CONDITION_PARAMETER, v, p, d);
    }
    void addBoundaryCondition(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d)
    {
        addParameter(BOUNDARY_CONDITION_PARAMETER, e, p, d);
    }
    void addInitialCondition(double v, int d)
    {
        addParameter(INITIAL_CONDITION_PARAMETER, v, "", d);
    }
    void addInitialCondition(string e, int d)
    {
        addParameter(INITIAL_CONDITION_PARAMETER, e, "", d);
    }
    void addInitialCondition(function<double (double, double, double, double)> e, int d)
    {
        addParameter(INITIAL_CONDITION_PARAMETER, e, nullptr, d);
    }
    void addSurfaceLoad(double v, string p, int d)
    {
        addParameter(SURFACE_LOAD_PARAMETER, v, p, d);
    }
    void addSurfaceLoad(string e, string p, int d)
    {
        addParameter(SURFACE_LOAD_PARAMETER, e, p, d);
    }
    void addSurfaceLoad(double v, function<double (double, double, double, double)> p, int d)
    {
        addParameter(SURFACE_LOAD_PARAMETER, v, p, d);
    }
    void addSurfaceLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d)
    {
        addParameter(SURFACE_LOAD_PARAMETER, e, p, d);
    }
    void addVolumeLoad(double v, string p, int d)
    {
        addParameter(VOLUME_LOAD_PARAMETER, v, p, d);
    }
    void addVolumeLoad(string e, string p, int d)
    {
        addParameter(VOLUME_LOAD_PARAMETER, e, p, d);
    }
    void addVolumeLoad(double v, function<double (double, double, double, double)> p, int d)
    {
        addParameter(VOLUME_LOAD_PARAMETER, v, p, d);
    }
    void addVolumeLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d)
    {
        addParameter(VOLUME_LOAD_PARAMETER, e, p, d);
    }
    void addConcentratedLoad(double v, string p, int d)
    {
        addParameter(CONCENTRATED_LOAD_PARAMETER, v, p, d);
    }
    void addConcentratedLoad(string e, string p, int d)
    {
        addParameter(CONCENTRATED_LOAD_PARAMETER, e, p, d);
    }
    void addConcentratedLoad(double v, function<double (double, double, double, double)> p, int d)
    {
        addParameter(CONCENTRATED_LOAD_PARAMETER, v, p, d);
    }
    void addConcentratedLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int d)
    {
        addParameter(CONCENTRATED_LOAD_PARAMETER, e, p, d);
    }
    void addPressureLoad(double v, string p)
    {
        addParameter(PRESSURE_LOAD_PARAMETER, v, p, 0);
    }
    void addPressureLoad(string e, string p)
    {
        addParameter(PRESSURE_LOAD_PARAMETER, e, p, 0);
    }
    void addPressureLoad(double v, function<double (double, double, double, double)> p)
    {
        addParameter(PRESSURE_LOAD_PARAMETER, v, p, 0);
    }
    void addPressureLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(PRESSURE_LOAD_PARAMETER, e, p, 0);
    }
    unsigned findParameter(int type)
    {
        unsigned counter = 0;

        for (auto it = begin(); it != end(); it++)
            if (it->getType() == type)
                counter++;
        return counter;
    }
};
#endif // LIST_H
