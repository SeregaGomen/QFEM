#ifndef FEMSTATIC_H
#define FEMSTATIC_H

#include <sstream>
#include <iomanip>
#include "fem/fem.h"

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
    void getConcentratedLoad(vector<double>&, unsigned, unsigned, double, ErrorCode&);
    void getSurfaceLoad(vector<double>&, unsigned, unsigned, double, ErrorCode&);
    void getPressureLoad(vector<double>&, unsigned, unsigned, double, ErrorCode&);
    void getVolumeLoad(vector<double>&, unsigned, unsigned, double, ErrorCode&);
    void getBoundaryCondition(unsigned, unsigned, ErrorCode&);
    void getFEResult(matrix<double>&, vector<double>&, vector<int>&, unsigned, unsigned, ErrorCode&);
    void getMatrix(unsigned, unsigned, bool, ErrorCode&);
    void getLoad(vector<double>&, unsigned, unsigned, ErrorCode&);
    void getStressIntensity(TResultList&, vector<double>&, unsigned, unsigned);
    void calcBoundaryCondition(void);
    void calcGlobalMatrix(bool = true);
    void calcLoad(vector<double>&, double = 0);
    void calcConcentratedLoad(vector<double>&, double = 0);
    void calcSurfaceLoad(vector<double>&, double = 0);
    void calcVolumeLoad(vector<double>&, double = 0);
    void calcPressureLoad(vector<double>&, double = 0);
    double calcStressIntensity(TResultList&, vector<double>&);
    bool checkBE(unsigned, TParameter&);
    bool checkFE(unsigned, TParameter&);
public:
    TFEMStatic(string n, TMesh *m, TResultList *r, list<string> *l) : TFEM(n, m, r, l)
    {
        TFEM::params.fType = FEMType::StaticProblem;
    }
    virtual ~TFEMStatic(void) {}
    virtual void startProcess(void) override;
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
    ostringstream out;

    isProcessStarted = true;
    isProcessAborted = false;
    solver.setMatrix(mesh);

    cout << S_NUM_THREAD << numThread << endl;

    TFEM::begin();
    // Предварительное вычисление компонент нагрузки
    calcLoad(load);
