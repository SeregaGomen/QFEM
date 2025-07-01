#ifndef FEMDYNAMIC_H
#define FEMDYNAMIC_H

#include "parser/parser.h"
#include "femstatic.h"

extern TMessenger* msg;

//----------------------------------------------------------
//  Реализация конечно-элементного расчета в соответствии с
//  вариационным принципом Гамильтона-Остроградского
//  (способ интегрирования - метод Ньюмарка)
//----------------------------------------------------------
template <typename SOLVER, typename FE> class TFEMDynamic : public TFEMStatic<SOLVER, FE>
{
private:
    double t; // Текущий момент времени
    matrix<double> u0;  // Начальные условия и результаты U, V, W, Ut,... по итерациям
    void createDynamicMatrix();
    void createDynamicVector(matrix<double>&, double, double);
protected:
    void ansambleLocalMatrix(TFE&, unsigned) override;
    void getInitialCondition(void);
    void setBoundaryConstant(unsigned, unsigned, double);
    void setLoadConstant(unsigned, unsigned, double);
    void calcDynamicResult(matrix<double>&);
    void createDynamicVector(void);
    void genResults(vector<double>&, bool = false);
    void saveResult(matrix<double>&, bool);
public:
    TFEMDynamic(string n, TMesh* m, TResults* r, list<string>* l) : TFEMStatic<SOLVER, FE>(n, m, r, l)
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
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::startProcess(void)
{
    unsigned hour,
        min,
        sec;
    vector<double> result;
    ostringstream out;

    TFEM::begin();
    t = TFEMStatic<SOLVER, FE>::params.t0 + TFEMStatic<SOLVER, FE>::params.th;
    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;
    TFEMStatic<SOLVER, FE>::solver.setMatrix(TFEM::mesh, true);

    // Учет граничных условий
    TFEMStatic<SOLVER, FE>::calcBoundaryCondition();

    // Формирование глобальных матриц жесткости, масс и демпфирования
    TFEMStatic<SOLVER, FE>::calcGlobalMatrix(false);

    // Формирование "статической" левой части СЛАУ и сохранение ее для последующего использования
    createDynamicMatrix();

    // Учет начальных условий
    getInitialCondition();

    // Итерационный процесс по времени
    cout << S_MSG_TIME_ITERATION << endl;

    //    TFEMStatic<SOLVER, SHAPE, FE>::solver.saveStiffnessMatrix("matrix.dat");

    while (t <= TFEM::params.t1)
    {
        cout << TFEM::params.names[3] << '=' << t << endl;
        // Формирование динамической правой части СЛАУ
        createDynamicVector();
        if (TFEMStatic<SOLVER, FE>::solver.solution(result, TFEM::params.eps, TFEM::isProcessAborted))
            genResults(result); // Вычисление результатов
        if (TFEM::isProcessAborted)
            throw ErrorCode::EAbort;
        TFEM::printResultSummary(t);
        if (fabs(TFEM::params.t1 - (t + TFEM::params.th)) <= TFEM::params.eps)
            t = TFEM::params.t1;
        else
            t += TFEM::params.th;

        //        TFEMStatic<SOLVER, SHAPE, FE>::solver.loadStiffnessMatrix("matrix.dat");
    }

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
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::ansambleLocalMatrix(TFE &fe, unsigned i)
{
    unsigned freedom = fe.getFreedom(),
        size = fe.getSize()*fe.getFreedom();

    // Учет матрицы
    for (unsigned l = 0; l < size; l++)
    {
        for (unsigned k = l; k < size; k++)
        {
            TFEMStatic<SOLVER, FE>::solver.addStiffness(fe.getStiffnessMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k%freedom);
            TFEMStatic<SOLVER, FE>::solver.addMass(fe.getMassMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k%freedom);
            TFEMStatic<SOLVER, FE>::solver.addDamping(fe.getDampingMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k%freedom);
            if (l != k)
            {
                TFEMStatic<SOLVER, FE>::solver.addStiffness(fe.getStiffnessMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k%freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom);
                TFEMStatic<SOLVER, FE>::solver.addMass(fe.getMassMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k % freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom);
                TFEMStatic<SOLVER, FE>::solver.addDamping(fe.getDampingMatrix(l, k), TFEMStatic<SOLVER, FE>::mesh->getFE(i, k/freedom)*freedom + k%freedom, TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l%freedom);
            }
        }
        TFEMStatic<SOLVER, FE>::solver.addLoad(fe.getLoad(l), TFEMStatic<SOLVER, FE>::mesh->getFE(i, l/freedom)*freedom + l % freedom);
    }
}
//---------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::getInitialCondition(void)
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
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::genResults(vector<double> &result, bool isAdd)
{
    matrix<double> newResult;

    // Вычисляем стандартные результаты по всем КЭ
    TFEMStatic<SOLVER, FE>::calcResult(newResult, result);

    // Вычисляем скорости и ускорения
    calcDynamicResult(newResult);

    // Сохраняем их
    saveResult(newResult, isAdd);

    // Запоминаем дату и время расчета
    TFEM::results->setCurrentSolutionTime();
}
//---------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::saveResult(matrix<double> &newResult, bool)
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
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::calcDynamicResult(matrix<double>& results)
{
    double ut,
        utt;

    // Вычисляем скорости и ускорения (методом конечных разностей)
    for (unsigned i = 0; i < TFEM::mesh->getDimension(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getNumVertex(); j++)
        {
            ut = (results[i][j] - u0[i][j])/TFEM::params.th;
            utt = (ut - u0[TFEM::mesh->getDimension() + i][j])/TFEM::params.th;

            results[results.size1() - 2*TFEM::mesh->getDimension() + i][j] = ut;
            results[results.size1() - TFEM::mesh->getDimension() + i][j] = utt;
            // Запоминаем результаты текущей итерации
            u0[i][j] = results[i][j];
            u0[TFEM::mesh->getDimension() + i][j] = results[results.size1() - 2*TFEM::mesh->getDimension() + i][j];
            u0[2*TFEM::mesh->getDimension() + i][j] = results[results.size1() - TFEM::mesh->getDimension() + i][j];
        }
}
//-----------------------------------------------------------------------------
// Формирование "динамической" правой части СЛАУ для текущего момента времени
//-----------------------------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::createDynamicVector(void)
{
    auto nvtx = TFEM::mesh->getNumVertex(),
        freedom = TFEM::mesh->getFreedom(),
        dim = TFEM::mesh->getDimension(),
        size = nvtx*freedom;
    double a1 = TFEM::params.gamma/(TFEM::params.beta*TFEM::params.th),
           a2 = 1.0/(TFEM::params.beta*TFEM::params.th*TFEM::params.th);
    vector<double> load(size),
        am(size),
        ac(size),
        r(size);

    // Вычисление вектора нагрузок для следующего момента времени
    TFEMStatic<SOLVER, FE>::calcLoad(load, t + TFEM::params.th);

    // Получаем значения U, Ut и Utt предыдущей итерации (или из начальных условий)
    msg->setProcess(ProcessCode::GeneratingDynamicLoad, 1, nvtx + size, 5);
    for (unsigned i = 0; i < nvtx; i++)
    {
        msg->addProgress();
        if (TFEM::isProcessAborted)
            throw ErrorCode::EAbort;
        for (unsigned j = 0; j < freedom; j++)
        {
            am[i*freedom + j] = a2*(u0[j][i] + TFEM::params.th*u0[u0.size1() - 2*dim + j][i] + 0.5*TFEM::params.th*TFEM::params.th*(1.0 - 2.0*TFEM::params.beta)*u0[u0.size1() - dim + j][i]);
            ac[i*freedom + j] = a1*(u0[j][i] + TFEM::params.th*u0[u0.size1() - 2*dim + j][i] + 0.5*TFEM::params.th*TFEM::params.th*(1.0 - 2.0*TFEM::params.beta)*u0[u0.size1() - dim + j][i]);
        }
    }

    for (auto i = 0u; i < size; i++)
    {
        msg->addProgress();
        if (TFEM::isProcessAborted)
            throw ErrorCode::EAbort;
        for (auto j = 0u; j < size; j++)
            r[i] += TFEMStatic<SOLVER, FE>::solver.getMass(i, j)*am[j] + TFEMStatic<SOLVER, FE>::solver.getDamping(i, j)*ac[j];
    }
    msg->stopProcess();

    // Формирование столбца правой части с учетом "динамической" составляющей
    for (unsigned i = 0; i < size; i++)
        load[i] += r[i];

    TFEMStatic<SOLVER, FE>::setLoad(load);
}

//--------------------------------------------------------------------------
// Формирование левой части СЛАУ в динамике (согласно методу Тета-Вильсона)
//--------------------------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMDynamic<SOLVER, FE>::createDynamicMatrix(void)
{
    double val,
           a1 = TFEM::params.gamma/(TFEM::params.beta*TFEM::params.th),
           a2 = 1.0/(TFEM::params.beta*TFEM::params.th*TFEM::params.th);

    msg->setProcess(ProcessCode::GeneratingDynamicMatrix, 1, TFEM::mesh->getNumVertex()*TFEM::mesh->getFreedom(), 5);
    for (unsigned i = 0; i < TFEM::mesh->getNumVertex()*TFEM::mesh->getFreedom(); i++)
    {
        msg->addProgress();
        if (TFEM::isProcessAborted)
            throw ErrorCode::EAbort;
        for (unsigned j = i; j < TFEM::mesh->getNumVertex()*TFEM::mesh->getFreedom(); j++)
        {
            val = a1*TFEMStatic<SOLVER, FE>::solver.getDamping(i, j) + a2*TFEMStatic<SOLVER, FE>::solver.getMass(i, j);
            if (val != 0.0)
            {
                TFEMStatic<SOLVER, FE>::solver.addStiffness(val, i, j);
                if (i != j)
                    TFEMStatic<SOLVER, FE>::solver.addStiffness(val, j, i);
            }
        }
    }
    msg->stopProcess();
}
//--------------------------------------------------------------------------


#endif // FEMDYNAMIC_H
