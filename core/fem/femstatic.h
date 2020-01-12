#ifndef FEMSTATIC_H
#define FEMSTATIC_H

#include <sstream>
#include <iomanip>
#include "fem/fem.h"


#define OPENMP // Многопоточный расчет

#ifdef OPENMP
#include <omp.h>
const int numThread = 4;//omp_get_max_threads() - 1;
#else
const int numThread = 1;
#endif

extern TMessenger* msg;


//----------------------------------------------------------
//  Реализация конечно-элементного расчета в соответствии с
//  вариационным принципом Лагранжа
//----------------------------------------------------------
template <class T> class TFEMStatic : public TFEM
{
protected:
    T solver;
    void ansambleLocalMatrix(TFE*, unsigned);
    void setLoad(vector<double>&);
    void genResults(vector<double>&, bool isAdd = false);
    void avgResults(matrix<double>&, vector<int>&);
    void calcResult(matrix<double>&, vector<double>&);
    void saveResult(matrix<double>&, bool);
    void getConcentratedLoad(vector<double>&, double&, unsigned, unsigned, double);
    void getSurfaceLoad(vector<double>&, double&, unsigned, unsigned, double);
    void getPressureLoad(vector<double>&, double&, unsigned, unsigned, double);
    void getVolumeLoad(vector<double>&, double&, unsigned, unsigned, double);
    void getBoundaryCondition(unsigned, unsigned);
    void getFEResult(matrix<double>&, vector<double>&, vector<int>&, unsigned, unsigned);
    void getMatrix(unsigned, unsigned, bool);
    void getLoad(vector<double>&, unsigned, unsigned);
    void calcBoundaryCondition(void);
    void calcGlobalMatrix(bool = true);
    double calcLoad(vector<double>&, double = 0);
    double calcConcentratedLoad(vector<double>&, double = 0);
    double calcSurfaceLoad(vector<double>&, double = 0);
    double calcVolumeLoad(vector<double>&, double = 0);
    double calcPressureLoad(vector<double>&, double = 0);
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
    solver.setMatrix(mesh);

    cout << S_NUM_THREAD << numThread << endl;

    // Предварительное вычисление компонент нагрузки
    full_timer = clock();
    calcConcentratedLoad(load);
    calcSurfaceLoad(load);
    calcPressureLoad(load);
    calcVolumeLoad(load);

    // Формирование ГМЖ
    calcGlobalMatrix();

    // Учет нагрузки
    setLoad(load);

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
            solver.addStiffness(fe->getStiffnessMatrix(l, k), mesh->getFE(i, l / freedom) * freedom + l % freedom, mesh->getFE(i, k / freedom) * freedom + k % freedom);
            if (l != k)
                solver.addStiffness(fe->getStiffnessMatrix(l, k), mesh->getFE(i, k / freedom) * freedom + k % freedom, mesh->getFE(i, l / freedom) * freedom + l % freedom);
        }
        solver.addLoad(fe->getLoad(l), mesh->getFE(i, l / freedom) * freedom + l % freedom);
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
//-------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getLoad(vector<double>& load, unsigned begin, unsigned end)
{
    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            throw ABORT_ERR;
        solver.addLoad(load[i], i);
    }
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
                    throw ABORT_ERR;
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
                    throw ABORT_ERR;
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
                    throw ABORT_ERR;
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
                    throw ABORT_ERR;
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
    unsigned direct,
             freedom = TFEM::mesh->getFreedom();
    vector<double> coord;

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        for (auto it = params.plist.begin(); it != params.plist.end(); it++)
            if (it->getType() == BOUNDARY_CONDITION_PARAMETER && (direct = unsigned(it->getDirect())))
            {
                if (isProcessAborted)
                    throw ABORT_ERR;
                mesh->getCoordVertex(i, coord);
                coord.push_back(0.0); // t = 0
                if (params.getPredicateValue(*it, coord))
                {
                    val = params.getExpressionValue(*it, coord);
                    if ((direct & DIR_X) == DIR_X)
                        solver.setBoundaryCondition(i * freedom + 0, val);
                    if ((direct & DIR_Y) == DIR_Y)
                        solver.setBoundaryCondition(i * freedom + 1, val);
                    if ((direct & DIR_Z) == DIR_Z)
                        solver.setBoundaryCondition(i * freedom + 2, val);
                }
            }
    }
}
//-----------------------------------------------------------------------------------------
//             Вычисление локальных МЖ, ММ и МД для заданного интервала узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getMatrix(unsigned begin, unsigned end, bool isStatic)
{
    TRFE fe(TFEM::mesh->getTypeFE());

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            throw ABORT_ERR;
        // Настройка КЭ
        setupFE(fe.getFE(), i);
        // Формирование локальной МЖ (ЛМЖ)
        fe.getFE()->generate(isStatic);
        // Ансамблирование ЛМЖ к ГМЖ
        ansambleLocalMatrix(fe.getFE(), i);
    }
}
//-----------------------------------------------------------------------------------------
//             Вычисление стандартных результатов КЭ для заданного интервала
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getFEResult(matrix<double>& res, vector<double>& u, vector<int>& counter, unsigned begin, unsigned end)
{
    vector<double> fe_u;
    matrix<double> fe_res(params.numResult(mesh->getTypeFE()), mesh->getSizeFE());
    TRFE fe(TFEM::mesh->getTypeFE());

    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
            throw ABORT_ERR;
        setupFE(fe.getFE(), i);
        // Формируем вектор перемещений для текущего КЭ
        fe_u.resize(mesh->getSizeFE() * mesh->getFreedom());
        for (unsigned j = 0; j < mesh->getSizeFE(); j++)
            for (unsigned k = 0; k < mesh->getFreedom(); k++)
                fe_u[j * mesh->getFreedom() + k] = u[mesh->getFreedom() * mesh->getFE(i, j) + k];
        fe.getFE()->calc(fe_res, fe_u);
        for (unsigned m = 0; m < params.numResult(mesh->getTypeFE()) - mesh->getFreedom(); m++)
            for (unsigned j = 0; j < mesh->getSizeFE(); j++)
            {
                res[m + mesh->getFreedom()][mesh->getFE(i, j)] += fe_res[m][j];
                if (m == 0)
                    counter[mesh->getFE(i, j)]++;
            }
        fe_res.fill(0);
    }
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
//             Формирование глобальных матриц жесткости, масс и демпфирования
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcGlobalMatrix(bool isStatic)
{
    ErrorCode error = NO_ERR;

    msg->setProcess((isStatic) ? GENERATE_FE_STATIC_PROCESS : GENERATE_FE_DYNAMIC_PROCESS, 1, mesh->getNumFE());
#ifdef OPENMP
    int step = TFEM::mesh->getNumFE() / numThread;

#pragma omp parallel num_threads(numThread)
    {
        int i = omp_get_thread_num();

        try
        {
            getMatrix(i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, isStatic);
        }
        catch (ErrorCode& err)
        {
            error = err;
        }
    }
#else
    getMatrix(0, TFEM::mesh->getNumFE(), isStatic);
#endif
    if (error)
        throw error;
    msg->stopProcess();
}
//-------------------------------------------------------------
//                  Учет граничных условий
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcBoundaryCondition(void)
{
    ErrorCode error = NO_ERR;

    if (params.plist.findParameter(BOUNDARY_CONDITION_PARAMETER))
    {
        msg->setProcess(CALC_BOUNDARY_CONDITION_PROCESS, 1, mesh->getNumVertex());
#ifdef OPENMP
        int step = TFEM::mesh->getNumVertex() / numThread;

#pragma omp parallel num_threads(numThread)
        {
            int i = omp_get_thread_num();

            try
            {
                getBoundaryCondition(i * step, (i == numThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step);
            }
            catch (ErrorCode& err)
            {
                error = err;
            }
        }
#else
        getBoundaryCondition(0, TFEM::mesh->getNumVertex());
#endif
        if (error)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений узловой сосредоточенной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcConcentratedLoad(vector<double>& load, double t)
{
    ErrorCode error = NO_ERR;
    double max_val = 0;

    if (params.plist.findParameter(CONCENTRATED_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_CONCENTRATED_LOAD_PROCESS, 1, mesh->getNumVertex());
#ifdef OPENMP
        int step = TFEM::mesh->getNumVertex() / numThread;
        vector<double> thd_max_val(numThread);

#pragma omp parallel num_threads(numThread)
        {
            int i = omp_get_thread_num();

            try
            {
                getConcentratedLoad(load, thd_max_val[i], i * step, (i == numThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step, t);
            }
            catch (ErrorCode& err)
            {
                error = err;
            }
        }
        if (error == NO_ERR)
            max_val = *max_element(thd_max_val.begin(), thd_max_val.end());
#else
        getConcentratedLoad(load, max_val, 0, TFEM::mesh->getNumVertex(), t);
#endif
        if (error)
            throw error;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений поверхностной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcSurfaceLoad(vector<double>& load, double t)
{
    ErrorCode error = NO_ERR;
    double max_val = 0;

    if (params.plist.findParameter(SURFACE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_SURFACE_LOAD_PROCESS, 1, mesh->getNumBE());
#ifdef OPENMP
        int step = TFEM::mesh->getNumBE() / numThread;
        vector<double> thd_max_val(numThread);

#pragma omp parallel num_threads(numThread)
        {
            int i = omp_get_thread_num();

            try
            {
                getSurfaceLoad(load, thd_max_val[i], i * step, (i == numThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
            }
            catch (ErrorCode& err)
            {
                error = err;
            }
        }
        if (error == NO_ERR)
            max_val = *max_element(thd_max_val.begin(), thd_max_val.end());
#else
        getSurfaceLoad(load, max_val, 0, TFEM::mesh->getNumBE(), t);
#endif
        if (error)
            throw error;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//                Предварительное вычисление значений нагрузки давлением
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcPressureLoad(vector<double>& load, double t)
{
    ErrorCode error = NO_ERR;
    double max_val = 0;

    if (params.plist.findParameter(PRESSURE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_PRESSURE_LOAD_PROCESS, 1, mesh->getNumBE());
#ifdef OPENMP
        int step = TFEM::mesh->getNumBE() / numThread;
        vector<double> thd_max_val(numThread);

#pragma omp parallel num_threads(numThread)
        {
            int i = omp_get_thread_num();

            try
            {
                getPressureLoad(load, thd_max_val[i], i * step, (i == numThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
            }
            catch (ErrorCode& err)
            {
                error = err;
            }
        }
        if (error == NO_ERR)
            max_val = *max_element(thd_max_val.begin(), thd_max_val.end());
#else
        getPressureLoad(load, max_val, 0, TFEM::mesh->getNumBE(), t);
#endif
        if (error)
            throw error;
        msg->stopProcess();
    }
    return max_val;
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений объемной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcVolumeLoad(vector<double>& load, double t)
{
    ErrorCode error = NO_ERR;
    double max_val = 0;

    if (params.plist.findParameter(VOLUME_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_VOLUME_LOAD_PROCESS, 1, mesh->getNumFE());
#ifdef OPENMP
        int step = TFEM::mesh->getNumFE() / numThread;
        vector<double> thd_max_val(numThread);

#pragma omp parallel num_threads(numThread)
        {
            int i = omp_get_thread_num();

            try
            {
                getVolumeLoad(load, thd_max_val[i], i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, t);
            }
            catch (ErrorCode& err)
            {
                error = err;
            }
        }
        if (error == NO_ERR)
            max_val = *max_element(thd_max_val.begin(), thd_max_val.end());
#else
        getVolumeLoad(load, max_val, 0, TFEM::mesh->getNumFE(), t);
#endif
        if (error)
            throw error;
        msg->stopProcess();
    }
    return max_val;
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcResult(matrix<double>& res, vector<double>& u)
{
    ErrorCode error = NO_ERR;
    vector<int> counter(mesh->getNumVertex()); // Счетчик кол-ва вхождения узлов для осреднения результатов

    res.resize(params.numResult(mesh->getTypeFE()), mesh->getNumVertex());
    // Копируем результаты расчета (перемещения)
    for (unsigned i = 0; i < mesh->getNumVertex(); i++)
        for (unsigned j = 0; j < mesh->getFreedom(); j++)
            res[j][i] = u[i * mesh->getFreedom() + j];

    // Вычисляем стандартные результаты по всем КЭ
    msg->setProcess(CALCULATION_STANDART_RESULT_PROCESS, 1, mesh->getNumFE());
#ifdef OPENMP
    int step = TFEM::mesh->getNumFE() / numThread;

#pragma omp parallel num_threads(numThread)
    {
        int i = omp_get_thread_num();

        try
        {
            getFEResult(res, u, counter, i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step);
        }
        catch (ErrorCode& err)
        {
            error = err;
        }
    }
#else
    getFEResult(res, u, counter, 0, TFEM::mesh->getNumFE());
#endif
    if (error)
        throw error;
    // Осредняем результаты
    avgResults(res, counter);
    msg->stopProcess();
}
//-------------------------------------------------------------
//      Формирование глобального вектора-столбца нагрузки
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::setLoad(vector<double>& load)
{
    ErrorCode error = NO_ERR;
    int size = mesh->getNumVertex() * mesh->getFreedom();

    msg->setProcess(CREATE_LOAD_PROCESS, 1, size);
#ifdef OPENMP
    int step = size / numThread;

#pragma omp parallel num_threads(numThread)
    {
        int i = omp_get_thread_num();

        try
        {
            getLoad(load, i * step, (i == numThread - 1) ? size : (i + 1) * step);
        }
        catch (ErrorCode& err)
        {
            error = err;
        }
    }
#else
    getLoad(load, 0, size);
#endif
    if (error)
        throw error;
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
#endif // FEMSTATIC_H