//    calcConcentratedLoad(load);
//    calcSurfaceLoad(load);
//    calcPressureLoad(load);
//    calcVolumeLoad(load);

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
        throw ErrorCode::EAbort;
    isProcessStarted = false;
    isProcessCalculated = true;

    TFEM::end(hour, min, sec);
    // Сохраняем информацию о времени расчета
    out << S_MSG_LEAD_TIME << setfill('0') << setw(2) << hour << ':' << setfill('0') << setw(2) << min << ':' << setfill('0') << setw(2) << sec << setfill(' ');
    TFEM::notes->push_back(out.str());
    cout << out.str() << endl;

    printResultSummary();
}
//-------------------------------------------------------------
//                  Ансамблирование ЛМЖ к ГМЖ
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::ansambleLocalMatrix(TFE *fe, unsigned i)
{
    unsigned freedom = mesh->getFreedom(),
             size = fe->getSize() * fe->getFreedom();


    // Учет матрицы
    for (unsigned l = 0; l < size; l++)
    {
        for (unsigned k = l; k < size; k++)
        {
            solver.addStiffness(fe->getStiffnessMatrix(l, k), mesh->getFE(i, l / freedom) * freedom + l % freedom, mesh->getFE(i, k / freedom) * freedom + k % freedom);
            if (l not_eq k)
                solver.addStiffness(fe->getStiffnessMatrix(l, k), mesh->getFE(i, k / freedom) * freedom + k % freedom, mesh->getFE(i, l / freedom) * freedom + l % freedom);
        }
        solver.addLoad(fe->getLoad(l), mesh->getFE(i, l / freedom) * freedom + l % freedom);
    }
}
//-----------------------------------------------------------------------------------------
//                 Предварительное вычисление значений узловой нагрузки
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcLoad(vector<double> &load, double t)
{
    calcConcentratedLoad(load, t);
    calcSurfaceLoad(load, t);
    calcVolumeLoad(load, t);
    calcPressureLoad(load, t);
}
//-----------------------------------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::saveResult(matrix<double> &res, bool isAdd)
{
    if (not isAdd)
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
template<class T> void TFEMStatic<T>::genResults(vector<double> &u, bool isAdd)
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
template<class T> bool TFEMStatic<T>::checkBE(unsigned index, TParameter &p)
{
    vector<double> coord;

    for (unsigned i = 0; i < mesh->getBaseSizeBE(); i++)
    {
        mesh->getCoordVertex(mesh->getBE(index, i), coord);
        if (not params.getPredicateValue(p, coord))
            return false;
    }
    return true;
}
//-------------------------------------------------------------------------------
// Проверка соответствия конечного элемента предикату отбора (всех его вершин)
//-------------------------------------------------------------------------------
template<class T> bool TFEMStatic<T>::checkFE(unsigned index, TParameter &p)
{
    vector<double> coord;

    for (unsigned i = 0; i < mesh->getBaseSizeFE(); i++)
    {
        mesh->getCoordVertex(mesh->getFE(index, i), coord);
        if (not params.getPredicateValue(p, coord))
            return false;
    }
    return true;
}
//-----------------------------------------------------------------------------------------
//                Осреднение результатов расчета деформаций, напряжений, ...
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::avgResults(matrix<double> &result, vector<int> &counter)
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
    unsigned step = TFEM::mesh->getNumFE() / numThread;
    ErrorCode error = ErrorCode::Undefined;
    vector<thread> thr(numThread);

    msg->setProcess((isStatic) ? ProcessCode::GeneratingStaticMatrix : ProcessCode::GeneratingDynamicMatrix, 1, TFEM::mesh->getNumFE());
    for (int i = 0; i < numThread; i++)
        thr[i] = thread(&TFEMStatic<T>::getMatrix, this, i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, isStatic, ref(error));
    for_each (thr.begin(), thr.end(), [](auto &tr) { tr.join(); });
    if (error not_eq ErrorCode::Undefined)
        throw error;
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
//             Вычисление локальных МЖ, ММ и МД для заданного интервала узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getMatrix(unsigned begin, unsigned end, bool isStatic, ErrorCode &error)
{
    TRFE fe(TFEM::mesh->getTypeFE());

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isProcessAborted)
                throw ErrorCode::EAbort;
            // Настройка КЭ
            setupFE(fe.getFE(), i);
            // Формирование локальной МЖ (ЛМЖ)
            fe.getFE()->generate(isStatic);
            // Ансамблирование ЛМЖ к ГМЖ
            ansambleLocalMatrix(fe.getFE(), i);
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-------------------------------------------------------------
//                  Учет граничных условий
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcBoundaryCondition(void)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumVertex() / numThread;
    vector<thread> thr(numThread);

    if (params.plist.findParameter(ParamType::BoundaryCondition))
    {
        msg->setProcess(ProcessCode::UsingBoundaryCondition, 1, TFEM::mesh->getNumVertex());
        for (int i = 0; i < numThread; i++)
            thr[i] = thread(&TFEMStatic<T>::getBoundaryCondition, this, i * step, (i == numThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step, ref(error));
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (error not_eq ErrorCode::Undefined)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//             Вычисление граничных условий для заданного интервала узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getBoundaryCondition(unsigned begin, unsigned end, ErrorCode &error)
{
    double val;
    unsigned freedom = TFEM::mesh->getFreedom();
    vector<double> coord;
    Direction direct;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            for (auto it: params.plist)
                if (it.getType() == ParamType::BoundaryCondition and ((direct = it.getDirect()) not_eq Direction::Undefined))
                {
                    if (isProcessAborted)
                        throw ErrorCode::EAbort;
                    mesh->getCoordVertex(i, coord);
                    coord.push_back(0.0); // t = 0
                    if (params.getPredicateValue(it, coord))
                    {
                        val = params.getExpressionValue(it, coord);
                        if (contains(direct, Direction::X))
                            solver.setBoundaryCondition(i * freedom + 0, val);
                        if (contains(direct, Direction::Y))
                            solver.setBoundaryCondition(i * freedom + 1, val);
                        if (contains(direct, Direction::Z))
                            solver.setBoundaryCondition(i * freedom + 2, val);
                    }
                }
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений узловой сосредоточенной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcConcentratedLoad(vector<double> &load, double t)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumVertex() / numThread;
    vector<thread> thr(numThread);

    if (params.plist.findParameter(ParamType::ConcentratedLoad))
    {
        msg->setProcess(ProcessCode::GeneratingConcentratedLoad, 1, TFEM::mesh->getNumVertex());
        for (int i = 0; i < numThread; i++)
            thr[i] = thread(&TFEMStatic<T>::getConcentratedLoad, this, ref(load), i * step, (i == numThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step, t, ref(error));
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (error not_eq ErrorCode::Undefined)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//           Вычисление сосредоточенной нагрузки для заданного диапазона узлов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getConcentratedLoad(vector<double> &load, unsigned begin, unsigned end, double t, ErrorCode &error)
{
    Direction direct;
    double val;
    vector<double> coord;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            for (auto it : params.plist)
                if (it.getType() == ParamType::ConcentratedLoad and ((direct = it.getDirect()) not_eq Direction::Undefined))
                {
                    if (isProcessAborted)
                        throw ErrorCode::EAbort;
                    mesh->getCoordVertex(i, coord);
                    coord.push_back(t);

                    if (not params.getPredicateValue(it, coord))
                        continue;
                    val = params.getExpressionValue(it, coord);
                    if (contains(direct, Direction::X)) // X
                        load[i * mesh->getFreedom() + 0] += val;
                    if (contains(direct, Direction::Y)) // Y
                        load[i * mesh->getFreedom() + 1] += val;
                    if (contains(direct, Direction::Z)) // Z (W - для пластины обрабатывается особо)
                        load[i * mesh->getFreedom() + ((mesh->isPlate()) ? 0 : 2)] += val;
                }
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений поверхностной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcSurfaceLoad(vector<double> &load, double t)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumBE() / numThread;
    vector<thread> thr(numThread);

    if (params.plist.findParameter(ParamType::SurfaceLoad))
    {
        msg->setProcess(ProcessCode::GeneratingSurfaceLoad, 1, TFEM::mesh->getNumBE());
        for (int i = 0; i < numThread; i++)
            thr[i] = thread(&TFEMStatic<T>::getSurfaceLoad, this, ref(load), i * step, (i == numThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t, ref(error));
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (error not_eq ErrorCode::Undefined)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//     Вычисление поверхностной нагрузки для заданного диапазона граничных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getSurfaceLoad(vector<double> &load, unsigned begin, unsigned end, double t, ErrorCode &error)
{
    Direction direct;
    double val;
    vector<double> share,
                   coord;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            for (auto it:  params.plist)
                if (it.getType() == ParamType::SurfaceLoad and ((direct = it.getDirect()) not_eq Direction::Undefined))
                {
                    if (isProcessAborted)
                        throw ErrorCode::EAbort;
                    // Проверка, все ли узлы ГЭ удвлетворяют предикату
                    if (not checkBE(i, it))
                        continue;
                    share = mesh->surfaceLoadShare() * mesh->beVolume(i);
                    mesh->getCenterBE(i, coord);
                    coord.push_back(t);
                    val = params.getExpressionValue(it, coord);
                    for (unsigned k = 0; k < mesh->getSizeBE(); k++)
                    {
                        if (contains(direct, Direction::X)) // X
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                        if (contains(direct, Direction::Y)) // Y
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 1] += val * share[k];
                        if (contains(direct, Direction::Z)) // Z или W - для пластины
                            load[mesh->getBE(i, k) * mesh->getFreedom() + ((mesh->isPlate()) ? 0 : 2)] += val * share[k];
                    }
                }
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-----------------------------------------------------------------------------------------
//                Предварительное вычисление значений нагрузки давлением
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcPressureLoad(vector<double> &load, double t)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumBE() / numThread;
    vector<thread> thr(numThread);

    if (params.plist.findParameter(ParamType::Pressure_load))
    {
        msg->setProcess(ProcessCode::GeneratingPressureLoad, 1, TFEM::mesh->getNumBE());
        for (int i = 0; i < numThread; i++)
            thr[i] = thread(&TFEMStatic<T>::getPressureLoad, this, ref(load), i * step, (i == numThread - 1) ? TFEM::mesh->getNumBE() : (i + 1) * step, t, ref(error));
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (error not_eq ErrorCode::Undefined)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//      Вычисление нагрузки давлением для заданного диапазона граничных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getPressureLoad(vector<double> &load, unsigned begin, unsigned end, double t, ErrorCode &error)
{
    double val;
    vector<double> share,
                   coord,
                   v(3);

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            for (auto it: params.plist)
                if (it.getType() == ParamType::Pressure_load)
                {
                    if (isProcessAborted)
                        throw ErrorCode::EAbort;
                    // Проверка, все ли узлы ГЭ удвлетворяют предикату
                    if (not checkBE(i, it))
                        continue;
                    // Вычисление нагрузки
                    share = mesh->surfaceLoadShare() * mesh->beVolume(i);
                    mesh->getCenterBE(i, coord);
                    coord.push_back(t);
                    val = params.getExpressionValue(it, coord);
                    // Вычисление нормали к ГЭ
                    mesh->normal(i, v);

                    for (unsigned k = 0; k < mesh->getSizeBE(); k++)
                    {
                        if (not mesh->isPlate())
                        {
                            // X
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k] * v[0];
                            // Y
                            if (mesh->getFreedom() > 1)
                                load[mesh->getBE(i, k) * mesh->getFreedom() + 1] += val * share[k] * v[1];
                            // Z или W - для пластины
                            if (mesh->getFreedom() > 2)
                                load[mesh->getBE(i, k) * mesh->getFreedom() + ((mesh->isPlate()) ? 0 : 2)] += val * share[k] * v[2];
                        }
                        else
                            load[mesh->getBE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                    }
                }
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-----------------------------------------------------------------------------------------
//         Предварительное вычисление значений объемной нагрузки
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcVolumeLoad(vector<double> &load, double t)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumFE() / numThread;
    vector<thread> thr(numThread);

    if (params.plist.findParameter(ParamType::VolumeLoad))
    {
        msg->setProcess(ProcessCode::GeneratingVolumeLoad, 1, mesh->getNumFE());
        for (int i = 0; i < numThread; i++)
            thr[i] = thread(&TFEMStatic<T>::getVolumeLoad, this, ref(load), i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, t, ref(error));
        for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
        if (error not_eq ErrorCode::Undefined)
            throw error;
        msg->stopProcess();
    }
}
//-----------------------------------------------------------------------------------------
//         Вычисление объемной нагрузки для заданного интервала конечных элементов
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getVolumeLoad(vector<double> &load, unsigned begin, unsigned end, double t, ErrorCode &error)
{
    Direction direct;
    double val;
    vector<double> share,
                   coord;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isProcessAborted)
                throw ErrorCode::EAbort;
            for (auto it: params.plist)
                if (it.getType() == ParamType::VolumeLoad and ((direct = it.getDirect()) not_eq Direction::Undefined))
                {
                    // Проверка, все ли узлы КЭ удовлетворяют предикату
                    if (not checkFE(i, it))
                        continue;
                    share = mesh->volumeLoadShare() * mesh->feVolume(i);
                    mesh->getCenterFE(i, coord);
                    coord.push_back(t);
                    val = params.getExpressionValue(it, coord);
                    for (unsigned k = 0; k < mesh->getSizeFE(); k++)
                    {
                        if (contains(direct, Direction::X)) // X или W - для пластины
                            load[mesh->getFE(i, k) * mesh->getFreedom() + 0] += val * share[k];
                        if (contains(direct, Direction::Y)) // Y
                            load[mesh->getFE(i, k) * mesh->getFreedom() + 1] += val * share[k];
                        if (contains(direct, Direction::Z)) // Z
                            load[mesh->getFE(i, k) * mesh->getFreedom() + ((mesh->isPlate()) ? 0 : 2)] += val * share[k];
                    }
                }
        }
    }
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-----------------------------------------------------------------------------------------
//                       Вычисление интенсивности напряжений
//-----------------------------------------------------------------------------------------
template<class T> double TFEMStatic<T>::calcStressIntensity(TResultList &res, vector<double> &si)
{
    unsigned step = TFEM::mesh->getNumVertex() / numThread;
    vector<thread> thr(numThread);

    for (int i = 0; i < numThread; i++)
        thr[i] = thread(&TFEMStatic<T>::getStressIntensity, this, ref(res), ref(si), i * step, (i == numThread - 1) ? TFEM::mesh->getNumVertex() : (i + 1) * step);
    for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
    return *std::max_element(si.begin(), si.end());
}
//--------------------------------------------------------------------------------------------------------------
// Вычисление интенсивности напряжений Si=((Sxx-Syy)^2+(Sxx-Szz)^2+(Syy-Szz)^2+6*(Sxy^2+Sxz^2+Syz^2))^0.5/2^0.5
//--------------------------------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getStressIntensity(TResultList &res, vector<double> &si, unsigned begin, unsigned end)
{
    double m_sqrt1_2 = 0.5 * sqrt(2.0);

    // Вычисление узловых значений интенсивности напряжений
    for (unsigned i = begin; i < end; i++)
        switch (TFEM::mesh->getTypeFE())
        {
            case FEType::fe1d2: // U, Exx, Sxx
                si[i] = m_sqrt1_2 * fabs(res[2].getResults(i));
                break;
            case FEType::fe2d3:
            case FEType::fe2d4:
            case FEType::fe2d6: // U, V, Exx, Eyy, Exy, Sxx, Syy, Sxy
                si[i] = m_sqrt1_2 * sqrt(pow(res[5].getResults(i) - res[6].getResults(i), 2) + 6.0 * (pow(res[7].getResults(i), 2)));
                break;
            case FEType::fe2d3p:
            case FEType::fe2d4p:
            case FEType::fe2d6p: // W, Tx, Ty, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz
            case FEType::fe3d4:
            case FEType::fe3d8:
            case FEType::fe3d10: // U, V, W, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz
                si[i] =  m_sqrt1_2 * sqrt(pow(res[9].getResults(i) - res[10].getResults(i), 2) + pow(res[9].getResults(i) - res[11].getResults(i), 2) +
                             pow(res[11].getResults(i) - res[12].getResults(i), 2) + 6.0 * (pow(res[12].getResults(i), 2) + pow(res[13].getResults(i), 2) + pow(res[14].getResults(i), 2)));
                break;
            case FEType::fe3d3s:
            case FEType::fe3d4s:
            case FEType::fe3d6s: // U, V, W, Tx, Ty, Tz, Exx, Eyy, Ezz, Exy, Exz, Eyz, Sxx, Syy, Szz, Sxy, Sxz, Syz, Ut, Vt, Wt, Utt, Vtt, Wtt
                si[i] = m_sqrt1_2 * sqrt(pow(res[12].getResults(i) - res[13].getResults(i), 2) + pow(res[12].getResults(i) - res[14].getResults(i), 2) +
                             pow(res[13].getResults(i) - res[14].getResults(i), 2) + 6.0 * (pow(res[15].getResults(i), 2) + pow(res[16].getResults(i), 2) + pow(res[17].getResults(i), 2)));
                break;
            default:
                si[i] = 0;
        }
}
//-------------------------------------------------------------
//                  Формирование результатов
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::calcResult(matrix<double> &res, vector<double> &u)
{
    ErrorCode error = ErrorCode::Undefined;
    unsigned step = TFEM::mesh->getNumFE() / numThread;
    vector<int> counter(TFEM::mesh->getNumVertex()); // Счетчик кол-ва вхождения узлов для осреднения результатов
    vector<thread> thr(numThread);

    res.resize(TFEM::params.numResult(TFEM::mesh->getTypeFE()), TFEM::mesh->getNumVertex());
    // Копируем результаты расчета (перемещения)
    for (unsigned i = 0; i < TFEM::mesh->getNumVertex(); i++)
        for (unsigned j = 0; j < TFEM::mesh->getFreedom(); j++)
            res[j][i] = u[i * TFEM::mesh->getFreedom() + j];

    // Вычисляем стандартные результаты по всем КЭ
    msg->setProcess(ProcessCode::GeneratingResult, 1, TFEM::mesh->getNumFE());
    for (int i = 0; i < numThread; i++)
        thr[i] = thread(&TFEMStatic<T>::getFEResult, this, ref(res), ref(u), ref(counter), i * step, (i == numThread - 1) ? TFEM::mesh->getNumFE() : (i + 1) * step, ref(error));
    for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
    if (error not_eq ErrorCode::Undefined)
        throw error;
    // Осредняем результаты
    avgResults(res, counter);
    msg->stopProcess();
}
//-----------------------------------------------------------------------------------------
//             Вычисление стандартных результатов КЭ для заданного интервала
//-----------------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getFEResult(matrix<double> &res, vector<double> &u, vector<int> &counter, unsigned begin, unsigned end, ErrorCode &error)
{
    vector<double> fe_u;
    matrix<double> fe_res(params.numResult(mesh->getTypeFE()), mesh->getSizeFE());
    TRFE fe(TFEM::mesh->getTypeFE());

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isProcessAborted)
                throw ErrorCode::EAbort;
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
    catch (ErrorCode e)
    {
        error = e;
    }
}
//-------------------------------------------------------------
//      Формирование глобального вектора-столбца нагрузки
//-------------------------------------------------------------
template<class T> void TFEMStatic<T>::setLoad(vector<double> &load)
{
    ErrorCode error = ErrorCode::Undefined;
    int size = mesh->getNumVertex() * mesh->getFreedom();
    unsigned step = size / numThread;
    vector<thread> thr(numThread);

    msg->setProcess(ProcessCode::UsingLoad, 1, size);
    for (int i = 0; i < numThread; i++)
        thr[i] = thread(&TFEMStatic<T>::getLoad, this, ref(load), i * step, (i == numThread - 1) ? size : (i + 1) * step, ref(error));
    for_each (thr.begin(), thr.end(), [](auto& tr) { tr.join(); });
    if (error not_eq ErrorCode::Undefined)
        throw error;
    msg->stopProcess();
}
//-------------------------------------------------------------------------------
template<class T> void TFEMStatic<T>::getLoad(vector<double> &load, unsigned begin, unsigned end, ErrorCode &error)
{
    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isProcessAborted)
        {
            error = ErrorCode::EAbort;
            break;
        }
        solver.addLoad(load[i], i);
    }
}
//-----------------------------------------------------------------------------------------
#endif // FEMSTATIC_H
