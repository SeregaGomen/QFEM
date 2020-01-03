#ifndef FEMSTATIC_H
#define FEMSTATIC_H

#define OMP

#include <sstream>
#include <iomanip>

#include <omp.h>
#include "fem/fem.h"

extern TMessenger* msg;

#ifdef OMP
const int numThread = omp_get_max_threads();
#else
const int numThread = 1;
#endif

//----------------------------------------------------------
//  Реализация конечно-элементного расчета в соответствии с
//  вариационным принципом Лагранжа
//----------------------------------------------------------
template <class T> class TFEMStatic : public TFEM
{
protected:
    T solver;
    void ansambleLocalMatrix(TFE*, unsigned);
    void setBoundaryConstant(unsigned, unsigned, double);
    void setLoadConstant(unsigned, unsigned, double);
    void useLoadCondition(vector<double>&);
    void genResults(vector<double>&, bool isAdd = false);
    void avgResults(matrix<double>&, vector<int>&);
    virtual void calcResult(matrix<double>&, vector<double>&);
    virtual void saveResult(matrix<double>&, bool);
    virtual void getConcentratedLoad(vector<double>&, double&, unsigned, unsigned, double);
    virtual void getSurfaceLoad(vector<double>&, double&, unsigned, unsigned, double);
    virtual void getPressureLoad(vector<double>&, double&, unsigned, unsigned, double);
    virtual void getVolumeLoad(vector<double>&, double&, unsigned, unsigned, double);
    virtual void getBoundaryCondition(unsigned, unsigned);
    virtual void getFEResult(matrix<double>&, vector<double>&, vector<int>&, unsigned, unsigned);
    virtual void getMatrix(unsigned, unsigned, bool);
    virtual void calcBoundaryCondition(void);
    virtual void calcGlobalMatrix(bool = true);
    virtual double calcLoad(vector<double>&, double = 0);
    virtual double calcConcentratedLoad(vector<double>&, double = 0);
    virtual double calcSurfaceLoad(vector<double>&, double = 0);
    virtual double calcVolumeLoad(vector<double>&, double = 0);
    virtual double calcPressureLoad(vector<double>&, double = 0);
    bool checkBE(unsigned, TParameter&);
    bool checkFE(unsigned, TParameter&);
public:
    TFEMStatic(string n, TMesh* m, TResultList* r, list<string>* l) : TFEM(n, m, r, l)
    {
        TFEM::params.fType = StaticProblem;
    }
    virtual ~TFEMStatic(void) {}
    void startProcess(void);
};
//----------------------------------------------------------
//                     Запуск расчета
//----------------------------------------------------------
template<class T> void TFEMStatic<T>::startProcess(void)
{
    unsigned hour,
             min,
             sec;
    vector<double> res,
                   load(mesh->getNumVertex() * mesh->getFreedom());
    time_t full_timer;
    ostringstream out;

    isProcessStarted = true;
    isProcessAborted = false;
    solver.setupStaticMatrix(mesh);

    // Предварительное вычисление компонент нагрузки
    full_timer = clock();
    calcConcentratedLoad(load);
    calcSurfaceLoad(load);
    calcPressureLoad(load);
    calcVolumeLoad(load);

    // Формирование ГМЖ
    calcGlobalMatrix();

    // Учет нагрузки
    useLoadCondition(load);

    // Учет краевых условий
    calcBoundaryCondition();

    //////////////////
    // solver.print("matr.res");
    //////////////////

    // Решение СЛАУ
    if (solver.solve(res, params.eps, isProcessAborted))
        genResults(res); // Вычисление дополнительных результатов
    solver.clear();
    if (isProcessAborted)
        throw ABORT_ERR;
    isProcessStarted = false;
    isProcessCalculated = true;

    full_timer = clock() - full_timer;
    hour = (unsigned(full_timer) / CLOCKS_PER_SEC) / 3600;
    min = ((unsigned(full_timer) / CLOCKS_PER_SEC) % 3600) / 60;
    sec = (unsigned(full_timer) / CLOCKS_PER_SEC) - hour * 3600 - min * 60;

    // Сохраняем информацию о времени расчета
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ');
    TFEM::notes->push_back(out.str());
    cout << out.str() << endl;

    printResultSummary();
}
//-------------------------------------------------------------
//                  Ансамблирование ЛМЖ к ГМЖ
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::ansambleLocalMatrix(TFE* fe, unsigned i)
{
    unsigned freedom = mesh->getFreedom(),
             size = fe->getSize() * fe->getFreedom();


    // Учет матрицы
    for (unsigned l = 0; l < size; l++)
    {
        for (unsigned k = l; k < size; k++)
        {
            solver.addStiffnessMatrix(fe->getStiffnessMatrix(l, k), mesh->getFE(i, l / freedom) * freedom + l % freedom, mesh->getFE(i, k / freedom) * freedom + k % freedom);
            if (l != k)
                solver.addStiffnessMatrix(fe->getStiffnessMatrix(l, k), mesh->getFE(i, k / freedom) * freedom + k % freedom, mesh->getFE(i, l / freedom) * freedom + l % freedom);
        }
        solver.addLoadVector(fe->getLoad(l), mesh->getFE(i, l / freedom) * freedom + l % freedom);
    }
}
//-------------------------------------------------------------
//                  Учет граничных условий
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcBoundaryCondition(void)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumVertex() / (numThread - 1) : TFEM::mesh->getNumVertex();

    if (params.plist.findParameter(BOUNDARY_CONDITION_PARAMETER))
    {
        msg->setProcess(CALC_BOUNDARY_CONDITION_PROCESS, 1, mesh->getNumVertex());
#ifdef OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
            getBoundaryCondition(i * step, (i == numThread - 2) ? TFEM::mesh->getNumVertex() : (i + 1) * step);
        if (isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//                 Предварительное вычисление значений узловой нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcLoad(vector<double>& load, double t)
{
    vector<double> maxVal;

    maxVal.push_back(calcConcentratedLoad(load, t));
    maxVal.push_back(calcSurfaceLoad(load, t));
    maxVal.push_back(calcVolumeLoad(load, t));
    maxVal.push_back(calcPressureLoad(load, t));
    return max(max(max(maxVal[0], maxVal[1]), maxVal[2]), maxVal[3]);
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений узловой сосредоточенной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcConcentratedLoad(vector<double>& load, double t)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumVertex() / (numThread - 1) : TFEM::mesh->getNumVertex();
    double max_val = 0;

    if (params.plist.findParameter(CONCENTRATED_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_CONCENTRATED_LOAD_PROCESS, 1, mesh->getNumVertex());

#ifdef OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
            getConcentratedLoad(load, max_val, i * step, (i == numThread - 2) ? TFEM::mesh->getNumFE() : (i + 1) * step, t);
        if (isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений поверхностной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcSurfaceLoad(vector<double>& load, double t)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumBE() / (numThread - 1) : TFEM::mesh->getNumBE();
    double max_val = 0;

    if (params.plist.findParameter(SURFACE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_SURFACE_LOAD_PROCESS, 1, mesh->getNumBE());
#ifdef OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
            getSurfaceLoad(load, max_val, i * step, (i == numThread - 2) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
        if (isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//                Предварительное вычисление значений нагрузки давлением
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcPressureLoad(vector<double>& load, double t)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumBE() / (numThread - 1) : TFEM::mesh->getNumBE();
    double max_val = 0;

    if (params.plist.findParameter(PRESSURE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_PRESSURE_LOAD_PROCESS, 1, mesh->getNumBE());
#ifdef OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
            getPressureLoad(load, max_val, i * step, (i == numThread - 2) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
        if (isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений объемной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcVolumeLoad(vector<double>& load, double t)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumFE() / (numThread - 1) : TFEM::mesh->getNumFE();
    double max_val = 0;

    if (params.plist.findParameter(VOLUME_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_VOLUME_LOAD_PROCESS, 1, mesh->getNumFE());
#ifdef OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
            getVolumeLoad(load, max_val, i * step, (i == numThread - 2) ? TFEM::mesh->getNumFE() : (i + 1) * step, t);
        if (isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::saveResult(matrix<double>& res, bool isAdd)
{
    if (!isAdd)
        results->clear();
    // Cохраняем результаты
    for (unsigned i = 0; i < params.numResult(mesh->getTypeFE()); i++)
    {
        if (isAdd)
            results->addResult(res[i], res.size2(), params.getName(i, mesh->getTypeFE()));
        else
            results->setResult(res[i], res.size2(), params.getName(i, mesh->getTypeFE()));
    }
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcResult(matrix<double>& res, vector<double>& u)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumFE() / (numThread - 1) : TFEM::mesh->getNumFE();
    vector<int> counter(mesh->getNumVertex()); // Счетчик кол-ва вхождения узлов для осреднения результатов

    res.resize(params.numResult(mesh->getTypeFE()), mesh->getNumVertex());
    // Копируем результаты расчета (перемещения)
    for (unsigned i = 0; i < mesh->getNumVertex(); i++)
        for (unsigned j = 0; j < mesh->getFreedom(); j++)
            res[j][i] = u[i * mesh->getFreedom() + j];

    // Вычисляем стандартные результаты по всем КЭ
    msg->setProcess(CALCULATION_STANDART_RESULT_PROCESS, 1, mesh->getNumFE());
#ifdef OMP
#pragma omp parallel for
#endif
    for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
        getFEResult(res, u, counter, i * step, (i == numThread - 2) ? TFEM::mesh->getNumFE() : (i + 1) * step);
    if (isProcessAborted)
        throw ABORT_ERR;
    // Осредняем результаты
    avgResults(res, counter);
    msg->stopProcess();
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::genResults(vector<double>& u, bool isAdd)
{
    matrix<double> res;

    // Вычисляем стандартные результаты по всем КЭ
    calcResult(res, u);

    // Сохраняем их
    saveResult(res, isAdd);

    // Запоминаем дату и время расчета
    results->setCurrentSolutionTime();
}
//-------------------------------------------------------------------------------
// Проверка соответствия граничного элемента предикату отбора (всех его вершин)
//-------------------------------------------------------------------------------
template<class T> bool TFEMStatic<T>::checkBE(unsigned index, TParameter& p)
{
    vector<double> coord;

    for (unsigned i = 0; i < mesh->getBaseSizeBE(); i++)
    {
        mesh->getCoordVertex(mesh->getBE(index, i), coord);
        if (!params.getPredicateValue(p, coord))
            return false;
    }
    return true;
}
//-------------------------------------------------------------------------------
// Проверка соответствия конечного элемента предикату отбора (всех его вершин)
//-------------------------------------------------------------------------------
template<class T> bool TFEMStatic<T>::checkFE(unsigned index, TParameter& p)
{
    vector<double> coord;

    for (unsigned i = 0; i < mesh->getBaseSizeFE(); i++)
    {
        mesh->getCoordVertex(mesh->getFE(index, i), coord);
        if (!params.getPredicateValue(p, coord))
            return false;
    }
    return true;
}
//-------------------------------------------------------------
//      Формирование глобального вектора-столбца нагрузки
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::useLoadCondition(vector<double>& load)
{
    int size = mesh->getNumVertex() * mesh->getFreedom();

    msg->setProcess(CREATE_LOAD_PROCESS, 1, size);
    for (int i = 0; i < size; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            throw ABORT_ERR;
        solver.addLoadVector(load[i], i);
    }
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
//           Вычисление сосредоточенной нагрузки для заданного диапазона узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getConcentratedLoad(vector<double>& load, double& max_load, unsigned begin, unsigned end, double t)
{
    unsigned direct;
    double val,
           max_val[] = { 0, 0, 0 };
    vector<double> coord;

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == CONCENTRATED_LOAD_PARAMETER && (direct = unsigned(it->getDirect())))
            {
                if (isProcessAborted)
                    return;
                mesh->getCoordVertex(i, coord);
                coord.push_back(t);

                if (!params.getPredicateValue(*it, coord))
                    continue;
                val = params.getExpressionValue(*it, coord);
                if ((direct & DIR_X) == DIR_X || (mesh->isPlate() && (direct & DIR_Z) == DIR_Z)) // X или W - для пластины
                {
                    load[i * mesh->getFreedom() + 0] += val;
                    max_val[0] = (fabs(val) > max_val[0]) ? fabs(val) : max_val[0];
                }
                if ((direct & DIR_Y) == DIR_Y) // Y
                {
                    load[i * mesh->getFreedom() + 1] += val;
                    max_val[1] = (fabs(val) > max_val[1]) ? fabs(val) : max_val[1];
                }
                if ((direct & DIR_Z) == DIR_Z) // Z
                {
                    load[i * mesh->getFreedom() + 2] += val;
                    max_val[2] = (fabs(val) > max_val[2]) ? fabs(val) : max_val[2];
                }
            }
    }
    max_load = pow(max_val[0] * max_val[0] + max_val[1] * max_val[1] + max_val[2] * max_val[2], 0.5);
}
//-----------------------------------------------------------------------------------------
//     Вычисление поверхностной нагрузки для заданного диапазона граничных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getSurfaceLoad(vector<double>& load, double& max_load, unsigned begin, unsigned end, double t)
{
    unsigned direct;
    double val,
           max_val[] = { 0, 0, 0 };
    vector<double> share,
                   coord;

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == SURFACE_LOAD_PARAMETER && (direct = unsigned(it->getDirect())))
            {
                if (isProcessAborted)
                    return;
                // Проверка, все ли узлы ГЭ удвлетворяют предикату
                if (!checkBE(i, *it))
                    continue;
                share = mesh->surfaceLoadShare() * mesh->beVolume(i);
                mesh->getCenterBE(i, coord);
                coord.push_back(t);
                val = params.getExpressionValue(*it, coord);
                for (unsigned k = 0; k < mesh->getSizeBE(); k++)
                {
                    if ((direct & DIR_X) == DIR_X || (mesh->isPlate() && (direct & DIR_Z) == DIR_Z)) // X или W - для пластины
                    {
                        load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                        max_val[0] = (fabs(val) > max_val[0]) ? fabs(val) : max_val[0];
                    }
                    if ((direct & DIR_Y) == DIR_Y) // Y
                    {
                        load[mesh->getBE(i, k) * mesh->getFreedom() + 1] += val * share[k];
                        max_val[1] = (fabs(val) > max_val[1]) ? fabs(val) : max_val[1];
                    }
                    if ((direct & DIR_Z) == DIR_Z) // Z
                    {
                        load[mesh->getBE(i, k) * mesh->getFreedom() + 2] += val * share[k];
                        max_val[2] = (fabs(val) > max_val[2]) ? fabs(val) : max_val[2];
                    }
                }
            }
    }
    max_load = pow(max_val[0] * max_val[0] + max_val[1] * max_val[1] + max_val[2] * max_val[2], 0.5);
}
//-----------------------------------------------------------------------------------------
//      Вычисление нагрузки давлением для заданного диапазона граничных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getPressureLoad(vector<double>& load, double& max_load, unsigned begin, unsigned end, double t)
{
    double val,
           max_val[] = { 0, 0, 0 };
    vector<double> share,
                   coord,
                   v(3);

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == PRESSURE_LOAD_PARAMETER)
            {
                if (isProcessAborted)
                    return;
                // Проверка, все ли узлы ГЭ удвлетворяют предикату
                if (!checkBE(i, *it))
                    continue;
                // Вычисление нагрузки
                share = mesh->surfaceLoadShare() * mesh->beVolume(i);
                mesh->getCenterBE(i, coord);
                coord.push_back(t);
                val = params.getExpressionValue(*it, coord);
                // Вычисление нормали к ГЭ
                mesh->normal(i, v);

                for (unsigned k = 0; k < mesh->getSizeBE(); k++)
                {
                    if (!mesh->isPlate())
                    {
                        // X
                        load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k] * v[0];
                        max_val[0] = (fabs(val) > max_val[0]) ? fabs(val * v[0]) : max_val[0];
                        // Y
                        if (mesh->getFreedom() > 1)
                        {
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 1] += val * share[k] * v[1];
                            max_val[1] = (fabs(val) > max_val[1]) ? fabs(val * v[1]) : max_val[1];
                        }
                        // Z
                        if (mesh->getFreedom() > 2)
                        {
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 2] += val * share[k] * v[2];
                            max_val[2] = (fabs(val) > max_val[2]) ? fabs(val * v[2]) : max_val[2];
                        }
                    }
                    else
                    {
                        load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                        max_val[0] = (fabs(val) > max_val[0]) ? fabs(val) : max_val[0];
                    }
                }
            }
    }
    max_load = pow(max_val[0] * max_val[0] + max_val[1] * max_val[1] + max_val[2] * max_val[2], 0.5);
}
//-----------------------------------------------------------------------------------------
//         Вычисление объемной нагрузки для заданного интервала конечных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getVolumeLoad(vector<double>& load, double& max_load, unsigned begin, unsigned end, double t)
{
    unsigned direct;
    double val,
           max_val[] = { 0, 0, 0 };
    vector<double> share,
                   coord;

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == VOLUME_LOAD_PARAMETER && (direct = unsigned(it->getDirect())))
            {
                if (isProcessAborted)
                    return;
                // Проверка, все ли узлы КЭ удовлетворяют предикату
                if (!checkFE(i, *it))
                    continue;
                share = mesh->volumeLoadShare() * mesh->feVolume(i);
                mesh->getCenterFE(i, coord);
                coord.push_back(t);
                val = params.getExpressionValue(*it, coord);
                for (unsigned k = 0; k < mesh->getSizeFE(); k++)
                {
                    if ((direct & DIR_X) == DIR_X || (mesh->isPlate() && (direct & DIR_Z) == DIR_Z)) // X или W - для пластины
                    {
                        load[mesh->getFE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                        max_val[0] = (fabs(val) > max_val[0]) ? fabs(val) : max_val[0];
                    }
                    if ((direct & DIR_Y) == DIR_Y) // Y
                    {
                        load[mesh->getFE(i, k) * mesh->getFreedom() + 1] += val * share[k];
                        max_val[1] = (fabs(val) > max_val[1]) ? fabs(val) : max_val[1];
                    }
                    if ((direct & DIR_Z) == DIR_Z) // Z
                    {
                        load[mesh->getFE(i, k) * mesh->getFreedom() + 2] += val * share[k];
                        max_val[2] = (fabs(val) > max_val[2]) ? fabs(val) : max_val[2];
                    }
                }
            }
    }
    max_load = pow(max_val[0] * max_val[0] + max_val[1] * max_val[1] + max_val[2] * max_val[2], 0.5);
}
//-----------------------------------------------------------------------------------------
//             Вычисление граничных условий для заданного интервала узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getBoundaryCondition(unsigned begin, unsigned end)
{
    double val;
    unsigned direct;
    vector<double> coord;

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == BOUNDARY_CONDITION_PARAMETER && (direct = unsigned(it->getDirect())))
            {
                if (isProcessAborted)
                    return;
                mesh->getCoordVertex(i, coord);
                coord.push_back(0.0); // t = 0
                if (params.getPredicateValue(*it, coord))
                {
                    val = params.getExpressionValue(*it, coord);
                    if ((direct & DIR_X) == DIR_X)
                        solver.setBoundaryCondition(i, 0, val);
                    if ((direct & DIR_Y) == DIR_Y)
                        solver.setBoundaryCondition(i, 1, val);
                    if ((direct & DIR_Z) == DIR_Z)
                        solver.setBoundaryCondition(i, 2, val);
                }
            }
    }
}
//-----------------------------------------------------------------------------------------
//             Формирование глобальных матриц жесткости, масс и демпфирования
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcGlobalMatrix(bool isStatic)
{
    int step = (numThread > 1) ? TFEM::mesh->getNumFE() / (numThread - 1) : TFEM::mesh->getNumFE();

    msg->setProcess((isStatic) ? GENERATE_FE_STATIC_PROCESS : GENERATE_FE_DYNAMIC_PROCESS, 1, mesh->getNumFE());
#ifdef OMP
#pragma omp parallel for
#endif
    for (int i = 0; i < ((numThread == 1) ? numThread : numThread - 1); i++)
        getMatrix(i * step, (i == numThread - 2) ? TFEM::mesh->getNumFE() : (i + 1) * step, isStatic);
    if (isProcessAborted)
        throw ABORT_ERR;
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
//             Вычисление локальных МЖ, ММ и МД для заданного интервала узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getMatrix(unsigned begin, unsigned end, bool isStatic)
{
    TFE *fe = TFEM::createFE();
    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            return;
        // Настройка КЭ
        setupFE(fe, i);
        // Формирование локальной МЖ (ЛМЖ)
        fe->generate(isStatic);
        // Ансамблирование ЛМЖ к ГМЖ
        ansambleLocalMatrix(fe, i);
    }
    TFEM::removeFE(fe);
}
//-----------------------------------------------------------------------------------------
//             Вычисление стандартных результатов КЭ для заданного интервала
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getFEResult(matrix<double>& res, vector<double>& u, vector<int>& counter, unsigned begin, unsigned end)
{
    vector<double> fe_u;
    matrix<double> fe_res(params.numResult(mesh->getTypeFE()), mesh->getSizeFE());
    TFE *fe = TFEM::createFE();

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            return;
        setupFE(fe, i);
        // Формируем вектор перемещений для текущего КЭ
        fe_u.resize(mesh->getSizeFE() * mesh->getFreedom());
        for (unsigned j = 0; j < mesh->getSizeFE(); j++)
            for (unsigned k = 0; k < mesh->getFreedom(); k++)
                fe_u[j * mesh->getFreedom() + k] = u[mesh->getFreedom() * mesh->getFE(i, j) + k];
        fe->calc(fe_res, fe_u);
        for (unsigned m = 0; m < params.numResult(mesh->getTypeFE()) - mesh->getFreedom(); m++)
            for (unsigned j = 0; j < mesh->getSizeFE(); j++)
            {
                res[m + mesh->getFreedom()][mesh->getFE(i, j)] += fe_res[m][j];
                if (m == 0)
                    counter[mesh->getFE(i, j)]++;
            }
        fe_res.fill(0);
    }
    TFEM::removeFE(fe);
}
//-----------------------------------------------------------------------------------------
//                Осреднение результатов расчета деформаций, напряжений, ...
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::avgResults(matrix<double>& result, vector<int>& counter)
{
    for (unsigned i = mesh->getFreedom(); i < params.numResult(mesh->getTypeFE()); i++)
        for (unsigned j = 0; j< mesh->getNumVertex(); j++)
        {
            result[i][j] /= counter[j];
            // Для "красоты" вывода результатов обнуляем близкие к нулю значения
            if (fabs(result[i][j]) < params.eps)
                result[i][j] = 0;
        }
}
//-----------------------------------------------------------------------------------------
#endif // FEMSTATIC_H
