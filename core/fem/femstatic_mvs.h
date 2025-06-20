#ifndef FEMSTATIC_MVS_H
#define FEMSTATIC_MVS_H

#include <ctime>
#include <iomanip>
#include <cmath>
#include "femstatic.h"

extern TMessenger* msg;

//----------------------------------------------------------------------------
//          Реализация конечно-элементного расчета в соответствии с
//          методом упругих решений (статика)
//----------------------------------------------------------------------------
template <typename SOLVER, typename FE> class TFEMStaticMVS : public TFEMStatic<SOLVER, FE>
{
private:
    int iterNo;                                                  // Номер итерации (0 - признак того, что расчет линейный)
    double loadStep;                                             // Шаг по нагрузке
    bool isStopGlobalIteration;                                  // Признак того, что расчет окончен (достигнут предел текучести)
    bool isStopLocalIteration;                                   // Признак того, что итерационный процесс для заданной нагрузки окончен
    vector<double> si;                                           // Интенсивность напряжений (по узлам)
    vector<double> e0;                                           // Модули упругости (для каждого КЭ), полученные на предыдущей итерации
    vector<unsigned> index0;                                     // Индекс упругих свойств, полученный на предыдущей итерации
    virtual void setupFE(TFE&, unsigned) override;   // Настройка парметров КЭ (для нелинейного случая)
    void calcIteration(void);
public:
    TFEMStaticMVS(double &step, string n, TMesh *m, TResults *r, list<string> *l) : TFEMStatic<SOLVER, FE>(n, m, r, l)
    {
        loadStep = step;
        iterNo = 0;
        isStopGlobalIteration = isStopLocalIteration = false;
    }
    virtual ~TFEMStaticMVS(void) {}
    virtual void startProcess(void) override;
};
//----------------------------------------------------------------------------
//                                Запуск расчета
//----------------------------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMStaticMVS<SOLVER, FE>::startProcess(void)

