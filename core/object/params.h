#ifndef PARAMS_H
#define PARAMS_H

#include <vector>
#include <map>
#include "object/plist.h"
#include "fe/fe.h"


enum class FEMType { StaticProblem, DynamicProblem }; // Тип задачи: статика или динамика
enum class TimeMethod { Zinkevich, Wilson  }; // Способ аппроксимации по времени (стандартный, Вильсона etc.)
enum class PlasticityMethod { Linear, MVS, MES };     // Метод расчета упруго-пластических задач (метод переменной жесткости, метод упругих решений etc.)

using namespace std;

// Параметры расчета задачи с использованием МКЭ
class TFEMParams
{
private:
    void getParam(ParamType, matrix<double>&, double&, matrix<double>&);
    double getScalarParam(ParamType, matrix<double>&);
    void getMatrixParam(matrix<double>&, matrix<double>&);
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
        fType = FEMType::StaticProblem;
        tMethod = TimeMethod::Wilson;
        pMethod = PlasticityMethod::Linear;
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
        fType = FEMType::StaticProblem;
        tMethod = TimeMethod::Wilson;
        pMethod = PlasticityMethod::Linear;
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
    double getYoungModule(matrix<double> &x)
    {
        return getScalarParam(ParamType::YoungModulus, x);
    }
    // Извлечение коэффициента Пуассона, ...
    double getPoissonRatio(matrix<double> &x)
    {
        return getScalarParam(ParamType::PoissonRatio, x);
    }
    // Извлечение толщины элемента
    double getThickness(matrix<double> &x)
    {
        return getScalarParam(ParamType::Thickness, x);
    }
    // Извлечение температуры
    double getTemperature(matrix<double> &x)
    {
        return getScalarParam(ParamType::Temperature, x);
    }
    // Извлечение коэффициента темперратурного расширения
    double getAlpha(matrix<double> &x)
    {
        return getScalarParam(ParamType::Alpha, x);
    }
    // Извлечение плотности
    double getDensity(matrix<double> &x)
    {
        return getScalarParam(ParamType::Density, x);
    }
    // Извлечение параметра демпфирования
    double getDamping(matrix<double> &x)
    {
        return getScalarParam(ParamType::Damping, x);
    }
    // Извлечение диаграммы деформирования
    void getStressStrainCurve(matrix<double> &x, matrix<double>& ssc)
    {
        getMatrixParam(x, ssc);
    }
    double getMinStress(void);
    bool getPredicateValue(TParameter&, const vector<double>&);
    double getExpressionValue(TParameter&, const vector<double>&);
    bool getPredicateValue(TParameter&, double, double, double);
    double getExpressionValue(TParameter&, double, double, double, double = 0);
    bool write(ofstream&);
    bool read(ifstream&);
    void addYoungModulus(double e, string p = "")
    {
        plist.addYoungModulus(e, p);
    }
    void addPoissonRatio(double m, string p = "")
    {
        plist.addPoissonRatio(m, p);
    }
    void addThickness(double t, string p = "")
    {
        plist.addThickness(t, p);
    }
    void addBoundaryCondition(Direction dir, double v, string p)
    {
        plist.addBoundaryCondition(v, p, dir);
    }
    void addVoluneLoad(Direction dir, double v, string p = "")
    {
        plist.addVolumeLoad(v, p, dir);
    }
    void addPressureLoad(double v, string p = "")
    {
        plist.addPressureLoad(v, p);
    }
    void addConcentratedLoad(Direction dir, double v, string p = "")
    {
        plist.addConcentratedLoad(v, p, dir);
    }
    bool checkElm(matrix<double> &x, TParameter &p)
    {
        vector<double> vx(x.size2());

        for (auto i = 0u; i < x.size1(); i++)
        {
            for (auto j = 0u; j < x.size2(); j++)
                vx[j] = x[i][j];
            if (not getPredicateValue(p, vx))
                return false;
        }
        return true;
    }
    bool checkElmCenter(matrix<double> &x, TParameter &p)
    {
        vector<double> cx(x.size2());

        // Вычисление координат центра элемента
        for (auto j = 0u; j < x.size2(); j++)
        {
            double c{0};
            for (auto i = 0u; i < x.size1(); i++)
                c += x(i, j);
            cx[j] = c / double(x.size1());
        }
        if (not getPredicateValue(p, cx))
            return false;
        return true;
    }
};

#endif // PARAMS_H
