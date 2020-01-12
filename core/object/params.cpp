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
//    Название заданной функции в зависимости от размерности задачи
//--------------------------------------------------------------------
string TFEMParams::getName(unsigned i, FEType type)
{
    unsigned // U, Exx, Sxx, Ut, Utt
        index1d[] = { 4, 10, 16, 22, 25 },
        // U, V, Exx, Eyy, Exy, Sxx, Syy, Sxy, Ut, Vt, Utt, Vtt
        index2d[] = { 4, 5, 10, 11, 13, 16, 17, 19, 22, 23, 25, 26  },
        // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
        index2dp[] = { 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 },
        // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
        index3d[] = { 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 },
        // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
        index3ds[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };
    string ret;

    switch (type)
    {
        case FE1D2:
            ret = names[index1d[i]];
            break;
        case FE2D3:
        case FE2D4:
        case FE2D6:
            ret = names[index2d[i]];
            break;
        case FE2D3P:
        case FE2D4P:
        case FE2D6P:
            ret = names[index2dp[i]];
            break;
        case FE3D4:
        case FE3D8:
        case FE3D10:
            ret = names[index3d[i]];
            break;
        case FE3D3S:
        case FE3D4S:
        case FE3D6S:
            ret = names[index3ds[i]];
            break;
        default:
            ret = -1; // ?
    }
    return ret;
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
    for (auto it = plist.begin(); it != plist.end(); it++)
        if (it->getType() == p)
        {
            if (!getPredicateValue(*it, cx))
                continue;
            if (p == STRESS_STRAIN_CURVE_PARAMETER)
                m = it->getStressStrainCurve();
            else
                d = getExpressionValue(*it, cx);
            break;
        }
}
//--------------------------------------------------------------------
double TFEMParams::getMinStress(void)
{
    double res = DBL_MAX;

    for (auto it = plist.begin(); it != plist.end(); it++)
        if (it->getType() == STRESS_STRAIN_CURVE_PARAMETER)
        {
            if (!it->getStressStrainCurve().size1())
                throw NONLINEAR_PARAM_ERR;
            if (it->getStressStrainCurve().size1() && it->getStressStrainCurve(1, 0) < res)
                res = it->getStressStrainCurve(1, 0);
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


