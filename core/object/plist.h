#ifndef LIST_H
#define LIST_H

#include <list>
//#include <fstream>
#include <string>
#include "msg/msg.h"
#include "util/matrix.h"

template <typename T> bool contains(const T lhs, const T rhs)
{
    return ((static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)) == static_cast<unsigned>(rhs)) ? true : false;
}
// Коды направлений вдоль осей координат
enum class Direction { Undefined = 0, X = 1, Y = 2, Z = 4 };
inline Direction operator | (Direction lhs, Direction rhs)
{
    return static_cast<Direction> (static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}

inline Direction operator |= (Direction& lhs, Direction rhs)
{
    return (lhs = static_cast<Direction>(lhs) | static_cast<Direction>(rhs));
}

// Коды начальных условий
enum class InitialCondition { Undefined = 0, U = 1, V = 2, W = 4, Ut = 8, Vt = 16, Wt = 32, Utt = 64, Vtt = 128, Wtt = 256 };

using namespace std;

// Типы параметров расчета:
enum class ParamType {
    Undefined,
    InitialCondition,    // Начальные условия
    BoundaryCondition,   // Граничное условие, заданноe выражением
    VolumeLoad,          // Объемная нагрузка
    SurfaceLoad,         // Поверхностная ...
    ConcentratedLoad,    // Сосредоточенная ...
    PressureLoad,        // Нагрузка давлением
    YoungModulus,        // Модуль Юнга
    PoissonRatio,        // Коэффициент Пуассона
    Thickness,           // Толщина элемента
    Temperature,         // Разность температур
    Alpha,               // Коэффициент теплового расширения
    Density,             // Плотность
    Damping,             // Параметр демпфирования
    StressStrainCurve    // Диаграмма деформирования
};

extern TMessenger *msg;

// Названия параметров
inline string paramName(ParamType type)
{
    string ret;

    switch (type)
    {
        case ParamType::InitialCondition:
            ret = S_INITIAL_CONDITION_PARAMETER;
            break;
        case ParamType::BoundaryCondition:
            ret = S_BOUNDARY_CONDITION_PARAMETER;
            break;
        case ParamType::VolumeLoad:
            ret = S_VOLUME_LOAD_PARAMETER;
            break;
        case ParamType::SurfaceLoad:
            ret = S_SURFACE_LOAD_PARAMETER;
            break;
        case ParamType::ConcentratedLoad:
            ret = S_CONCENTRATED_LOAD_PARAMETER;
            break;
        case ParamType::PressureLoad:
            ret = S_PRESSURE_LOAD_PARAMETER;
            break;
        case ParamType::YoungModulus:
            ret = S_YOUNG_MODULUS_PARAMETER;
            break;
        case ParamType::PoissonRatio:
            ret = S_POISSON_RATIO_PARAMETER;
            break;
        case ParamType::Thickness:
            ret = S_THICKNESS_PARAMETER;
            break;
        case ParamType::Temperature:
            ret = S_TEMPERATURE_PARAMETER;
            break;
        case ParamType::Alpha:
            ret = S_ALPHA_PARAMETER;
            break;
        case ParamType::Density:
            ret = S_DENSITY_PARAMETER;
            break;
        case ParamType::Damping:
            ret = S_DAMPING_PARAMETER;
            break;
        case ParamType::StressStrainCurve:
            ret = S_STRESS_STRAIN_CURVE_PARAMETER;
            break;
        default:
            ret = S_UNDEFINED_PARAMETER;
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
    void operator = (const matrix<double> &val)
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
    ParamType type;
    // Направление (для граничного условия) или начальное условие
    int attr = 0;
    // Значение параметра в виде числовой константы
    double d_value = 0;
    // Значение параметра в виде функционального выражения или матрицы
    ParamValue e_value;
    ParamValue predicate;
public:
    TParameter(void)
    {
        type = ParamType::Undefined;
    }
    TParameter(ParamType t, double v, string p, int a)
    {
        type = t;
        d_value = v;
        predicate = p;
        attr = a;
    }
    TParameter(ParamType t, string e, string p, int a)
    {
        type = t;
        e_value = e;
        predicate = p;
        attr = a;
    }
    TParameter(ParamType t, double v, function<double (double, double, double, double)> p, int a)
    {
        type = t;
        d_value = v;
        predicate = p;
        attr = a;
    }
    TParameter(ParamType t, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int a)
    {
        type = t;
        e_value = e;
        predicate = p;
        attr = a;
    }
    TParameter(matrix<double> &m, string p)
    {
        type = ParamType::StressStrainCurve;
        e_value = m;
        predicate = p;
    }
    TParameter(matrix<double> &m, function<double (double, double, double, double)> p)
    {
        type = ParamType::StressStrainCurve;
        e_value = m;
        predicate = p;
    }
    TParameter(const TParameter &r)
    {
        attr = r.attr;
        type = r.type;
        d_value = r.d_value;
        e_value = r.e_value;
        predicate = r.predicate;
    }
    ~TParameter(void) {}
    TParameter operator = (const TParameter &r)
    {
        attr = r.attr;
        type = r.type;
        d_value = r.d_value;
        e_value = r.e_value;
        predicate = r.predicate;
        return *this;
    }
    InitialCondition getInitialCondition(void) const
    {
        return static_cast<InitialCondition>(attr);
    }
    Direction getDirect(void) const
    {
        return static_cast<Direction>(attr);
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
    ParamType getType(void) const
    {
        return type;
    }
    matrix<double> &getStressStrainCurve(void)
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
    void addParameter(ParamType t, double v, string p, int a = 0)
    {
        TParameter c(t, v, p, a);

        push_back(c);
    }
    void addParameter(ParamType t, string e, string p, int a = 0)
    {
        TParameter c(t, e, p, a);

        push_back(c);
    }
    void addParameter(ParamType t, double v, function<double (double, double, double, double)> p, int a = 0)
    {
        TParameter c(t, v, p, a);

        push_back(c);
    }
    void addParameter(ParamType t, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, int a = 0)
    {
        TParameter c(t, e, p, a);

        push_back(c);
    }
    void addStressStrainCurve(matrix<double> &ssc, string p)
    {
        TParameter c(ssc, p);

        push_back(c);
    }
    void addStressStrainCurve(matrix<double> &ssc, function<double (double, double, double, double)> p)
    {
        TParameter c(ssc, p);

        push_back(c);
    }
    void addThickness(double v, string p)
    {
        addParameter(ParamType::Thickness, v, p);
    }
    void addThickness(string e, string p)
    {
        addParameter(ParamType::Thickness, e, p);
    }
    void addThickness(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Thickness, v, p);
    }
    void addThickness(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Thickness, e, p);
    }
    void addTemperature(double v, string p)
    {
        addParameter(ParamType::Temperature, v, p);
    }
    void addTemperature(string e, string p)
    {
        addParameter(ParamType::Temperature, e, p);
    }
    void addTemperature(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Temperature, e, p);
    }
    void addTemperature(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Temperature, v, p);
    }
    void addAlpha(double v, string p)
    {
        addParameter(ParamType::Alpha, v, p);
    }
    void addAlpha(string e, string p)
    {
        addParameter(ParamType::Alpha, e, p);
    }
    void addAlpha(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Alpha, e, p);
    }
    void addAlpha(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Alpha, v, p);
    }
    void addDensity(double v, string p)
    {
        addParameter(ParamType::Density, v, p);
    }
    void addDensity(string e, string p)
    {
        addParameter(ParamType::Density, e, p);
    }
    void addDensity(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Density, v, p);
    }
    void addDensity(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Density, e, p);
    }
    void addDamping(double v, string p)
    {
        addParameter(ParamType::Damping, v, p);
    }
    void addDamping(string e, string p)
    {
        addParameter(ParamType::Damping, e, p);
    }
    void addDamping(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Damping, v, p);
    }
    void addDamping(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::Damping, e, p);
    }
    void addYoungModulus(double v, string p)
    {
        addParameter(ParamType::YoungModulus, v, p);
    }
    void addYoungModulus(string e, string p)
    {
        addParameter(ParamType::YoungModulus, e, p);
    }
    void addYoungModulus(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::YoungModulus, v, p);
    }
    void addYoungModulus(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::YoungModulus, e, p);
    }
    void addPoissonRatio(double v, string p)
    {
        addParameter(ParamType::PoissonRatio, v, p);
    }
    void addPoissonRatio(string e, string p)
    {
        addParameter(ParamType::PoissonRatio, e, p);
    }
    void addPoissonRatio(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::PoissonRatio, v, p);
    }
    void addPoissonRatio(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::PoissonRatio, e, p);
    }
    void addBoundaryCondition(double v, string p, Direction d)
    {
        addParameter(ParamType::BoundaryCondition, v, p, static_cast<int>(d));
    }
    void addBoundaryCondition(string e, string p, Direction d)
    {
        addParameter(ParamType::BoundaryCondition, e, p, static_cast<int>(d));
    }
    void addBoundaryCondition(double v, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::BoundaryCondition, v, p, static_cast<int>(d));
    }
    void addBoundaryCondition(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::BoundaryCondition, e, p, static_cast<int>(d));
    }
    void addInitialCondition(double v, InitialCondition i)
    {
        addParameter(ParamType::InitialCondition, v, "", static_cast<int>(i));
    }
    void addInitialCondition(string e, InitialCondition i)
    {
        addParameter(ParamType::InitialCondition, e, "", static_cast<int>(i));
    }
    void addInitialCondition(function<double (double, double, double, double)> e, InitialCondition i)
    {
        addParameter(ParamType::InitialCondition, e, nullptr, static_cast<int>(i));
    }
    void addSurfaceLoad(double v, string p, Direction d)
    {
        addParameter(ParamType::SurfaceLoad, v, p, static_cast<int>(d));
    }
    void addSurfaceLoad(string e, string p, Direction d)
    {
        addParameter(ParamType::SurfaceLoad, e, p, static_cast<int>(d));
    }
    void addSurfaceLoad(double v, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::SurfaceLoad, v, p, static_cast<int>(d));
    }
    void addSurfaceLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::SurfaceLoad, e, p, static_cast<int>(d));
    }
    void addVolumeLoad(double v, string p, Direction d)
    {
        addParameter(ParamType::VolumeLoad, v, p, static_cast<int>(d));
    }
    void addVolumeLoad(string e, string p, Direction d)
    {
        addParameter(ParamType::VolumeLoad, e, p, static_cast<int>(d));
    }
    void addVolumeLoad(double v, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::VolumeLoad, v, p, static_cast<int>(d));
    }
    void addVolumeLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::VolumeLoad, e, p, static_cast<int>(d));
    }
    void addConcentratedLoad(double v, string p, Direction d)
    {
        addParameter(ParamType::ConcentratedLoad, v, p, static_cast<int>(d));
    }
    void addConcentratedLoad(string e, string p, Direction d)
    {
        addParameter(ParamType::ConcentratedLoad, e, p, static_cast<int>(d));
    }
    void addConcentratedLoad(double v, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::ConcentratedLoad, v, p, static_cast<int>(d));
    }
    void addConcentratedLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p, Direction d)
    {
        addParameter(ParamType::ConcentratedLoad, e, p, static_cast<int>(d));
    }
    void addPressureLoad(double v, string p)
    {
        addParameter(ParamType::PressureLoad, v, p);
    }
    void addPressureLoad(string e, string p)
    {
        addParameter(ParamType::PressureLoad, e, p);
    }
    void addPressureLoad(double v, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::PressureLoad, v, p);
    }
    void addPressureLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        addParameter(ParamType::PressureLoad, e, p);
    }
    unsigned findParameter(ParamType type)
    {
        unsigned counter = 0;

        for (auto it = begin(); it not_eq end(); it++)
            if (it->getType() == type)
                counter++;
        return counter;
    }
};
#endif // LIST_H
