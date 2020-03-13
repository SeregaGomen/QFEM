#ifndef PARAMS_H
#define PARAMS_H

#include <vector>
#include <map>
#include "object/plist.h"
#include "fe/fe.h"


enum FEMType { StaticProblem, DynamicProblem }; // Тип задачи: статика или динамика
enum TimeMethod { TEmpty, Zinkevich, Wilson  }; // Способ аппроксимации по времени (стандартный, Вильсона etc.)
enum PlasticityMethod { Linear, MVS, MES };     // Метод расчета упруго-пластических задач (метод переменной жесткости, метод упругих решений etc.)

using namespace std;

// Параметры расчета задачи с использованием МКЭ
class TFEMParams
{
private:
    void getParam(int, vector<double>&, double&, matrix<double>&);
    double getScalarParam(int, vector<double>&);
    void getMatrixParam(vector<double>&, matrix<double>&);
    vector<unsigned> getFunIndex(FEType);
public:
    FEMType fType;                  // Тип задачи (статика, динамика,...)
    TimeMethod tMethod;             // Способ аппроксимации по времени
    PlasticityMethod pMethod;       // Метод решения задач теории пластичности
    int width;                      // Длина выводимых чисел при печати результата
    int precision;                  // Количество выводимых чисел после запятой
    double eps;                     // Точность вычислений
    double t0;                      // Начальный момент времени расчета
    double t1;                      // Конечный момент времени расчета
    double th;                      // Шаг по времени
    double theta;                   // Тета Вильсона
    double loadStep;                // Шаг по нагрузке при нелинейном расчете
    vector<string> names;           // Массив имен искомых функций
    TParameterList plist;           // Список краевых условий, нагрузок etc
    map<string, double> variables;  // Список дополнительных числовых параметров
    void clear(void)
    {
        fType = StaticProblem;
        tMethod = TEmpty;
        pMethod = Linear;
        eps = 1.0e-10;
        width = 12;
        precision = 5;
        loadStep = 1;
        theta = 1.37;
        t0 = t1 = th = 0;
        plist.clear();
        variables.clear();
        // Заполнение имен функций стандартными значениями
        names = stdNames();
        // Добавление стандартных параметров
        variables["eps"] = eps;
    }
    TFEMParams(void)
    {
        fType = StaticProblem;
        tMethod = TEmpty;
        pMethod = Linear;
        eps = 1.0E-10;
        width = 12;
        precision = 5;
        loadStep = 1;
        theta = 1.37;
        t0 = t1 = th = 0;
        // Заполнение имен функций стандартными значениями
        names = stdNames();
        // Добавление стандартных параметров
        variables["eps"] = eps;
    }
    TFEMParams(TFEMParams& r)
    {
        fType = r.fType;
        tMethod = r.tMethod;
        pMethod = r.pMethod;
        eps = r.eps;
        width = r.width;
        precision = r.precision;
        t0 = r.t0;
        t1 = r.t1;
        th = r.th;
        theta = r.theta;
        plist = r.plist;
        variables = r.variables;
        loadStep = r.loadStep;
        names = r.names;
    }
    ~TFEMParams(void)
    {
        clear();
    }
    void setFunName(vector<string>& fn)
    {
        names = fn;
    }
    unsigned numResult(FEType);
    string getName(unsigned, FEType);
    TFEMParams& operator = (TFEMParams& r)
    {
        fType = r.fType;
        tMethod = r.tMethod;
        pMethod = r.pMethod;
        eps = r.eps;
        width = r.width;
        precision = r.precision;
        t0 = r.t0;
        t1 = r.t1;
        th = r.th;
        theta = r.theta;
        plist = r.plist;
        variables = r.variables;
        loadStep = r.loadStep;
        names = r.names;
        return *this;
    }
    vector<string> stdNames(void)
    {
        // Названия функций и аргументов, принятые по умолчанию
        vector<string> stdName = {
            "x",   // 0  - идентификатор первого аргумента (x)
            "y",   // 1  - ... второго аргумента
            "z",   // 2  - ... третьего аргумента
            "t",   // 3  - ... времени
            "U",   // 4  - компонента вектора перемещений по первому направлению (x)
            "V",   // 5  - ... по y
            "W",   // 6  - ... по z
            "Tx",  // 7  - компонента поворота слоя пластины (оболочки) относительно оси x
            "Ty",  // 8  - ... y
            "Tz",  // 9  - ... z
            "Exx", // 10 - компонента тензора нормальных деформаций по xx
            "Eyy", // 11 - ... по yy
            "Ezz", // 12 - ... по zz
            "Exy", // 13 - компонента тензора тангенциальных деформаций по xу
            "Exz", // 14 - ... по xz
            "Eyz", // 15 - ... по yz
            "Sxx", // 16 - компонента тензора нормальных напряжений по xx
            "Syy", // 17 - ... по yy
            "Szz", // 18 - ... по zz
            "Sxy", // 19 - компонента тензора тангенциальных напряжений по xу
            "Sxz", // 20 - ... по xz
            "Syz", // 21 - ... по yz
            "Ut",  // 22 - скорость по x
            "Vt",  // 23 - ... по y
            "Wt",  // 24 - ... по z
            "Utt", // 25 - ускорение по x
            "Vtt", // 26 - ... по y
            "Wtt"  // 27 - ... по z
        };
        return stdName;
    }
    // Извлечение модуля Юнга, соответствующего заданной координате
    double getYoungModule(vector<double>& cx)
    {
        return getScalarParam(YOUNG_MODULUS_PARAMETER, cx);
    }
    // Извлечение коэффициента Пуассона, ...
    double getPoissonRatio(vector<double>& cx)
    {
        return getScalarParam(POISSON_RATIO_PARAMETER, cx);
    }
    // Извлечение толщины элемента
    double getThickness(vector<double>& cx)
    {
        return getScalarParam(THICKNESS_PARAMETER, cx);
    }
    // Извлечение температуры
    double getTemperature(vector<double>& cx)
    {
        return getScalarParam(TEMPERATURE_PARAMETER, cx);
    }
    // Извлечение коэффициента темперратурного расширения
    double getAlpha(vector<double>& cx)
    {
        return getScalarParam(ALPHA_PARAMETER, cx);
    }
    // Извлечение плотности
    double getDensity(vector<double>& cx)
    {
        return getScalarParam(DENSITY_PARAMETER, cx);
    }
    // Извлечение параметра демпфирования
    double getDamping(vector<double>& cx)
    {
        return getScalarParam(DAMPING_PARAMETER, cx);
    }
    // Извлечение диаграммы деформирования
    void getStressStrainCurve(vector<double>& cx, matrix<double>& ssc)
    {
        getMatrixParam(cx, ssc);
    }
    double getMinStress(void);
    bool getPredicateValue(TParameter&, vector<double>&);
    double getExpressionValue(TParameter&, vector<double>&);
    bool getPredicateValue(TParameter&, double, double, double);
    double getExpressionValue(TParameter&, double, double, double, double = 0);
};

#endif // PARAMS_H
