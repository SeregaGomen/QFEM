#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "params.h"
#include "mesh/mesh.h"
#include "fem/fem.h"
#include "analyse/analyse.h"

using namespace std;

class TFEM;

// Класс, описывающий конечно-элементный объект
class TFEMObject
{
private:
    int numThread;                      // Количество потоков, используемых при вычислениях
    string fileName;                    // Имя файла с данными
    string objName;                     // Имя объекта
    bool isProcessStarted;              // Признак того, что процесс запущен
    bool isProcessCalculated;           // ... успешно завершен
    list<string> notes;                 // Вспомогательная информация по решению задачи
    TFEMParams params;                  // Параметры решения задачи
    TResults results;                   // Результаты расчета
    TFEM *fem;                          // Реализация МКЭ
    TMesh mesh;                         // КЭ сетка
    template<typename SOLVER>
    TFEM *createProblem(void);          // Создание задачи с заданным решателем
public:
    TFEMObject(void)
    {
        fem = nullptr;
        isProcessStarted = isProcessCalculated = false;
        numThread = thread::hardware_concurrency() - 1;
    }
   ~TFEMObject(void)
    {
        clear();
    }
    void clear(void);
    void setProcessCalculated(bool p)
    {
        isProcessCalculated = p;
    }
    bool setMeshFile(string);
    TFEMParams& getParams(void)
    {
        return params;
    }
    TMesh& getMesh(void)
    {
        return mesh;
    }
    bool isCalculated(void)
    {
        return isProcessCalculated;
    }
    bool isStarted(void)
    {
        return isProcessStarted;
    }
    void stop(void)
    {
        if (fem)
            fem->breakProcess();
    }
    void setTaskParam(FEMType);
    bool start(void);
    list<string>& getNotes(void)
    {
        return notes;
    }
    TResults& getResult(void)
    {
        return results;
    }
    TResult& getResult(unsigned i)
    {
        return results[i];
    }
    FEType getFEType(void)
    {
        return mesh.getTypeFE();
    }
    bool saveResult(string);
    bool loadResult(string);
    void printResult(string);
    void setFileName(string n)
    {
        cout << endl << n << endl;
        fileName = n;
        objName = n.substr(n.find_last_of("/\\") + 1,n.find_last_of(".") - n.find_last_of("/\\") - 1);
    }
    string getFileName(void)
    {
        return fileName;
    }
    string getObjectName(void)
    {
        return objName;
    }
    void setT0(unsigned p)
    {
        params.t0 = p;
    }
    void setT1(unsigned p)
    {
        params.t1 = p;
    }
    void setTh(unsigned p)
    {
        params.th = p;
    }
    void setEps(double p)
    {
        params.eps = p;
    }
    void setWidth(int p)
    {
        params.width = p;
    }
    void setPrecision(int p)
    {
        params.precision = p;
    }
    void setLoadStep(double p)
    {
        params.loadStep = p;
    }
    // Толщина
    void addThickness(double v, string p)
    {
        params.plist.addThickness(v, p);
    }
    void addThickness(double v)
    {
        params.plist.addThickness(v, "");
    }
    void addThickness(string e, string p)
    {
        params.plist.addThickness(e, p);
    }
    void addThickness(string e)
    {
        params.plist.addThickness(e, "");
    }
    void addThickness(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addThickness(v, p);
    }
    void addThickness(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addThickness(e, p);
    }
    void addThickness(function<double (double, double, double, double)> e)
    {
        params.plist.addThickness(e, nullptr);
    }
    // Температура
    void addTemperature(double v, string p)
    {
        params.plist.addTemperature(v, p);
    }
    void addTemperature(double v)
    {
        params.plist.addTemperature(v, "");
    }
    void addTemperature(string e, string p)
    {
        params.plist.addTemperature(e, p);
    }
    void addTemperature(string e)
    {
        params.plist.addTemperature(e, "");
    }
    void addTemperature(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addTemperature(v, p);
    }
    void addTemperature(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addTemperature(e, p);
    }
    void addTemperature(function<double (double, double, double, double)> e)
    {
        params.plist.addTemperature(e, nullptr);
    }
    // Альфа
    void addAlpha(double v, string p)
    {
        params.plist.addAlpha(v, p);
    }
    void addAlpha(double v)
    {
        params.plist.addAlpha(v, "");
    }
    void addAlpha(string e, string p)
    {
        params.plist.addAlpha(e, p);
    }
    void addAlpha(string e)
    {
        params.plist.addAlpha(e, "");
    }
    void addAlpha(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addAlpha(v, p);
    }
    void addAlpha(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addAlpha(e, p);
    }
    void addAlpha(function<double (double, double, double, double)> e)
    {
        params.plist.addAlpha(e, nullptr);
    }
    // Плотность
    void addDensity(double v, string p)
    {
        params.plist.addDensity(v, p);
    }
    void addDensity(double v)
    {
        params.plist.addDensity(v, "");
    }
    void addDensity(string e, string p)
    {
        params.plist.addDensity(e, p);
    }
    void addDensity(string e)
    {
        params.plist.addDensity(e, "");
    }
    void addDensity(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addDensity(v, p);
    }
    void addDensity(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addDensity(e, p);
    }
    void addDensity(function<double (double, double, double, double)> e)
    {
        params.plist.addDensity(e, nullptr);
    }
    // Параметр демпфирования
    void addDamping(double v, string p)
    {
        params.plist.addDamping(v, p);
    }
    void addDamping(double v)
    {
        params.plist.addDamping(v, "");
    }
    void addDamping(string e, string p)
    {
        params.plist.addDamping(e, p);
    }
    void addDamping(string e)
    {
        params.plist.addDamping(e, "");
    }
    void addDamping(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addDamping(v, p);
    }
    void addDamping(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addDamping(e, p);
    }
    void addDamping(function<double (double, double, double, double)> e)
    {
        params.plist.addDamping(e, nullptr);
    }
    // Модуль Юнга
    void addYoungModulus(double v, string p)
    {
        params.plist.addYoungModulus(v, p);
    }
    void addYoungModulus(double v)
    {
        params.plist.addYoungModulus(v, "");
    }
    void addYoungModulus(string e, string p)
    {
        params.plist.addYoungModulus(e, p);
    }
    void addYoungModulus(string e)
    {
        params.plist.addYoungModulus(e, "");
    }
    void addYoungModulus(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addYoungModulus(v, p);
    }
    void addYoungModulus(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addYoungModulus(e, p);
    }
    void addYoungModulus(function<double (double, double, double, double)> e)
    {
        params.plist.addYoungModulus(e, nullptr);
    }
    // Коэффициент Пуассона
    void addPoissonRatio(double v, string p)
    {
        params.plist.addPoissonRatio(v, p);
    }
    void addPoissonRatio(double v)
    {
        params.plist.addPoissonRatio(v, "");
    }
    void addPoissonRatio(string e, string p)
    {
        params.plist.addPoissonRatio(e, p);
    }
    void addPoissonRatio(string e)
    {
        params.plist.addPoissonRatio(e, "");
    }
    void addPoissonRatio(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addPoissonRatio(e, p);
    }
    void addPoissonRatio(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addPoissonRatio(v, p);
    }
    void addPoissonRatio(function<double (double, double, double, double)> e)
    {
        params.plist.addPoissonRatio(e, nullptr);
    }
    // Граничные условия
    void addBoundaryCondition(Direction dir, double v, string p)
    {
        params.plist.addBoundaryCondition(v, p, dir);
    }
    void addBoundaryCondition(Direction dir, string e, string p)
    {
        params.plist.addBoundaryCondition(e, p, dir);
    }
    void addBoundaryCondition(Direction dir, double v, function<double (double, double, double, double)> p)
    {
        params.plist.addBoundaryCondition(v, p, dir);
    }
    void addBoundaryCondition(Direction dir, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addBoundaryCondition(e, p, dir);
    }
    // Начальные условия
    void addInitialCondition(InitialCondition ic, double v)
    {
        params.plist.addInitialCondition(v, ic);
    }
    void addInitialCondition(InitialCondition ic, string e)
    {
        params.plist.addInitialCondition(e, ic);
    }
    void addInitialCondition(InitialCondition ic, function<double (double, double, double, double)> e)
    {
        params.plist.addInitialCondition(e, ic);
    }
    // Сосредоточенная нагрузка
    void addConcentratedLoad(Direction dir, double v, string p)
    {
        params.plist.addConcentratedLoad(v, p, dir);
    }
    void addConcentratedLoad(Direction dir, double v)
    {
        params.plist.addConcentratedLoad(v, "", dir);
    }
    void addConcentratedLoad(Direction dir, string e, string p)
    {
        params.plist.addConcentratedLoad(e, p, dir);
    }
    void addConcentratedLoad(Direction dir, string e)
    {
        params.plist.addConcentratedLoad(e, "", dir);
    }
    void addConcentratedLoad(Direction dir, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addConcentratedLoad(e, p, dir);
    }
    void addConcentratedLoad(Direction dir, double v, function<double (double, double, double, double)> p)
    {
        params.plist.addConcentratedLoad(v, p, dir);
    }
    void addConcentratedLoad(Direction dir, function<double (double, double, double, double)> e)
    {
        params.plist.addConcentratedLoad(e, nullptr, dir);
    }
    // Объемная нагрузка
    void addVolumeLoad(Direction dir, double v, string p)
    {
        params.plist.addVolumeLoad(v, p, dir);
    }
    void addVolumeLoad(Direction dir, double v)
    {
        params.plist.addVolumeLoad(v, "", dir);
    }
    void addVolumeLoad(Direction dir, string e, string p)
    {
        params.plist.addVolumeLoad(e, p, dir);
    }
    void addVolumeLoad(Direction dir, string e)
    {
        params.plist.addVolumeLoad(e, "", dir);
    }
    void addVolumeLoad(Direction dir, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addVolumeLoad(e, p, dir);
    }
    void addVolumeLoad(Direction dir, double v, function<double (double, double, double, double)> p)
    {
        params.plist.addVolumeLoad(v, p, dir);
    }
    void addVolumeLoad(Direction dir, function<double (double, double, double, double)> e)
    {
        params.plist.addVolumeLoad(e, nullptr, dir);
    }
    // Поверхностная нагрузка
    void addSurfaceLoad(Direction dir, double v, string p)
    {
        params.plist.addSurfaceLoad(v, p, dir);
    }
    void addSurfaceLoad(Direction dir, double v)
    {
        params.plist.addSurfaceLoad(v, "", dir);
    }
    void addSurfaceLoad(Direction dir, string e, string p)
    {
        params.plist.addSurfaceLoad(e, p, dir);
    }
    void addSurfaceLoad(Direction dir, string e)
    {
        params.plist.addSurfaceLoad(e, "", dir);
    }
    void addSurfaceLoad(Direction dir, double v, function<double (double, double, double, double)> p)
    {
        params.plist.addSurfaceLoad(v, p, dir);
    }
    void addSurfaceLoad(Direction dir, function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addSurfaceLoad(e, p, dir);
    }
    void addSurfaceLoad(Direction dir, function<double (double, double, double, double)> e)
    {
        params.plist.addSurfaceLoad(e, nullptr, dir);
    }
    // Давление
    void addPressureLoad(double v, string p)
    {
        params.plist.addPressureLoad(v, p);
    }
    void addPressureLoad(double v)
    {
        params.plist.addPressureLoad(v, "");
    }
    void addPressureLoad(string e, string p)
    {
        params.plist.addPressureLoad(e, p);
    }
    void addPressureLoad(string e)
    {
        params.plist.addPressureLoad(e, "");
    }
    void addPressureLoad(double v, function<double (double, double, double, double)> p)
    {
        params.plist.addPressureLoad(v, p);
    }
    void addPressureLoad(function<double (double, double, double, double)> e, function<double (double, double, double, double)> p)
    {
        params.plist.addPressureLoad(e, p);
    }
    void addPressureLoad(function<double (double, double, double, double)> e)
    {
        params.plist.addPressureLoad(e, nullptr);
    }
    void addVariable(string name, double val)
    {
        params.variables[name] = val;
    }
    // Диаграмма деформирования
    void addStressStrainCurve(matrix<double> &ssc, function<double (double, double, double, double)> p)
    {
        params.plist.addStressStrainCurve(ssc, p);
    }
    void addStressStrainCurve(matrix<double> &ssc, string p)
    {
        params.plist.addStressStrainCurve(ssc, p);
    }
    void addStressStrainCurve(matrix<double> &ssc)
    {
        params.plist.addStressStrainCurve(ssc, "");
    }
    void setFunName(vector<string>& fn)
    {
        params.names = fn;
    }
    void setPlasticityMethod(PlasticityMethod p)
    {
        params.pMethod = p;
    }
    void setNumThread(int n)
    {
        numThread = n;
    }
    void setLanguage(int);
    string stdTxtResName(void)
    {
        return getFileName().substr(0, getFileName().find_last_of(".")) + ".txt";
    }
    string stdResName(void)
    {
        return getFileName().substr(0, getFileName().find_last_of(".")) + ".res";
    }
};

#endif // OBJECT_H