{
    double maxSi,
           maxSsc,
           coef = 1,
           loadFactor,
           addCount = 0,
           step = loadStep * 0.01;
    unsigned hour,
             min,
             sec,
             count = 1;
    vector<double> result,
                   load(TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom());
    bool isLoaded = false;
    ostringstream out;

    cout << S_NUM_THREAD << TFEMStatic<SOLVER, FE>::numThread << endl;

    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;

    TFEM::begin();
    // Предварительное вычисление компонент нагрузки
    TFEMStatic<SOLVER, FE>::calcLoad(load);

    maxSsc = TFEM::params.getMinStress();

    // Итерационный процесс линеаризации упруго-пластической задачи
    e0.resize(TFEM::mesh->getNumFE());
    si.resize(TFEM::mesh->getNumVertex(), 0);
    index0.resize(TFEM::mesh->getNumFE(), 0);

    do
    {
        isStopLocalIteration = true;
        TFEMStatic<SOLVER, FE>::solver.setMatrix(TFEM::mesh);
        // Формирование ГМЖ
        TFEMStatic<SOLVER, FE>::calcGlobalMatrix();
        // Учет нагрузки
        TFEMStatic<SOLVER, FE>::setLoad(load);
        // Учет краевых условий
        TFEMStatic<SOLVER, FE>::calcBoundaryCondition();
        // Решение СЛАУ
        if (!TFEMStatic<SOLVER, FE>::solver.solution(result, TFEM::params.eps, TFEM::isProcessAborted))
        {
            TFEM::isProcessCalculated = false;
            if (TFEM::isProcessAborted)
                throw ErrorCode::EAbort;
            return;
        }
        if (iterNo == 0)
        {
            // Вычисление дополнительных результатов
            TFEMStatic<SOLVER, FE>::genResults(result);
            // Вычисление интенсивности напряжений
            if ((maxSi = TFEMStatic<SOLVER, FE>::calcStressIntensity(*TFEM::results, si)) > maxSsc)
            {
                // Задана слишком большая первоначальная нагрузка, уменьшаем ее на порядок
                coef *= 0.1;
                for_each(load.begin(), load.end(), [](double& i) -> double{ return i *= 0.1; });
                iterNo--;
            }
            else
            {
                if (not isLoaded)
                {
                    // Вычисляем поправочный коэффициент для "пропуска" упругой зоны
                    coef *= (loadFactor = 0.95 * (maxSsc / maxSi));
                    for_each(load.begin(), load.end(), [loadFactor](double& i) -> double{ return i *= loadFactor; });
                    isLoaded = true;
                    iterNo--;
                }
                else // Устанавливаем нагрузку в значение "шаг по нагрузке"
                    for_each(load.begin(), load.end(), [step](double& i) -> double{ return i *= step; });
            }
        }
        else
            if (isStopLocalIteration)
            {
                TFEMStatic<SOLVER, FE>::genResults(result, true);
                maxSi = TFEMStatic<SOLVER, FE>::calcStressIntensity(*TFEM::results, si);
                TFEM::results->setResult(si, "Si");
            }

        // Вывод рез-тов по каждой функции на экран
        TFEM::printResultSummary();
        // Вывод информации об итерации
        cout << S_MSG_LOAD << " x " << (coef * (1 + addCount * step)) << endl;
        cout << S_MSG_SI << maxSi << endl;
        cout << S_MSG_ITERATION << count++ << endl;
        cout.unsetf(ios::scientific);

        if (++iterNo > 0 and isStopLocalIteration)
        {
            addCount += 1;
            isStopLocalIteration = false;
        }
        //TFEMStatic<SOLVER, FE>::solver.clear();
    }
    while (not isStopGlobalIteration);

    TFEM::isProcessStarted = false;
    TFEM::isProcessCalculated = true;

    TFEM::end(hour, min, sec);
    // Выводим и сохраняем информацию об итерационном процессе
    out << S_MSG_LOAD << " x " << (coef * (1 + addCount * step)) << endl;
    if (TFEM::notes)
        TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_SI << maxSi << endl;
    if (TFEM::notes)
        TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_ITERATION << --count << endl;
    if (TFEM::notes)
        TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ') << endl;
    if (TFEM::notes)
        TFEM::notes->push_back(out.str());
    cout << out.str() << endl;
    // Вывод коэффициента изменения нагрузки P = P0 * k1 * (1 + k2 * n), где:
    // k1 - коэффициент пропуска упругой зоны (возврата в упругую зону);
    // k2 - коэффициент увеличения нагрузки (loadStep / 100);
    // n - количество итераций по приращению нагрузки.
//    cout << "P = P0 * " << (coef * (1 + addCount * step)) << endl;
}
//----------------------------------------------------------------------------
//                      Настройка упругих парметров КЭ
//----------------------------------------------------------------------------
template <typename SOLVER, typename FE> void TFEMStaticMVS<SOLVER, FE>::setupFE(TFE &fe, unsigned i)
{
    double newE,
           feSi = si[TFEM::mesh->getFE(i, 0)];
    unsigned index;
    matrix<double> x,
                   ssCurve;

    TFEM::setupFE(fe, i);
    // -------------- Линейный (упругий) случай ------------------
    if (iterNo == 0)
        return;

    // Загружаем диаграмму деформирования, соответствующую текущему КЭ
    TFEM::mesh->getCoordFE(i, x);
    TFEM::params.getStressStrainCurve(x, ssCurve);
    if (ssCurve.size1() == 0)
        throw ErrorCode::EStressStrainCurve;

    // Определяем среднюю по КЭ интенсивность наряжений
    for (unsigned j = 1; j < TFEM::mesh->getSizeFE(); j++)
        if (feSi < si[TFEM::mesh->getFE(i, j)])
            feSi = si[TFEM::mesh->getFE(i, j)];

    // ------------- Нелинейный случай ----------------
    // Поиск в таблице свойств материала соответствующего напряжения
    if (feSi < ssCurve[1][0])
        index = 0;
    else
        for (index = 1; index < ssCurve.size1(); index++)
            if (feSi > ssCurve[index - 1][0] and feSi <= ssCurve[index][0])
                break;

    if (index == ssCurve.size1())
    {
        // Достигнут предел текучести
        index--;
        isStopGlobalIteration = true;
    }

    if (index != index0[i])
        //newE = fabs((ssCurve[index][0] - ssCurve[index0[i]][0]) / (ssCurve[index][1] - ssCurve[index0[i]][1]));
        newE = fabs(ssCurve[index][0] / ssCurve[index][1] - ssCurve[index0[i]][0] / ssCurve[index0[i]][1]);
    else
        newE = (e0[i] == 0.0) ? fe.getYoungModulus() : e0[i];

    fe.setYoungModulus(newE);
    fe.setPoissonRatio(TFEM::params.getPoissonRatio(x));

    // Проверка на изменение модуля упругости по сравнению с предыдущей итерацией
    if (index != index0[i])
        isStopLocalIteration = false;

    // Запоминаем рассчитанное значение модуля упругости и индекс
    e0[i] = newE;
    index0[i] = index;
}
//----------------------------------------------------------------------------

#endif // FEMSTATIC_MVS_H
