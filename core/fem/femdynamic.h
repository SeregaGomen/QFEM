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
template <typename SOLVER, typename SHAPE, template<typename> typename FE> class TFEMDynamic : public TFEMStatic<SOLVER, SHAPE, FE>
{
private:
    double t; // Текущий момент времени
    matrix<double> u0;  // Начальные условия и результаты U, V, W, Ut,... по итерациям
    void createDynamicMatrix(double, double);
    void createDynamicVector(matrix<double>&, double, double);
protected:
    void ansambleLocalMatrix(TFE*, unsigned);
    void getInitialCondition(void);
    void setBoundaryConstant(unsigned, unsigned, double);
    void setLoadConstant(unsigned, unsigned, double);
    void calcDynamicResult(matrix<double>&);
    void createDynamicVector(void);
    void genResults(vector<double>&, bool = false);
    void saveResult(matrix<double>&, bool);
public:
    TFEMDynamic(string n, TMesh* m, TResults* r, list<string>* l) : TFEMStatic<SOLVER, SHAPE, FE>(n, m, r, l)
    {
        t = 0;
        TFEM::params.fType = FEMType::DynamicProblem;
    }
    virtual ~TFEMDynamic(void) {}
    virtual void startProcess(void) override;
};
//----------------------------------------------------------
//                     Запуск расчета
//----------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::startProcess(void)
{
    unsigned hour,
             min,
             sec;
    vector<double> result;
    ostringstream out;

    TFEM::begin();
    t = TFEMStatic<SOLVER, SHAPE, FE>::params.t0 + TFEMStatic<SOLVER, SHAPE, FE>::params.th;
    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;
    TFEMStatic<SOLVER, SHAPE, FE>::solver.setMatrix(TFEM::mesh, true);

    // Формирование глобальных матриц жесткости, масс и демпфирования
    TFEMStatic<SOLVER, SHAPE, FE>::calcGlobalMatrix(false);

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
        TFEMStatic<SOLVER, SHAPE, FE>::calcBoundaryCondition();
        if (TFEMStatic<SOLVER, SHAPE, FE>::solver.solve(result, TFEM::params.eps, TFEM::isProcessAborted))
            genResults(result); // Вычисление результатов
        if (TFEM::isProcessAborted)
            throw ErrorCode::EAbort;
        TFEM::printResultSummary(t);
        if (fabs(TFEM::params.t1 - (t + TFEM::params.th)) <= TFEM::params.eps)
            t = TFEM::params.t1;
        else
            t += TFEM::params.th;
    }

    TFEMStatic<SOLVER, SHAPE, FE>::solver.clear();
    TFEM::isProcessStarted = false;
    TFEM::isProcessCalculated = true;

    TFEM::end(hour, min, sec);
    // Сохраняем информацию о времени расчета
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ');
    if (TFEM::notes)
        TFEM::notes->push_back(out.str());
    cout << out.str() << endl;
}
//-------------------------------------------------------------
//           Ансамблирование ЛМЖ, ЛММ и ЛМД к ГМЖ
//-------------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::ansambleLocalMatrix(TFE* fe, unsigned i)
{
    unsigned freedom = fe->getFreedom(),
             size = fe->getSize() * fe->getFreedom();

    // Ансамблирование матрицы
    for (unsigned l = 0; l < size; l++)
    {
        for (unsigned k = l; k < size; k++)
        {
            TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixStiffnessElement(fe->getStiffnessMatrix(l, k), TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixMassElement(fe->getMassMatrix(l, k), TFEM::mesh->getFE(i, l / freedom)*freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixDampingElement(fe->getDampingMatrix(l, k), TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom, TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom);
            if (l not_eq k)
            {
                TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixStiffnessElement(fe->getStiffnessMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
                TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixMassElement(fe->getMassMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
                TFEMStatic<SOLVER, SHAPE, FE>::solver.addMatrixDampingElement(fe->getDampingMatrix(l, k), TFEM::mesh->getFE(i, k / freedom) * freedom + k % freedom, TFEM::mesh->getFE(i, l / freedom) * freedom + l % freedom);
            }
        }
        TFEMStatic<SOLVER, SHAPE, FE>::solver.addLoadElement(fe->getLoad(l), l);
    }
}
//---------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::getInitialCondition(void)
{
    TParser parser;
    InitialCondition init;
    double value;

    u0.resize(9, TFEM::mesh->getNumVertex());
    for (auto it : TFEM::params.plist)
        if (it.getType() == ParamType::InitialCondition)
        {
            parser.set_expression(it.getExpression());
            value = parser.run();
            init = it.getInitialCondition();

            for (unsigned j = 0; j < TFEM::mesh->getNumVertex(); j++)
            {
                if (contains(init, InitialCondition::U))
                    u0[0][j] = value;
                if (contains(init, InitialCondition::V))
                    u0[1][j] = value;
                if (contains(init, InitialCondition::W))
                    u0[2][j] = value;
                if (contains(init, InitialCondition::Ut))
                    u0[3][j] = value;
                if (contains(init, InitialCondition::Vt))
                    u0[4][j] = value;
                if (contains(init, InitialCondition::Wt))
                    u0[5][j] = value;
                if (contains(init, InitialCondition::Utt))
                    u0[6][j] = value;
                if (contains(init, InitialCondition::Vtt))
                    u0[7][j] = value;
                if (contains(init, InitialCondition::Wtt))
                    u0[8][j] = value;
            }
        }
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::genResults(vector<double>& result, bool isAdd)
{
    matrix<double> newResult;

    // Вычисляем стандартные результаты по всем КЭ
    TFEMStatic<SOLVER, SHAPE, FE>::calcResult(newResult, result);

    // Вычисляем скорости и ускорения
    calcDynamicResult(newResult);

    // Сохраняем их
    saveResult(newResult, isAdd);

    // Запоминаем дату и время расчета
    TFEM::results->setCurrentSolutionTime();
}
//---------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::saveResult(matrix<double>& newResult, bool)
{
    stringstream s;

    // Cохраняем результаты
    for (unsigned i = 0; i < TFEM::params.numResult(TFEM::mesh->getTypeFE()); i++)
    {
        s << t;
        TFEM::results->setResult(newResult[i], newResult.size2(), TFEM::params.getName(i, TFEM::mesh->getTypeFE()) + '(' + s.str() + ')', t);
        s.str("");
        s.clear();
    }
}
//---------------------------------------------------------
//    Вычисление скоростей и ускорений (Ut,...,Utt,..)
//---------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::calcDynamicResult(matrix<double>& results)
{
    double ut,
           utt;

    // Вычисляем скорости и ускорения (методом конечных разностей)
    for (unsigned i = 0; i < TFEM::mesh->getDimension(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getNumVertex(); j++)
        {
            ut = (results[i][j] - u0[i][j]) / TFEM::params.th;
            utt = (ut - u0[TFEM::mesh->getDimension() + i][j])/TFEM::params.th;

            results[results.size1() - 2 * TFEM::mesh->getDimension() + i][j] = ut;
            results[results.size1() - TFEM::mesh->getDimension() + i][j] = utt;
            // Запоминаем результаты текущей итерации
            u0[i][j] = results[i][j];
            u0[TFEM::mesh->getDimension() + i][j] = results[results.size1() - 2 * TFEM::mesh->getDimension() + i][j];
            u0[2 * TFEM::mesh->getDimension() + i][j] = results[results.size1() - TFEM::mesh->getDimension() + i][j];
        }
}
//-----------------------------------------------------------------------------
// Формирование "динамической" правой части СЛАУ для текущего момента времени
//-----------------------------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::createDynamicVector(void)
{
    unsigned nvtx = TFEM::mesh->getNumVertex(),
             freedom = TFEM::mesh->getFreedom(),
             dim = TFEM::mesh->getDimension(),
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
    TFEMStatic<SOLVER, SHAPE, FE>::calcLoad(load, t);

    // Получаем значения U, Ut и Utt предыдущей итерации (или из начальных условий)
    for (unsigned i = 0; i < nvtx; i++)
        for (unsigned j = 0; j < freedom; j++)
        {
            u1[i * freedom + j] = (k1 * u0[j][i] + 2.0 * k2 * u0[u0.size1() - 2 * dim + j][i] + 2.0 * u0[u0.size1() - dim + j][i]) / k2;
            u2[i * freedom + j] = (k2 * u0[j][i] + 2.0 * u0[u0.size1() - 2 * dim + j][i] + k3 * u0[u0.size1() - dim + j][i]) / k1;
        }
    TFEMStatic<SOLVER, SHAPE, FE>::solver.product(TFEMStatic<SOLVER, SHAPE, FE>::solver.getMassMatrix(), u1, r1);
    TFEMStatic<SOLVER, SHAPE, FE>::solver.product(TFEMStatic<SOLVER, SHAPE, FE>::solver.getDampingMatrix(), u2, r2);
//    TFEMStatic<SOLVER, SHAPE, FE>::solver.product(TFEMStatic<SOLVER, SHAPE, FE>::solver.getMM(), u1, r1);
//    TFEMStatic<SOLVER, SHAPE, FE>::solver.product(TFEMStatic<SOLVER, SHAPE, FE>::solver.getDM(), u2, r2);

    // Формирование столбца правой части с учетом "динамической" составляющей
    for (unsigned i = 0; i < size; i++)
        load[i] += (r1[i] + r2[i]);

    TFEMStatic<SOLVER, SHAPE, FE>::setLoad(load);
}
//--------------------------------------------------------------------------
// Формирование левой части СЛАУ в динамике (согласно методу Тета-Вильсона)
//--------------------------------------------------------------------------
template <typename SOLVER, typename SHAPE, template<typename> typename FE> void TFEMDynamic<SOLVER, SHAPE, FE>::createDynamicMatrix(double th, double theta)
{
    double val,
           k1 = 3.0 / (theta * th),
           k2 = 6.0 / (theta * theta * th * th);

    for (unsigned i = 0; i < TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom(); j++)
        {
            val = k1 * TFEMStatic<SOLVER, SHAPE, FE>::solver.getDamping(i, j) + k2 * TFEMStatic<SOLVER, SHAPE, FE>::solver.getMass(i, j);
            if (val not_eq 0.0)
                TFEMStatic<SOLVER, SHAPE, FE>::solver.addStiffness(val, i, j);
        }
}
//--------------------------------------------------------------------------


#endif // FEMDYNAMIC_H

