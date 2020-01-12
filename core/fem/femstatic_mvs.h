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
    double maxAvgSi;                        // Максимальная средняя по КЭ интенсивность напряжений
    bool isStopGlobalIteration;             // Признак того, что расчет окончен (достигнут предел текучести)
    bool isStopLocalIteration;              // Признак того, что итерационный процесс для заданной нагрузки окончен
    vector<double> si;                      // Интенсивность напряжений (по элементам)
    vector<double> e0;                      // Модули упругости (для каждого КЭ), полученные на предыдущей итерации
    vector<unsigned> index0;                // Индекс упругих свойств, полученный на предыдущей итерации
    void setupFE(TFE*, unsigned);           // Настройка парметров КЭ (для нелинейного случая)
    void calcIteration(void);
    double calcStressIntensity(TResultList&);
public:
    TFEMStaticMVS(double& step, string n, TMesh* m, TResultList* r, list<string>* l) : TFEMStatic<T>(n, m, r, l)
    {
        maxAvgSi = 0;
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
           maxLoad,
           loadFactor,
           addLoad = 0,
           step = loadStep / 100;
    unsigned hour,
             min,
             sec,
             count = 1;
    vector<double> result,
                   load(TFEM::mesh->getNumVertex() * TFEM::mesh->getFreedom());
    time_t full_timer;
    bool isLoaded = false,
         isStopRewind = false;
    ostringstream out;

    maxLoad = 0;
    TFEM::isProcessStarted = true;
    TFEM::isProcessAborted = false;

    full_timer = clock();
    // Предварительное вычисление компонент нагрузки
    maxLoad = TFEMStatic<T>::calcLoad(load);

    maxSsc = TFEM::params.getMinStress();

    // Итерационный процесс линеаризации упруго-пластической задачи
    e0.resize(TFEM::mesh->getNumFE());
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
        if (TFEMStatic<T>::solver.solve(result,TFEM::params.eps,TFEM::isProcessAborted))
        {
            // Вычисление дополнительных результатов
            if (iterNo == 0)
            {
                TFEMStatic<T>::genResults(result);
                // Вычисление интенсивности напряжений
                maxSi = calcStressIntensity(*TFEM::results);
            }
            else
                if (isStopLocalIteration)
                {
                    TFEMStatic<T>::genResults(result, true);
                    maxSi = calcStressIntensity(*TFEM::results);
                }

            // Вывод рез-тов по каждой функции на экран
            TFEM::printResultSummary();
            // Вывод информации об итерации
            cout << S_MSG_LOAD << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxLoad << endl;
            cout << S_MSG_SI << " max: " << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxSi << ", avg: " << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxAvgSi <<endl;
            cout << S_MSG_ITERATION << count++ << endl << endl;
            cout.unsetf(ios::scientific);

            if (iterNo == 0)
            {
                if (maxSi > maxSsc && !isStopRewind)
                {
                    // Задана слишком большая первоначальная нагрузка, уменьшаем ее на порядок
                    for_each(load.begin(), load.end(), [](double& i) -> double{ return i *= 0.1; });
                    maxLoad *= 0.1;
                    if (maxLoad < TFEM::params.eps) // Если не удается вернуться в упругую зону
                    {
                        isStopRewind = true;
                        isLoaded = true;
                    }
                    iterNo--;
                }
                else
                {
                    if (!isLoaded)
                    {
                        // Вычисляем поправочный коэффициент для "пропуска" упругой зоны
                        loadFactor = 0.95 * (maxSsc / maxSi);
                        maxLoad *= loadFactor;
                        for_each(load.begin(), load.end(), [loadFactor](double& i) -> double{ return i *= loadFactor; });
                        isLoaded = true;
                        iterNo--;
                    }
                    else
                    {
                        // Устанавливаем нагрузку в значение "шаг по нагрузке"
                        for_each(load.begin(), load.end(), [step](double& i) -> double{ return i *= step; });
                        addLoad = maxLoad * step;
                    }
                }
            }
            if (++iterNo > 0 && isStopLocalIteration)
            {
                maxLoad += addLoad;
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
    out << S_MSG_LOAD << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxLoad;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_SI << " max: " << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxSi << ", avg: " << setw(TFEM::params.width) << setprecision(TFEM::params.precision) << maxAvgSi;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_ITERATION << --count;
    TFEM::notes->push_back(out.str());
    out.str("");
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ');
    TFEM::notes->push_back(out.str());
    cout << out.str() << endl;
}
//---------------------------------------------------------------------------------
//                       Вычисление интенсивности напряжений
//    Si=((Sxx-Syy)^2+(Sxx-Szz)^2+(Syy-Szz)^2+6*(Sxy^2+Sxz^2+Syz^2))^0.5/2^0.5
//---------------------------------------------------------------------------------
template<class T> double TFEMStaticMVS<T>::calcStressIntensity(TResultList& res)
{
    double m_sqrt1_2 = 0.5*sqrt(2.0);
    unsigned dimension = TFEM::mesh->getDimension();

    // Вычисление узловых значений интенсивности напряжений
    si.resize(TFEM::mesh->getNumVertex(),0);
    for (unsigned i = 0; i < TFEM::mesh->getNumVertex(); i++)
        switch (dimension)
        {
            case 1:
                si[i] = fabs(res[2].getResults(i))*m_sqrt1_2;
                break;
            case 2:
                si[i] = sqrt(pow(res[5].getResults(i) - res[6].getResults(i), 2) + 6.0*(pow(res[7].getResults(i), 2)))*m_sqrt1_2;
                break;
            case 3:
                si[i] = sqrt(pow(res[9].getResults(i) - res[10].getResults(i), 2) +
                             pow(res[9].getResults(i) - res[11].getResults(i), 2) +
                             pow(res[10].getResults(i) - res[11].getResults(i), 2) +
                             6.0*(pow(res[12].getResults(i), 2) + pow(res[13].getResults(i), 2) + pow(res[14].getResults(i), 2)))*m_sqrt1_2;
        }
    return *std::max_element(si.begin(),si.end());
}
//----------------------------------------------------------------------------
//                      Настройка упругих парметров КЭ
//----------------------------------------------------------------------------
template<class T> void TFEMStaticMVS<T>::setupFE(TFE *fe, unsigned i)
{
    double newE,
           feSi = 0;
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
    for (unsigned j = 0; j < TFEM::mesh->getSizeFE(); j++)
        feSi += si[TFEM::mesh->getFE(i, j)];
    feSi /= double(TFEM::mesh->getSizeFE());
    maxAvgSi = (feSi > maxAvgSi) ? feSi : maxAvgSi;


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
