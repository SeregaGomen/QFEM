#ifndef FEMTSTATIC_T_H
#define FEMTSTATIC_T_H

#include "femstatic.h"

extern TMessenger* msg;

//----------------------------------------------------------
//  Многопоточная реализация конечно-элементного расчета
//  в соответствии с вариационным принципом Лагранжа
//----------------------------------------------------------
template <class T> class TFEMStaticThread : public TFEMStatic<T>
{
private:
    unsigned maxThread;
protected:
    virtual void calcBoundaryCondition(void);
    virtual double calcConcentratedLoad(vector<double>&, double = 0);
    virtual double calcSurfaceLoad(vector<double>&, double = 0);
    virtual double calcVolumeLoad(vector<double>&, double = 0);
    virtual double calcPressureLoad(vector<double>&, double = 0);
    virtual void calcGlobalMatrix(bool = true);
    virtual void calcResult(matrix<double>&, vector<double>&);
public:
    TFEMStaticThread<T>(string n, TMesh* m, TResultList* r, list<string>* l) : TFEMStatic<T>(n, m, r, l)
    {
        maxThread = (thread::hardware_concurrency() > 1) ? thread::hardware_concurrency() - 1 : 0;
    }
    virtual ~TFEMStaticThread(void) {}
};
//-----------------------------------------------------------------------------------------
//                      Многопоточное вычисление нагрузки давлением
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStaticThread<T>::calcPressureLoad(vector<double>& load, double t)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcPressureLoad(load, t);
    if (TFEM::params.plist.findParameter(PRESSURE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_PRESSURE_LOAD_PROCESS, 1, TFEM::mesh->getNumBE());
        step = TFEM::mesh->getNumBE() / maxThread;
        for (unsigned i = 0; i < maxThread; i++)
            thr[i] = thread(&TFEMStaticThread<T>::getPressureLoad, this, ref(load), ref(maxVal[i]), i * step, (i == maxThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return *max_element(maxVal.begin(), maxVal.end());
}
//-----------------------------------------------------------------------------------------
//                                ... поверхностной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStaticThread<T>::calcSurfaceLoad(vector<double>& load, double t)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcSurfaceLoad(load, t);
    if (TFEM::params.plist.findParameter(SURFACE_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_SURFACE_LOAD_PROCESS, 1, TFEM::mesh->getNumBE());
        step = TFEM::mesh->getNumBE() / maxThread;
        for (unsigned i = 0; i < maxThread; i++)
            thr[i] = thread(&TFEMStaticThread<T>::getSurfaceLoad, this, ref(load), ref(maxVal[i]), i * step, (i == maxThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t);
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });

        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return *max_element(maxVal.begin(), maxVal.end());
}
//-----------------------------------------------------------------------------------------
//                              ... сосредоточенной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStaticThread<T>::calcConcentratedLoad(vector<double>& load, double t)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcConcentratedLoad(load, t);
    if (TFEM::params.plist.findParameter(CONCENTRATED_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_CONCENTRATED_LOAD_PROCESS, 1, TFEM::mesh->getNumVertex());
        step = TFEM::mesh->getNumVertex() / maxThread;
        for (unsigned i = 0; i < maxThread; i++)
            thr[i] = thread(&TFEMStaticThread<T>::getConcentratedLoad, this, ref(load), ref(maxVal[i]), i * step, (i == maxThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step, t);
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return *max_element(maxVal.begin(), maxVal.end());
}
//-----------------------------------------------------------------------------------------
//                              ... объемной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStaticThread<T>::calcVolumeLoad(vector<double>& load, double t)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcVolumeLoad(load, t);
    if (TFEM::params.plist.findParameter(VOLUME_LOAD_PARAMETER))
    {
        msg->setProcess(CALCULATION_VOLUME_LOAD_PROCESS, 1, TFEM::mesh->getNumFE());
        step = TFEM::mesh->getNumFE() / maxThread;
        for (unsigned i = 0; i < maxThread; i++)
            thr[i] = thread(&TFEMStaticThread<T>::getVolumeLoad, this, ref(load), ref(maxVal[i]), i * step, (i == maxThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, t);
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
    return *max_element(maxVal.begin(), maxVal.end());
}
//-----------------------------------------------------------------------------------------
//                              ... граничных условий
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStaticThread<T>::calcBoundaryCondition(void)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcBoundaryCondition();
    if (TFEM::params.plist.findParameter(BOUNDARY_CONDITION_PARAMETER))
    {
        msg->setProcess(CALC_BOUNDARY_CONDITION_PROCESS, 1, TFEM::mesh->getNumVertex());
        step = TFEM::mesh->getNumVertex() / maxThread;
        for (unsigned i = 0; i < maxThread; i++)
            thr[i] = thread(&TFEMStaticThread<T>::getBoundaryCondition, this, i * step, (i == maxThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step);
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (TFEM::isProcessAborted)
            throw ABORT_ERR;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//                              ... граничных условий
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStaticThread<T>::calcGlobalMatrix(bool isStatic)
{
    unsigned step;
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcGlobalMatrix(isStatic);
    msg->setProcess((isStatic) ? GENERATE_FE_STATIC_PROCESS : GENERATE_FE_DYNAMIC_PROCESS, 1, TFEM::mesh->getNumFE());
    step = TFEM::mesh->getNumFE() / maxThread;
    for (unsigned i = 0; i < maxThread; i++)
        thr[i] = thread(&TFEMStaticThread<T>::getMatrix, this, i * step, (i == maxThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, isStatic);
    for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
    if (TFEM::isProcessAborted)
        throw ABORT_ERR;
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStaticThread<T>::calcResult(matrix<double>& res, vector<double>& u)
{
    unsigned step;
    vector<int> counter(TFEM::mesh->getNumVertex()); // Счетчик кол-ва вхождения узлов для осреднения результатов
    vector<double> maxVal(maxThread);
    vector<thread> thr(maxThread);

    if (maxThread == 0)
        return TFEMStatic<T>::calcResult(res, u);

    res.resize(TFEM::params.numResult(TFEM::mesh->getTypeFE()), TFEM::mesh->getNumVertex());
    // Копируем результаты расчета (перемещения)
    for (unsigned i = 0; i < TFEM::mesh->getNumVertex(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getFreedom(); j++)
            res[j][i] = u[i * TFEM::mesh->getFreedom() + j];

    // Вычисляем стандартные результаты по всем КЭ
    msg->setProcess(CALCULATION_STANDART_RESULT_PROCESS, 1, TFEM::mesh->getNumFE());
    step = TFEM::mesh->getNumFE() / maxThread;
    for (unsigned i = 0; i < maxThread; i++)
        thr[i] = thread(&TFEMStaticThread<T>::getFEResult, this, ref(res), ref(u), ref(counter), i * step, (i == maxThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step);
    for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
    if (TFEM::isProcessAborted)
        throw ABORT_ERR;

    // Осредняем результаты
    TFEMStatic<T>::avgResults(res, counter);
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------


#endif // FEMTSTATIC_T_H
