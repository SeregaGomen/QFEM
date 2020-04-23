#ifndef FEMDYNAMIC_H
#define FEMDYNAMIC_H

#include "parser/parser.h"
#include "femstatic.h"

extern TMessenger* msg;

//----------------------------------------------------------
//  Реализация конечно-элементного расчета в соответствии с
//  вариационным принципом Гамильтона-Остроградского
//  (способ интегрирования - метод Вильсона)
//----------------------------------------------------------
template <class T> class TFEMDynamic : public TFEMStatic<T>
{
private:
    double t; // Текущий момент времени
    matrix<double> u0;  // Начальные условия и результаты U,V,W,Ut,... по итерациям
    void createDynamicMatrix(double, double);
    void createDynamicVector(matrix<double>&, double, double);
protected:
    void ansambleLocalMatrix(TFE*, unsigned);
    void getInitialCondition(void);
    void setBoundaryConstant(unsigned,unsigned,double);
    void setLoadConstant(unsigned,unsigned,double);
    void calcDynamicResult(matrix<double>&);
    void createDynamicVector(void);
    void genResults(vector<double>&,bool isAdd = false);
    virtual void saveResult(matrix<double>&,bool);
public:
    TFEMDynamic(string n,TMesh* m,TResultList* r,list<string>* l) : TFEMStatic<T>(n,m,r,l)
    {
        t = 0;
        TFEM::params.fType = FEMType::DynamicProblem;
    }
    virtual ~TFEMDynamic(void) {}
    void startProcess(void);
};
//----------------------------------------------------------
//                     Запуск расчета
//----------------------------------------------------------
template<class T> void TFEMDynamic<T>::startProcess(void)
{
    unsigned hour,
             min,
             sec;
    vector<double> result;
    chrono::system_clock::time_point timer = chrono::system_clock::now();
    ostringstream out;

    t = TFEMStatic<T>::params.t0 + TFEMStatic<T>::params.th;
    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;
    TFEMStatic<T>::solver.setMatrix(TFEM::mesh, true);

    // Формирование глобальных матриц жесткости, масс и демпфирования
    TFEMStatic<T>::calcGlobalMatrix(false);

    // Формирование "статической" левой части СЛАУ и сохранение ее для последующего использования
    createDynamicMatrix(TFEM::params.th, TFEM::params.theta);

    // Учет начальных условий
    getInitialCondition();

    // Итерационный процесс по времени
    cout << S_MSG_TIME_ITERATION << endl;
    while (t <= TFEM::params.t1)
    {
        cout << TFEM::params.names[3] << '=' << t << endl;
        // Формирование динамической правой части СЛАУ
        createDynamicVector();
        // Учет граничных условий
        TFEMStatic<T>::calcBoundaryCondition();
        if (TFEMStatic<T>::solver.solve(result,TFEM::params.eps,TFEM::isProcessAborted))
            genResults(result); // Вычисление результатов
        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        TFEM::printResultSummary(t);
        if (fabs(TFEM::params.t1 - (t + TFEM::params.th)) <= TFEM::params.eps)
            t = TFEM::params.t1;
        else
            t += TFEM::params.th;
    }

    TFEMStatic<T>::solver.clear();
    TFEM::isProcessStarted = false;
    TFEM::isProcessCalculated = true;

    hour = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) / 3600;
    min = (unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) % 3600) / 60;
    sec = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) - hour * 3600 - min * 60;
    // Сохраняем информацию о времени расчета
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ');
    TFEM::notes->push_back(out.str());
    cout << out.str() << endl;
}
//-------------------------------------------------------------
//           Ансамблирование ЛМЖ, ЛММ и ЛМД к ГМЖ
//-------------------------------------------------------------
template<class T> void TFEMDynamic<T>::ansambleLocalMatrix(TFE* fe, unsigned i)
{
    unsigned freedom = fe->getFreedom(),
             size = fe->getSize() * fe->getFreedom();

    // Ансамблирование матрицы
    for (unsigned l = 0; l < size; l++)
    {
        for (unsigned k = l; k < size; k++)
        {
            TFEMStatic<T>::solver.addMatrixStiffnessElement(fe->getStiffnessMatrix(l, k), TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            TFEMStatic<T>::solver.addMatrixMassElement(fe->getMassMatrix(l, k), TFEM::mesh->getFE(i, l / freedom)*freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            TFEMStatic<T>::solver.addMatrixDampingElement(fe->getDampingMatrix(l, k), TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            if (l not_eq k)
            {
                TFEMStatic<T>::solver.addMatrixStiffnessElement(fe->getStiffnessMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
                TFEMStatic<T>::solver.addMatrixMassElement(fe->getMassMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
                TFEMStatic<T>::solver.addMatrixDampingElement(fe->getDampingMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
            }
        }
        TFEMStatic<T>::solver.addLoadElement(fe->getLoad(l), l);
    }
}
//---------------------------------------------------------
template<class T> void TFEMDynamic<T>::getInitialCondition(void)
{
    TParser parser;
    unsigned direct;
    double value;

    u0.resize(9, TFEM::mesh->getNumVertex());
    for (auto it : TFEM::params.plist)
        if (it.getType() == ParamType::InitialCondition)
        {
            parser.set_expression(it.getExpression());
            value = parser.run();
            direct = unsigned(it.getDirect());

            for (unsigned j = 0; j < TFEM::mesh->getNumVertex(); j++)
            {
                if ((direct & FUN_U) == FUN_U)
                    u0[0][j] = value;
                if ((direct & FUN_V) == FUN_V)
                    u0[1][j] = value;
                if ((direct & FUN_W) == FUN_W)
                    u0[2][j] = value;
                if ((direct & FUN_UT) == FUN_UT)
                    u0[3][j] = value;
                if ((direct & FUN_VT) == FUN_VT)
                    u0[4][j] = value;
                if ((direct & FUN_WT) == FUN_WT)
                    u0[5][j] = value;
                if ((direct & FUN_UTT) == FUN_UTT)
                    u0[6][j] = value;
                if ((direct & FUN_VTT) == FUN_VTT)
                    u0[7][j] = value;
                if ((direct & FUN_WTT) == FUN_WTT)
                    u0[8][j] = value;
            }
        }
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMDynamic<T>::genResults(vector<double>& result,bool isAdd)
{
    matrix<double> newResult;

    // Вычисляем стандартные результаты по всем КЭ
    TFEMStatic<T>::calcResult(newResult,result);

    // Вычисляем скорости и ускорения
    calcDynamicResult(newResult);

    // Сохраняем их
    saveResult(newResult,isAdd);

    // Запоминаем дату и время расчета
    TFEM::results->setCurrentSolutionTime();
}
//---------------------------------------------------------
template<class T> void TFEMDynamic<T>::saveResult(matrix<double>& newResult,bool)
{
    stringstream s;

    // Cохраняем результаты
    for (unsigned i = 0; i < TFEM::params.numResult(TFEM::mesh->getTypeFE()); i++)
    {
        s << t;
        TFEM::results->setResult(newResult[i],newResult.size2(),TFEM::params.getName(i, TFEM::mesh->getTypeFE()) + '(' + s.str() + ')',t);
        s.str("");
        s.clear();
    }
}
//---------------------------------------------------------
//    Вычисление скоростей и ускорений (Ut,...,Utt,..)
//---------------------------------------------------------
template<class T> void TFEMDynamic<T>::calcDynamicResult(matrix<double>& results)
{
    double ut,
           utt;

    // Вычисляем скорости и ускорения (методом конечных разностей)
    for (unsigned i = 0; i < TFEM::mesh->getFreedom(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getNumVertex(); j++)
        {
            ut = (results[i][j] - u0[i][j]) / TFEM::params.th;
            utt = (ut - u0[TFEM::mesh->getFreedom() + i][j])/TFEM::params.th;

            results[results.size1() - 2 * TFEM::mesh->getFreedom() + i][j] = ut;
            results[results.size1() - TFEM::mesh->getFreedom() + i][j] = utt;
            // Запоминаем результаты текущей итерации
            u0[i][j] = results[i][j];
            u0[TFEM::mesh->getFreedom() + i][j] = results[results.size1() - 2 * TFEM::mesh->getFreedom() + i][j];
            u0[2*TFEM::mesh->getFreedom() + i][j] = results[results.size1() - TFEM::mesh->getFreedom() + i][j];
        }
}
//-----------------------------------------------------------------------------
// Формирование "динамической" правой части СЛАУ для текущего момента времени
//-----------------------------------------------------------------------------
template<class T> void TFEMDynamic<T>::createDynamicVector(void)
{
    unsigned nvtx = TFEM::mesh->getNumVertex(),
             freedom = TFEM::mesh->getFreedom(),
             size = nvtx * freedom;
    double k1 = 3.0 / (TFEM::params.theta * TFEM::params.th),
           k2 = 6.0 / (TFEM::params.theta * TFEM::params.theta * TFEM::params.th * TFEM::params.th),
           k3 = 0.5 * TFEM::params.theta * TFEM::params.th;
    vector<double> load(size),
                   u1(size),
                   u2(size),
                   r1(size),
                   r2(size);

    // Вычисление вектора нагрузок для текущего момента времени
    TFEMStatic<T>::calcLoad(load, t);

    // Получаем значения U, Ut и Utt предыдущей итерации (или из начальных условий)
    for (unsigned i = 0; i < nvtx; i++)
        for (unsigned j = 0; j < freedom; j++)
        {
            u1[i * freedom + j] = (k1 * u0[j][i] + 2.0 * k2 * u0[u0.size1() - 2 * freedom + j][i] + 2.0 * u0[u0.size1() - freedom + j][i]) / k2;
            u2[i * freedom + j] = (k2 * u0[j][i] + 2.0 * u0[u0.size1() - 2 * freedom + j][i] + k3 * u0[u0.size1() - freedom + j][i]) / k1;
        }
    TFEMStatic<T>::solver.product(TFEMStatic<T>::solver.getMassMatrix(), u1, r1);
    TFEMStatic<T>::solver.product(TFEMStatic<T>::solver.getDampingMatrix(), u2, r2);
//    TFEMStatic<T>::solver.product(TFEMStatic<T>::solver.getMM(), u1, r1);
//    TFEMStatic<T>::solver.product(TFEMStatic<T>::solver.getDM(), u2, r2);

    // Формирование столбца правой части с учетом "динамической" составляющей
    for (unsigned i = 0; i < size; i++)
        load[i] += (r1[i] + r2[i]);

    TFEMStatic<T>::setLoad(load);
}
//--------------------------------------------------------------------------
// Формирование левой части СЛАУ в динамике (согласно методу Тета-Вильсона)
//--------------------------------------------------------------------------
template<class T> void TFEMDynamic<T>::createDynamicMatrix(double th, double theta)
{
    double val,
           k1 = 3.0 / (theta * th),
           k2 = 6.0 / (theta * theta * th * th);

    for (unsigned i = 0; i < TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom(); j++)
        {
            val = k1 * TFEMStatic<T>::solver.getDamping(i, j) + k2 * TFEMStatic<T>::solver.getMass(i, j);
            if (val not_eq 0.0)
                TFEMStatic<T>::solver.addStiffness(val, i, j);
        }
}
//--------------------------------------------------------------------------


#endif // FEMDYNAMIC_H

