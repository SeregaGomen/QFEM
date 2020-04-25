#ifndef FEM_H
#define FEM_H

#include <iomanip>
#include <vector>
#include <map>
#include "parser/parser.h"
#include "object/params.h"
#include "object/plist.h"
#include "analyse/analyse.h"
#include "mesh/mesh.h"
#include "fe/fe1d.h"
#include "fe/fe2d.h"
#include "fe/fe3d.h"
#include "fe/fe2dp.h"
#include "fe/fe3ds.h"


//-------------------------------------------------------------
//            Класс-обертка для TFE с поддержкой RAII
//-------------------------------------------------------------
class TRFE
{
private:
    TFE* fe;
    void createFE(FEType type)
    {
        // Создание конечного элемента
        switch (type)
        {
            case FEType::fe1d2:
                fe = new TFE1D<TShape1D2>();
                break;
            case FEType::fe2d3:
                fe = new TFE2D<TShape2D3>();
                break;
            case FEType::fe2d4:
                fe = new TFE2D<TShape2D4>();
                break;
            case FEType::fe2d6:
                fe = new TFE2D<TShape2D6>();
                break;
            case FEType::fe2d3p:
                fe = new TFE2DP<TShape2D3>();
                break;
            case FEType::fe2d4p:
                fe = new TFE2DP<TShape2D4>();
                break;
            case FEType::fe2d6p:
                fe = new TFE2DP<TShape2D6>();
                break;
            case FEType::fe3d4:
                fe = new TFE3D<TShape3D4>();
                break;
            case FEType::fe3d8:
                fe = new TFE3D<TShape3D8>();
                break;
            case FEType::fe3d10:
                fe = new TFE3D<TShape3D10>();
                break;
            case FEType::fe3d3s:
                fe = new TFE3DS<TShape2D3>();
                break;
            case FEType::fe3d4s:
                fe = new TFE3DS<TShape2D4>();
                break;
            case FEType::fe3d6s:
                fe = new TFE3DS<TShape2D6>();
                break;
            default:
                //throw UNKNOWN_FE_ERR;
                fe = nullptr;
        }
    }
public:
    TRFE(FEType type)
    {
        createFE(type);
    }
    ~TRFE(void)
    {
        if (fe)
            delete fe;
    }
    TFE* getFE(void)
    {
        return fe;
    }
};


//-------------------------------------------------------------
//  Абстрактный класс, реализующий метод конечных элементов
//-------------------------------------------------------------
class TFEM
{
private:
    chrono::system_clock::time_point timer;
protected:
    // Количество потоков, используемых для расчетов
    int numThread;
    // Назание объекта расчета
    string objName;
    // Параметры расчета
    TFEMParams params;
    // КЭ-сетка
    TMesh* mesh;
    // Результаты расчета
    TResultList* results;
    // Список примечаний (заметок) по процессу решения задачи
    list<string>* notes;
    // Признак того, что процесс расчета запущен
    bool isProcessStarted;
    // Признак того, что процесс расчета прерван
    bool isProcessAborted;
    // Признак того, что процесс расчета успешно завершен
    bool isProcessCalculated;
    // Фиксация времени начала расчета
    void begin(void)
    {
        timer = chrono::system_clock::now();
    }
    // Получение времени окончания расчета
    void end(unsigned& hour, unsigned& min, unsigned& sec)
    {
        hour = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) / 3600;
        min = (unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) % 3600) / 60;
        sec = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) - hour * 3600 - min * 60;
    }
    // Настройка КЭ
    virtual void setupFE(TFE *fe, unsigned i)
    {
        double val;
        matrix<double> x;
        vector<double> cx;

        // Загрузка координат КЭ
        mesh->getCoordFE(i, x);
        fe->setCoord(x);
        // Получение координат центра КЭ
        mesh->getCenterFE(i, cx);
        // Загрузка в КЭ модуля упругости
        if ((val = params.getYoungModule(cx)) == 0.0)
            throw ErrorCode::EYoungModulus;
        // Загрузка в КЭ коэффициента Пуассона
        fe->setYoungModulus(val);
        if ((val = params.getPoissonRatio(cx)) == 0.0 and not mesh->is1D())
            throw ErrorCode::EPoissonRatio;
        fe->setPoissonRatio(val);
        // Загрузка толщины элемента
        if (not mesh->is3D())
        {
            if ((val = params.getThickness(cx)) == 0.0)
                throw ErrorCode::EThickness;
            fe->setThickness(val);
        }
        // Загрузка температуры
        fe->setTemperature(params.getTemperature(cx));
        // Загрузка альфы
        fe->setAlpha(params.getAlpha(cx));
        // Загрузка плотности
        if (params.fType == FEMType::DynamicProblem)
        {
            if ((val = params.getDensity(cx)) == 0.0)
                throw ErrorCode::EDensity;
            fe->setDensity(val);
            // Загрузка параметра демпфирования
            if ((val = params.getDamping(cx)) == 0.0)
                throw ErrorCode::EDamping;
            fe->setDamping(val);
        }
    }
public:
    TFEM(string n, TMesh* m, TResultList* r, list<string>* l = nullptr)
    {
        objName = n;
        mesh = m;
        results = r;
        notes = l;
        isProcessStarted = isProcessAborted = isProcessCalculated = false;
        numThread = thread::hardware_concurrency() - 1;
    }
    virtual ~TFEM(void) {}
    virtual void startProcess(void) = 0;
    void breakProcess(void)
    {
        isProcessAborted = true;
    }
    void setParams(TFEMParams& p)
    {
        params = p;
    }
    void setNumThread(int n)
    {
        numThread = n;
    }
    // Вывод рез-тов по каждой функции на экран
    void printResultSummary(double t = 0)
    {
        cout << "---------------------------------------------------------------------" << endl;
        cout.setf(ios::left);
        cout << setw(10) << ' ' << setw(params.width) << "\tmin" << ' ' << setw(params.width) << "\tmax" << endl;
        cout.flags (ios::floatfield | ios::scientific | ios::showpos);
        for (unsigned i = 0; i < results->size(); i++)
            if ((*results)[i].getTime() == t)
                cout << setw(10) << (*results)[i].getName() << '\t' << std::scientific << setw(params.width) <<
                        setprecision(params.precision) << (*results)[i].min() << '\t' << setw(params.width) << (*results)[i].max() << endl;
        cout.unsetf(ios::showpos);
        cout << "---------------------------------------------------------------------" << endl;
    }
    bool isCalculated(void)
    {
        return isProcessCalculated;
    }
};

#endif // QFEM_H
