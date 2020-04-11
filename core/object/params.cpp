#include <cfloat>
#include <string>
#include "params.h"
#include "parser/parser.h"

//--------------------------------------------------------------------
// Количество результатов в зависимости от типа и размерности задачи
//--------------------------------------------------------------------
unsigned TFEMParams::numResult(FEType type)
{
    unsigned ret = 0;

    switch (type)
    {
        case FE1D2:
            ret = (fType == StaticProblem) ? 3 : 5;
            break;
        case FE2D3:
        case FE2D4:
        case FE2D6:
            ret = (fType == StaticProblem) ? 8 : 12;
            break;
        case FE2D3P:
        case FE2D4P:
        case FE2D6P:
        case FE3D4:
        case FE3D8:
        case FE3D10:
            ret = (fType == StaticProblem) ? 15 : 21;
            break;
        case FE3D3S:
        case FE3D4S:
        case FE3D6S:
            ret = (fType == StaticProblem) ? 18 : 24;
            break;
        default:
            ret = 0;
    }
    return ret;
}
//--------------------------------------------------------------------
//    Индексы названий функций в зависимости от размерности задачи
//--------------------------------------------------------------------
vector<unsigned> TFEMParams::getFunIndex(FEType type)
{
    switch (type)
    {
        case FE1D2:
            // U, Exx, Sxx, Ut, Utt
            return { 4, 10, 16, 22, 25 };
        case FE2D3:
        case FE2D4:
        case FE2D6:
            // U, V, Exx, Eyy, Exy, Sxx, Syy, Sxy, Ut, Vt, Utt, Vtt
            return { 4, 5, 10, 11, 13, 16, 17, 19, 22, 23, 25, 26 };
        case FE2D3P:
        case FE2D4P:
        case FE2D6P:
            // W, Tx, Ty, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
            return { 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
        case FE3D4:
        case FE3D8:
        case FE3D10:
            // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
            return { 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
        case FE3D3S:
        case FE3D4S:
        case FE3D6S:
            // U, V, W, Tx, Ty, Tz, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
            return { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
        default:
            break; // ?
    }
    return {};
}
//--------------------------------------------------------------------
//    Название заданной функции в зависимости от размерности задачи
//--------------------------------------------------------------------
string TFEMParams::getName(unsigned i, FEType type)
{
    return names[getFunIndex(type)[i]];
}
//--------------------------------------------------------------------
// Извлечение значения параметра, соответствующего заданной координате
//--------------------------------------------------------------------
double TFEMParams::getScalarParam(int p, vector<double>& cx)
{
    matrix<double> tmp;
    double res = 0;

    getParam(p, cx, res, tmp);
    return res;
}
//--------------------------------------------------------------------
//           Извлечение диаграммы деформирования материала
//--------------------------------------------------------------------
void TFEMParams::getMatrixParam(vector<double>& cx, matrix<double>& res)
{
    double tmp;

    getParam(STRESS_STRAIN_CURVE_PARAMETER, cx, tmp, res);
}
//--------------------------------------------------------------------
void TFEMParams::getParam(int p, vector<double>& cx, double& d, matrix<double>& m)
{
    for (auto it : plist)
        if (it.getType() == p)
        {
            if (!getPredicateValue(it, cx))
                continue;
            if (p == STRESS_STRAIN_CURVE_PARAMETER)
                m = it.getStressStrainCurve();
            else
                d = getExpressionValue(it, cx);
            break;
        }
}
//--------------------------------------------------------------------
double TFEMParams::getMinStress(void)
{
    double res = DBL_MAX;

    for (auto it : plist)
        if (it.getType() == STRESS_STRAIN_CURVE_PARAMETER)
        {
            if (!it.getStressStrainCurve().size1())
                throw NONLINEAR_PARAM_ERR;
            if (it.getStressStrainCurve().size1() && it.getStressStrainCurve(1, 0) < res)
                res = it.getStressStrainCurve(1, 0);
        }
    return res;
}
//--------------------------------------------------------------------
//          Вычисление значения предиката в заданной точке
//--------------------------------------------------------------------
bool TFEMParams::getPredicateValue(TParameter& p, vector<double>& cx)
{
    TParser parser;

    if (p.isFuncPredicate())
        return bool(p.getFuncPredicate(cx[0], cx[1], cx[2]));
    if (!p.getPredicate().length())
        return true;

    parser.set_variables(variables);
    for (unsigned i = 0; i < cx.size(); i++)
        parser.set_variable(names[i], cx[i]);
    parser.set_expression(p.getPredicate());
    return bool(parser.run());
}
//--------------------------------------------------------------------
bool TFEMParams::getPredicateValue(TParameter& p, double cx, double cy, double cz)
{
    TParser parser;

    if (p.isFuncPredicate())
        return bool(p.getFuncPredicate(cx, cy, cz));
    if (!p.getPredicate().length())
        return true;

    parser.set_variables(variables);
    parser.set_variable(names[0], cx);
    parser.set_variable(names[1], cy);
    parser.set_variable(names[2], cz);
    parser.set_expression(p.getPredicate());
    return bool(parser.run());
}
//--------------------------------------------------------------------
//          Вычисление значения выражения в заданной точке
//--------------------------------------------------------------------
double TFEMParams::getExpressionValue(TParameter& p, vector<double>& cx)
{
    TParser parser;

    if (p.isFuncExpression())
        return p.getFuncExpression(cx[0], cx[1], cx[2], (cx.size() == 4) ? cx[3] : 0.0);
    if (!p.getExpression().length())
        return p.getValue();

    parser.set_variables(variables);
    for (unsigned i = 0; i < cx.size(); i++)
        parser.set_variable(names[i], cx[i]);

    parser.set_expression(p.getExpression());
    return parser.run();
}
//--------------------------------------------------------------------
double TFEMParams::getExpressionValue(TParameter& p, double cx, double cy, double cz, double t)
{
    TParser parser;

    if (p.isFuncExpression())
        return p.getFuncExpression(cx, cy, cz, t);
    if (!p.getExpression().length())
        return p.getValue();

    parser.set_variables(variables);
    parser.set_variable(names[0], cx);
    parser.set_variable(names[1], cy);
    parser.set_variable(names[2], cz);
    parser.set_expression(p.getExpression());
    return parser.run();
}
//--------------------------------------------------------------------
bool TFEMParams::write(ofstream& out)
{
    out << "Parameters" << endl;
    // Тип задачи
    out << fType << endl;

    // Способ аппроксимации по времени
    out << tMethod << endl;

    // Метод решения упруго-пластических задач
    out << pMethod << endl;

    // Погрешность расчета
    out << eps << endl;

    // Ширина и точность вывода
    out << width << ' ' << precision << endl;

    // Шаг по нагрузке
    out << loadStep << endl;

    // Параметры времени
    out << t0 << ' ' << t1 << ' ' << th << endl;

    // Названия функций
    out << names.size() << endl;
    for (unsigned i = 0; i < names.size(); i++)
        out << names[i].c_str() << endl;

    // Краевые условия, нагрузки, etc
    out << plist.size() << endl;
    for (auto it : plist)
    {
        out << it.getType() << endl;   // Тип условия
        out << it.getDirect() << endl; // Номер функции: 0 - X, ...
        out << it.getExpression() << endl;
        out << it.getPredicate() << endl;
    }

    // Вспомогательные параметры
    out << variables.size() << endl;
    for (map<string, double>::iterator it = variables.begin(); it != variables.end(); ++it)
        out << it->first.c_str() << ' ' << it->second << endl;
    return !out.fail();
}
//--------------------------------------------------------------------
bool TFEMParams::read(ifstream& in)
{
    string str,
           key;
    int type,
        dir;
    unsigned len;
    double value;

    in >> str;
    // Тип задачи
    in >> reinterpret_cast<int&>(fType);

    // Способ аппроксимации по времени
    in >> reinterpret_cast<int&>(tMethod);

    // Метод решения упруго-пластических задач
    in >> reinterpret_cast<int&>(pMethod);

    // Погрешность расчета
    in >> eps;

    // Ширина и точность вывода
    in >> width >> precision;

    // Шаг по нагрузке
    in >> loadStep;

    // Параметры времени
    in >> t0 >> t1 >> th;

    // Функции
    in >> len;
    for (unsigned i = 0; i < len; i++)
        in >> names[i];

    // Краевые условия, нагрузки, etc
    in >> len;
    plist.clear();
    for (unsigned i = 0; i < len; i++)
    {
        in >> type >> dir;
        getline(in, str); // endl
        getline(in, str);
        getline(in, key);
        plist.addParameter(type, str, key, dir);
    }

    // Вспомогательные параметры
    variables.clear();
    in >> len;
    for (unsigned i = 0; i < len; i++)
    {
        in >> key >> value;
        variables[key] = value;
    }
    return !in.fail();
}
//--------------------------------------------------------------------




