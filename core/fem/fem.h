#ifndef FEM_H
#define FEM_H

#include <iomanip>
#include "object/params.h"
#include "analyse/analyse.h"
#include "mesh/mesh.h"

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
    TMesh *mesh;
    // Результаты расчета
    TResults *results;
    // Список примечаний (заметок) по процессу решения задачи
    list<string> *notes;
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
    void end(unsigned &hour, unsigned &min, unsigned &sec)
    {
        hour = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count())/3600;
        min = (unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) % 3600)/60;
        sec = unsigned(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()) - hour*3600 - min*60;
    }
    // Настройка КЭ
    virtual void setupFE(TFE &fe, unsigned i)
    {
        double val;
        matrix<double> x;

        // Загрузка координат КЭ
        mesh->getCoordFE(i, x);
        fe.setCoord(x);
        // Загрузка в КЭ модуля упругости
        if ((val = params.getYoungModule(x)) == 0.0)
            throw ErrorCode::EYoungModulus;
        // Загрузка в КЭ коэффициента Пуассона
        fe.setYoungModulus(val);
        if ((val = params.getPoissonRatio(x)) == 0.0 && !mesh->is1D())
            throw ErrorCode::EPoissonRatio;
        fe.setPoissonRatio(val);
        // Загрузка толщины элемента
        if (not mesh->is3D())
        {
            if ((val = params.getThickness(x)) == 0.0)
                throw ErrorCode::EThickness;
            fe.setThickness(val);
        }
        // Загрузка температуры
        fe.setTemperature(params.getTemperature(x));
        // Загрузка альфы
        fe.setAlpha(params.getAlpha(x));
        // Загрузка плотности
        if (params.fType == FEMType::DynamicProblem)
        {
            if ((val = params.getDensity(x)) == 0.0)
                throw ErrorCode::EDensity;
            fe.setDensity(val);
            // Загрузка параметра демпфирования
            if ((val = params.getDamping(x)) == 0.0)
                throw ErrorCode::EDamping;
            fe.setDamping(val);
        }
    }
public:
    TFEM(string n, TMesh *m, TResults *r, list<string> *l = nullptr)
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
    void setParams(TFEMParams &p)
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
