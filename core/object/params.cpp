#include <cfloat>
#include <string>
#include <fstream>
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
        case FEType::fe1d2:
            ret = (fType == FEMType::StaticProblem) ? 3 : 5;
            break;
        case FEType::fe2d3:
        case FEType::fe2d4:
        case FEType::fe2d6:
            ret = (fType == FEMType::StaticProblem) ? 8 : 12;
            break;
        case FEType::fe2d3p:
        case FEType::fe2d4p:
        case FEType::fe2d6p:
        case FEType::fe3d4:
        case FEType::fe3d8:
        case FEType::fe3d10:
            ret = (fType == FEMType::StaticProblem) ? 15 : 21;
            break;
        case FEType::fe3d3s:
        case FEType::fe3d4s:
        case FEType::fe3d6s:
            ret = (fType == FEMType::StaticProblem) ? 18 : 24;
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
        case FEType::fe1d2:
            // U, Exx, Sxx, Ut, Utt
            return { 4, 10, 16, 22, 25 };
        case FEType::fe2d3:
        case FEType::fe2d4:
        case FEType::fe2d6:
            // U, V, Exx, Eyy, Exy, Sxx, Syy, Sxy, Ut, Vt, Utt, Vtt
            return { 4, 5, 10, 11, 13, 16, 17, 19, 22, 23, 25, 26 };
        case FEType::fe2d3p:
        case FEType::fe2d4p:
        case FEType::fe2d6p:
            // W, Tx, Ty, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
            return { 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
        case FEType::fe3d4:
        case FEType::fe3d8:
        case FEType::fe3d10:
            // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
            return { 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
        case FEType::fe3d3s:
        case FEType::fe3d4s:
        case FEType::fe3d6s:
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
double TFEMParams::getScalarParam(ParamType p, matrix<double> &x)
{
    matrix<double> tmp;
    double res = 0;

    getParam(p, x, res, tmp);
    return res;
}
//--------------------------------------------------------------------
//           Извлечение диаграммы деформирования материала
//--------------------------------------------------------------------
void TFEMParams::getMatrixParam(matrix<double> &x, matrix<double> &res)
{
    double tmp;

    getParam(ParamType::StressStrainCurve, x, tmp, res);
}
//--------------------------------------------------------------------
void TFEMParams::getParam(ParamType p, matrix<double> &x, double &d, matrix<double> &m)
{
    for (auto it : plist)
        if (it.getType() == p)
        {
            if (checkElmCenter(x, it))
            {
                if (p == ParamType::StressStrainCurve)
                    m = it.getStressStrainCurve();
                else
                    d = getExpressionValue(it, ([x]() -> vector<double> {
                        vector<double> coord(3);

                        for (auto j = 0u; j < x.size2(); j++)
                        {
                            double c = 0;
                            for (auto i = 0u; i < x.size1(); i++)
                                c += x[i][j];
                            coord[j] = c / double(x.size1());
                        }
                        return coord;
                    })());
                break;
            }
        }
}
//--------------------------------------------------------------------
double TFEMParams::getMinStress(void)
{
    double res = DBL_MAX;

    for (auto it : plist)
        if (it.getType() == ParamType::StressStrainCurve)
        {
            if (not it.getStressStrainCurve().size1())
                throw ErrorCode::EStressStrainCurve;
            if (it.getStressStrainCurve().size1() && it.getStressStrainCurve(1, 0) < res)
                res = it.getStressStrainCurve(1, 0);
        }
    return res;
}
//--------------------------------------------------------------------
//          Вычисление значения предиката в заданной точке
//--------------------------------------------------------------------
bool TFEMParams::getPredicateValue(TParameter& p, const vector<double>& cx)
{
    TParser parser;

    if (p.isFuncPredicate())
        return bool(p.getFuncPredicate(cx[0], cx[1], cx[2]));
    if (not p.getPredicate().length())
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
    if (not p.getPredicate().length())
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
double TFEMParams::getExpressionValue(TParameter &p, const vector<double> &cx)
{
    TParser parser;

    if (p.isFuncExpression())
        return p.getFuncExpression(cx[0], cx[1], cx[2], (cx.size() == 4) ? cx[3] : 0.0);
    if (not p.getExpression().length())
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
    if (not p.getExpression().length())
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
    out << static_cast<int>(fType) << endl;

    // Способ аппроксимации по времени
    out << static_cast<int>(tMethod) << endl;

    // Метод решения упруго-пластических задач
    out << static_cast<int>(pMethod) << endl;

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
    for (auto &it : plist)
    {
        out << static_cast<int>(it.getType()) << endl;   // Тип условия
        out << static_cast<unsigned>(it.getDirect()) << endl; // Номер функции: 0 - X, ...
        out << it.getExpression() << endl;
        out << it.getPredicate() << endl;
    }

    // Вспомогательные параметры
    out << variables.size() << endl;
    for (map<string, double>::iterator it = variables.begin(); it not_eq variables.end(); ++it)
        out << it->first.c_str() << ' ' << it->second << endl;
    return not out.fail();
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
        plist.addParameter(static_cast<ParamType>(type), str, key, dir);
    }

    // Вспомогательные параметры
    variables.clear();
    in >> len;
    for (unsigned i = 0; i < len; i++)
    {
        in >> key >> value;
        variables[key] = value;
    }
    return not in.fail();
}
//--------------------------------------------------------------------




