#ifndef FEMSTATIC_MVS_H
#define FEMSTATIC_MVS_H

#include <time.h>
#include <iomanip>
#include <cmath>
#include "femstatic.h"

extern TMessenger* msg;

//----------------------------------------------------------------------------
//          Реализация конечно-элементного расчета в соответствии с
//          методом упругих решений (статика)
//----------------------------------------------------------------------------
template <class T> class TFEMStaticMVS : public TFEMStatic<T>
{
private:
    int iterNo;                             // Номер итерации (0 - признак того, что расчет линейный)
    double loadStep;                        // Шаг по нагрузке
    bool isStopGlobalIteration;             // Признак того, что расчет окончен (достигнут предел текучести)
    bool isStopLocalIteration;              // Признак того, что итерационный процесс для заданной нагрузки окончен
    vector<double> si;                      // Интенсивность напряжений (по узлам)
    vector<double> e0;                      // Модули упругости (для каждого КЭ), полученные на предыдущей итерации
    vector<unsigned> index0;                // Индекс упругих свойств, полученный на предыдущей итерации
    virtual void setupFE(TFE*, unsigned);   // Настройка парметров КЭ (для нелинейного случая)
    void calcIteration(void);
public:
    TFEMStaticMVS(double& step, string n, TMesh* m, TResultList* r, list<string>* l) : TFEMStatic<T>(n, m, r, l)
    {
        loadStep = step;
        iterNo = 0;
        isStopGlobalIteration = isStopLocalIteration = false;
    }
    virtual ~TFEMStaticMVS(void) {}
    void startProcess(void);
};
//----------------------------------------------------------------------------
//                                Запуск расчета
//----------------------------------------------------------------------------
template<class T> void TFEMStaticMVS<T>::startProcess(void)

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
    time_t full_timer;
    bool isLoaded = false;
    ostringstream out;

    cout << S_NUM_THREAD << TFEMStatic<T>::numThread << endl;

    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;

    full_timer = clock();
    // Предварительное вычисление компонент нагрузки
    TFEMStatic<T>::calcLoad(load);

    maxSsc = TFEM::params.getMinStress();

    // Итерационный процесс линеаризации упруго-пластической задачи
    e0.resize(TFEM::mesh->getNumFE());
    si.resize(TFEM::mesh->getNumVertex(), 0);
    index0.resize(TFEM::mesh->getNumFE(), 0);
    do
    {
        isStopLocalIteration = true;
        TFEMStatic<T>::solver.setMatrix(TFEM::mesh);
        // Формирование ГМЖ
        TFEMStatic<T>::calcGlobalMatrix();

        // Учет нагрузки
        TFEMStatic<T>::setLoad(load);

        // Учет краевых условий
        TFEMStatic<T>::calcBoundaryCondition();

        // Решение СЛАУ
        if (TFEMStatic<T>::solver.solve(result, TFEM::params.eps, TFEM::isProcessAborted))
        {
            // Вычисление дополнительных результатов
            if (iterNo == 0)
            {
                TFEMStatic<T>::genResults(result);
                // Вычисление интенсивности напряжений
                maxSi = TFEMStatic<T>::calcStressIntensity(*TFEM::results, si);
            }
            else
                if (isStopLocalIteration)
                {
                    TFEMStatic<T>::genResults(result, true);
                    maxSi = TFEMStatic<T>::calcStressIntensity(*TFEM::results, si);
                    TFEM::results->setResult(si, "Si");
                }

            // Вывод рез-тов по каждой функции на экран
            TFEM::printResultSummary();
            // Вывод информации об итерации
            cout << S_MSG_LOAD << " x " << (coef * (1 + addCount * step)) << endl;
            cout << S_MSG_SI << maxSi << endl;
            cout << S_MSG_ITERATION << count++ << endl;
            cout.unsetf(ios::scientific);

            if (iterNo == 0)
            {
                if (maxSi > maxSsc)
                {
                    // Задана слишком большая первоначальная нагрузка, уменьшаем ее на порядок
                    coef *= 0.1;
                    for_each(load.begin(), load.end(), [](double& i) -> double{ return i *= 0.1; });
                    iterNo--;
                }
                else
                {
                    if (!isLoaded)
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
            if (++iterNo > 0 && isStopLocalIteration)
            {
                addCount += 1;
                isStopLocalIteration = false;
            }
        }
        else
        {
            TFEM::isProcessCalculated = false;
            TFEMStatic<T>::solver.clear();
            if (TFEM::isProcessAborted)
                throw ABORT_ERR;
            return;
        }
        TFEMStatic<T>::solver.clear();
    }
    while (!isStopGlobalIteration);

    TFEM::isProcessStarted = false;
    TFEM::isProcessCalculated = true;

    full_timer = clock() - full_timer;
    hour = unsigned(full_timer / CLOCKS_PER_SEC) / 3600;
    min = unsigned((full_timer / CLOCKS_PER_SEC) % 3600) / 60;
    sec = unsigned(full_timer / CLOCKS_PER_SEC) - hour * 3600 - min * 60;

    // Выводим и сохраняем информацию об итерационном процессе
    out << S_MSG_LOAD << " x " << (coef * (1 + addCount * step)) << endl;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_SI << maxSi << endl;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_ITERATION << --count << endl;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ') << endl;
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
template<class T> void TFEMStaticMVS<T>::setupFE(TFE *fe, unsigned i)
{
    double newE,
           feSi = si[TFEM::mesh->getFE(i, 0)];
    unsigned index;
    vector<double> cx;
    matrix<double> ssCurve;

    TFEM::setupFE(fe, i);
    // -------------- Линейный (упругий) случай ------------------
    if (iterNo == 0)
        return;

    // Загружаем диаграмму деформирования, соответствующую текущему КЭ
    TFEM::mesh->getCenterFE(i, cx);
    TFEM::params.getStressStrainCurve(cx, ssCurve);
    if (ssCurve.size1() == 0)
        throw NONLINEAR_PARAM_ERR;

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
            if (feSi > ssCurve[index - 1][0] && feSi <= ssCurve[index][0])
                break;

    if (index == ssCurve.size1())
    {
        // Достигнут предел текучести
        index--;
        isStopGlobalIteration = true;
    }

    if (index != index0[i])
        newE = fabsl((ssCurve[index][0] - ssCurve[index0[i]][0]) / (ssCurve[index][1] - ssCurve[index0[i]][1]));
    else
        newE = (e0[i] == 0.0) ? fe->getYoungModulus() : e0[i];

    fe->setYoungModulus(newE);
    fe->setPoissonRatio(TFEM::params.getPoissonRatio(cx));

    // Проверка на изменение модуля упругости по сравнению с предыдущей итерацией
    if (index != index0[i])
        isStopLocalIteration = false;

    // Запоминаем рассчитанное значение модуля упругости и индекс
    e0[i] = newE;
    index0[i] = index;
}
//----------------------------------------------------------------------------

#endif // FEMSTATIC_MVS_H
