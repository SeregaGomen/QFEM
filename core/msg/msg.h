#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <thread>

extern int langCode;

#define S_ERR                           ((langCode) ? "Ошибка" : "Error")
#define S_ERR_OPEN_FILE                 ((langCode) ? "Ошибка открытия файла" : "Error opening file")
#define S_ERR_WRITE_FILE                ((langCode) ? "Ошибка записи файла" : "Error writing file")
#define S_ERR_FORMAT_FILE               ((langCode) ? "Некорректный формат файла" : "Incorrect file format")
#define S_ERR_READ_FILE                 ((langCode) ? "Ошибка чтения файла" : "Error reading file")
#define S_ERR_INCORRECT_FE              ((langCode) ? "Вырожденный конечный элемент" : "Invalid finite element")
#define S_ERR_UNKNOWN_FILE              ((langCode) ? "Некорректный тип файла" : "Invalid file type")
#define S_ERR_UNKNOWN_FE                ((langCode) ? "Некорректный тип конечного элемента" : "Invalid finite element")
#define S_ERR_SYNTAX                    ((langCode) ? "Синтаксическая ошибка" : "Syntax error")
#define S_ERR_CRAMP                     ((langCode) ? "Несбалансированные скобки" : "Unbalanced brackets")
#define S_ERR_NAME                      ((langCode) ? "Некорректное имя переменной" : "Incorrect variable name")
#define S_ERR_UNDEF_VARIABLE            ((langCode) ? "Неопределенная переменная" : "Undefined variable")
#define S_ERR_DEF_VARIABLE              ((langCode) ? "Переопределение переменной" : "Redefining the variable")
#define S_ERR_EQUATION_NOT_SOLVED       ((langCode) ? "Система уравнений не решена" : "The system of equations is not solved")
#define S_ERR_ABORT                     ((langCode) ? "Процесс остановлен пользователем" : "The process was stopped by user")
#define S_ERR_MEMORY                    ((langCode) ? "Ошибка выделения памяти" : "Memory allocation error")
#define S_ERR_NONLINEAR                 ((langCode) ? "Некорректно задана диаграмма деформирования" : "Invalid stress-strain curve")
#define S_ERR_YOUNG_MODULUS             ((langCode) ? "Некорректно задано значение модуля Юнга" : "Invalid Young modulus")
#define S_ERR_POISSON_RATIO             ((langCode) ? "Некорректно задано значение коэффициента Пуассона" : "Invalid Poisson ratio")
#define S_ERR_THICKNESS                 ((langCode) ? "Некорректно задано значение толщины элемента" : "Invalid FE thickness")
#define S_ERR_TEMPERATURE               ((langCode) ? "Некорректно задано значение температуры" : "Invalid temperature")
#define S_ERR_ALPHA                     ((langCode) ? "Некорректно задано значение коэффициента температурного расширения" : "Invalid alpha")
#define S_ERR_DENSITY                   ((langCode) ? "Некорректно задано значение плотности" : "Invalid density")
#define S_ERR_DAMPING                   ((langCode) ? "Некорректно задано значение параметра демпфирования" : "Invalid damping")
#define S_ERR_INDEX                     ((langCode) ? "Некорректно задано значение индекса" : "Invalid index")

#define S_MSG_START                     ((langCode) ? "******************************* Старт *******************************" : "******************************* Start *******************************")
#define S_MSG_STOP                      ((langCode) ? "******************************* Стоп  *******************************" : "******************************* Stop  *******************************")
#define S_MSG_CALC_CONCENTRATED_LOAD    ((langCode) ? "Расчет сосредоточенных нагрузок" : "Calculation of concentrated loads")
#define S_MSG_CALC_SURFACE_LOAD         ((langCode) ? "Расчет поверхностных нагрузок" : "Calculation of surface loads")
#define S_MSG_CALC_VOLUME_LOAD          ((langCode) ? "Расчет объемных нагрузок" : "Calculation of volume loads")
#define S_MSG_CALC_PRESSURE_LOAD        ((langCode) ? "Расчет нагрузок давления" : "Calculation of pressure loads")
#define S_MSG_CREATE_LOAD               ((langCode) ? "Формирование вектора-столбца нагрузки" : "Building the load vector-column")
#define S_MSG_FILE_NAME                 ((langCode) ? "Файл данных: " : "Data file: ")
#define S_MSG_FE_TYPE                   ((langCode) ? "Тип КЭ: " : "FE type: ")
#define S_MSG_NO_TYPE                   ((langCode) ? "NOTYPE - неопределенный тип конечного элемента" : "NOTYPE - undefined FE type")
#define S_MSG_FE1D2                     ((langCode) ? "FE1D2 - линейный одномерный элемент (2 узла)" : "FE1D2 - one-dimensional linear element (2 nodes)")
#define S_MSG_FE2D3                     ((langCode) ? "FE2D3 - линейный треугольный элемент (3 узла)" : "FE2D3 - linear triangular element (3 nodes)")
#define S_MSG_FE2D4                     ((langCode) ? "FE2D4 - четырехугольный элемент (4 узла)" : "FE2D4 - quadrilateral element (4 nodes)")
#define S_MSG_FE2D6                     ((langCode) ? "FE2D6 - квадратичный треугольный элемент (6 узлов)" : "FE2D6 - quadratic triangular element (6 nodes)")
#define S_MSG_FE3D4                     ((langCode) ? "FE3D4 - линейный тетраэдр (4 узла)" : "FE3D4 - linear tetrahedron (4 nodes)")
#define S_MSG_FE3D8                     ((langCode) ? "FE3D8 - куб (8 узлов)" : "FE3D8 - cube element (8 nodes)")
#define S_MSG_FE3D10                    ((langCode) ? "FE3D10 - квадратичный тетраэдр (10 узлов)" : "FE3D10 - quadratic tetrahedron (10 nodes)")
#define S_MSG_FE2D3_PLATE               ((langCode) ? "FE2D3P - треугольный элемент пластины (3 узла)" : "FE2D3P - plate triangular element (3 nodes)")
#define S_MSG_FE2D4_PLATE               ((langCode) ? "FE2D4P - четырехугольный элемент пластины (4 узла)" : "FE2D4P - plate quadrilateral element (4 nodes)")
#define S_MSG_FE2D6_PLATE               ((langCode) ? "FE2D6P - треугольный элемент пластины (6 узлов)" : "FE2D6P - plate quadrilateral element (6 nodes)")
#define S_MSG_FE3D3_SHELL               ((langCode) ? "FE3D3S - треугольный оболочечный элемент (3 узла)" : "FE3D3S - shell triangular element (3 nodes)")
#define S_MSG_FE3D4_SHELL               ((langCode) ? "FE3D4S - четырехугольный оболочечный элемент (4 узла)" : "FE3D4S - shell quadrilateral element (4 nodes)")
#define S_MSG_FE3D6_SHELL               ((langCode) ? "FE3D6S - треугольный оболочечный элемент (6 узлов)" : "FE3D6S - shell triangular element (6 nodes)")
#define S_MSG_NUM_NODES                 ((langCode) ? "Количество узлов: " : "Number of nodes: ")
#define S_MSG_NUM_FE                    ((langCode) ? "Количество конечных элементов: " : "Number of finite elements: ")
#define S_MSG_FE_STATIC_PROCESS         ((langCode) ? "Формирование глобальной матрицы жесткости" : "Building a global stiffness matrix")
#define S_MSG_FE_DYNAMIC_PROCESS        ((langCode) ? "Формирование глобальных матриц" : "Building of the global matrix")
#define S_MSG_BOUNDARY_PROCESS          ((langCode) ? "Вычисление граничных условий" : "Calculation of boundary conditions")
#define S_MSG_TIMER                     ((langCode) ? "Выполнено за: " : "Done in: ")
#define S_MSG_LEAD_TIME                 ((langCode) ? "Время выполнения: " : "Lead time: ")
#define S_MSG_SEC                       ((langCode) ? " сек." : " sec.")
#define S_MSG_SYSTEM_PREPARE            ((langCode) ? "Подготовка системы уравнений" : "Preparing the system of equations")
#define S_MSG_SYSTEM_SOLUTION           ((langCode) ? "Решение системы уравнений" : "Solution of the system of equations")
#define S_MSG_ITERATION_ERROR           ((langCode) ? "Невязка" : "Error")
#define S_MSG_SYSTEM_FACTORIZATION      ((langCode) ? "Факторизация системы уравнений" : "Factorization equations")
#define S_MSG_PRINT_RESULT              ((langCode) ? "Печать результатов расчета" : "Printing results")
#define S_MSG_LOAD                      ((langCode) ? "Нагрузка: " : "Load: ")
#define S_MSG_SI                        ((langCode) ? "Максимальная интенсивность напряжений: " : "Max stress intensity: ")
#define S_MSG_TIME_ITERATION            ((langCode) ? "Расчет по времени" : "Time calculation")
#define S_MSG_MESH_ANALYSE              ((langCode) ? "Анализ структуры сетки" : "Analysing of the mesh structure")
#define S_MSG_CHECK_MESH                ((langCode) ? "Проверка структуры сетки" : "Checking mesh")
#define S_MSG_RESTRUCTURE_MESH          ((langCode) ? "Перестроение сетки" : "Restructuring mesh")
#define S_MSG_RESAVE_MESH               ((langCode) ? "Перезапись сетки" : "Rewritinging mesh")
#define S_MSG_CREATE_BC                 ((langCode) ? "Создание списка краевых условий" : "Creating a list of boundary conditions")
#define S_MSG_WRITE_RESULT              ((langCode) ? "Сохранение результатов расчета" : "Writing results")
#define S_MSG_READ_RESULT               ((langCode) ? "Загрузка результатов расчета" : "Reading results")
#define S_MSG_CALC_STANDART_RESULTS     ((langCode) ? "Расчет стандартных результатов КЭ" : "Calculation of standard FE results")
#define S_MSG_MESH_NAME                 ((langCode) ? "Файл сетки: " : "Mesh file: ")
#define S_NUM_THREAD                    ((langCode) ? "Использовано потоков: " : "Using threads: ")
#define S_INITIAL_CONDITION_PARAMETER   ((langCode) ? "Начальные условия" : "Initial condition")
#define S_BOUNDARY_CONDITION_PARAMETER  ((langCode) ? "Граничные условия" : "Boundary condition")
#define S_VOLUME_LOAD_PARAMETER         ((langCode) ? "Объемная нагрузка" : "Volume load")
#define S_SURFACE_LOAD_PARAMETER        ((langCode) ? "Поверхностная нагрузка" : "Surface load")
#define S_CONCENTRATED_LOAD_PARAMETER   ((langCode) ? "Сосредоточенная нагрузка" : "Concentrated load")
#define S_PRESSURE_LOAD_PARAMETER       ((langCode) ? "Нагрузка давлением" : "Pressure load")
#define S_YOUNG_MODULUS_PARAMETER       ((langCode) ? "Модуль Юнга" : "Young's modulus")
#define S_POISSON_RATIO_PARAMETER       ((langCode) ? "Коэффициент Пуассона" : "Poisson's ratio")
#define S_THICKNESS_PARAMETER           ((langCode) ? "Толщина КЭ" : "FE thickness")
#define S_TEMPERATURE_PARAMETER         ((langCode) ? "Разность температур" : "Temperature difference")
#define S_ALPHA_PARAMETER               ((langCode) ? "Температурное расширение" : "Thermal expansion")
#define S_DENSITY_PARAMETER             ((langCode) ? "Плотность" : "Density")
#define S_DAMPING_PARAMETER             ((langCode) ? "Демпфирование" : "Damping")
#define S_STRESS_STRAIN_CURVE_PARAMETER ((langCode) ? "Диаграмма деформирования" : "Stress–strain curve")
#define S_UNDEFINED_PARAMETER           ((langCode) ? "Неопределенный параметер" : "Undefined")
#define S_MSG_ITERATION                 ((langCode) ? "Выполнено итераций: " : "Iterate: ")

using namespace std;


// ----------------------- Коды ошибок -------------------------------
enum class ErrorCode { Undefined = 0, EOpenFile, EReadFile, EWriteFile, EFormatFile, EUndefTypeFile, EUndefTypeFE, ESyntax, EBracket, EName, EUndefVariable,
                       ERedefVariable, EIncorrectFE, EEquationNotSolved, EAbort, EAllocMemory, EStressStrainCurve, EEmptyExpression, EYoungModulus,
                       EPoissonRatio, EThickness, ETemperature, EAlpha, EDensity, EDamping, EIndex };
enum class ProcessCode
            {
                Undefined,
                GeneratingStaticMatrix,
                CalcBoundaryCondition,
                UsingLoad,
                PreparingSystemEquation,
                FactorizationSystemEquation,
                PrintingResult,
                SolutionSystemEquation,
                GeneratingDynamicMatrix,
                AnalysingMesh,
                CheckingMesh,
                RestructuringMesh,
                GeneratingBoundaryCondition,
                GeneratingConcentratedLoad,
                GeneratingSurfaceLoad,
                GeneratingVolumeLoad,
                GeneratingPressureLoad,
                WritingResult,
                ReadingResult,
                GeneratingResult
            };

inline string sayProcess(ProcessCode code)
{
    switch (code)
    {
        case ProcessCode::GeneratingStaticMatrix:
            return S_MSG_FE_STATIC_PROCESS;
        case ProcessCode::CalcBoundaryCondition:
            return S_MSG_BOUNDARY_PROCESS;
        case ProcessCode::UsingLoad:
            return S_MSG_CREATE_LOAD;
        case ProcessCode::PreparingSystemEquation:
            return S_MSG_SYSTEM_PREPARE;
        case ProcessCode::FactorizationSystemEquation:
            return S_MSG_SYSTEM_FACTORIZATION;
        case ProcessCode::PrintingResult:
            return S_MSG_PRINT_RESULT;
        case ProcessCode::GeneratingConcentratedLoad:
            return S_MSG_CALC_CONCENTRATED_LOAD;
        case ProcessCode::GeneratingSurfaceLoad:
            return S_MSG_CALC_SURFACE_LOAD;
        case ProcessCode::GeneratingVolumeLoad:
            return S_MSG_CALC_VOLUME_LOAD;
        case ProcessCode::GeneratingPressureLoad:
            return S_MSG_CALC_PRESSURE_LOAD;
        case ProcessCode::GeneratingResult:
            return S_MSG_CALC_STANDART_RESULTS;
        case ProcessCode::SolutionSystemEquation:
            return S_MSG_SYSTEM_SOLUTION;
        case ProcessCode::GeneratingDynamicMatrix:
            return S_MSG_FE_DYNAMIC_PROCESS;
        case ProcessCode::AnalysingMesh:
            return S_MSG_MESH_ANALYSE;
        case ProcessCode::CheckingMesh:
            return S_MSG_CHECK_MESH;
        case ProcessCode::RestructuringMesh:
            return S_MSG_RESTRUCTURE_MESH;
        case ProcessCode::GeneratingBoundaryCondition:
            return S_MSG_CREATE_BC;
        case ProcessCode::WritingResult:
            return S_MSG_WRITE_RESULT;
        case ProcessCode::ReadingResult:
            return S_MSG_READ_RESULT;
        default:
            break;
    }
    return "";
}

inline string sayError(ErrorCode code)
{
    switch (code)
    {
        case ErrorCode::EOpenFile:
            return S_ERR_OPEN_FILE;
        case ErrorCode::EReadFile:
            return S_ERR_READ_FILE;
        case ErrorCode::EWriteFile:
            return S_ERR_WRITE_FILE;
        case ErrorCode::EFormatFile:
            return S_ERR_FORMAT_FILE;
        case ErrorCode::EUndefTypeFile:
            return S_ERR_FORMAT_FILE;
        case ErrorCode::EUndefTypeFE:
            return S_ERR_UNKNOWN_FE;
        case ErrorCode::ESyntax:
            return S_ERR_SYNTAX;
        case ErrorCode::EBracket:
            return S_ERR_CRAMP;
        case ErrorCode::EName:
            return S_ERR_NAME;
        case ErrorCode::EUndefVariable:
            return S_ERR_UNDEF_VARIABLE;
        case ErrorCode::ERedefVariable:
            return S_ERR_DEF_VARIABLE;
        case ErrorCode::EIncorrectFE:
            return S_ERR_INCORRECT_FE;
        case ErrorCode::EEquationNotSolved:
            return S_ERR_EQUATION_NOT_SOLVED;
        case ErrorCode::EAbort:
            return S_ERR_ABORT;
        case ErrorCode::EAllocMemory:
            return S_ERR_MEMORY;
        case ErrorCode::EStressStrainCurve:
            return S_ERR_NONLINEAR;
        case ErrorCode::EYoungModulus:
            return S_ERR_YOUNG_MODULUS;
        case ErrorCode::EPoissonRatio:
            return S_ERR_POISSON_RATIO;
        case ErrorCode::EThickness:
            return S_ERR_THICKNESS;
        case ErrorCode::ETemperature:
            return S_ERR_TEMPERATURE;
        case ErrorCode::EAlpha:
            return S_ERR_ALPHA;
        case ErrorCode::EDensity:
            return S_ERR_DENSITY;
        case ErrorCode::EDamping:
            return S_ERR_DAMPING;
        case ErrorCode::EIndex:
            return S_ERR_INDEX;
        default:
            break;
    }
    return "";
}


//-------------------------------------------------------------------
// Реализация системы сообщений о ходе выполнения текущего процесса
//-------------------------------------------------------------------
class TMessenger
{
private:
    chrono::system_clock::time_point timer;
    bool isStopped = false;
    thread progress_thread;
    void backgroundRun(bool& isStopped)
    {
        char chr[] = { '|', '/', '-', '\\' };
        int i = 0;

        while (not isStopped)
        {
            cout << '\r' << sayProcess(processCode) << "... " << chr[i++ % 4] << flush;
            //std::this_thread::yield();
            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        cout << '\r' << sayProcess(processCode) << "... 100%" << endl;
    }
protected:
    ProcessCode processCode;    // Код процесса
    int processStart;
    int processStop;
    int processCurrent;
    int processStep;
    int oldPersent;
public:
    TMessenger(void)
    {
        processCode = ProcessCode::Undefined;
        processStart = processStop = processCurrent = oldPersent = 0;
        processStep = 1;
    }
    virtual ~TMessenger(void) {}
    virtual void setProcess(ProcessCode code)
    {
        isStopped = false;
        processCode = code;
        processStart = processStop = processCurrent = oldPersent = 0;

        progress_thread = thread(&TMessenger::backgroundRun, this, ref(this->isStopped));
        progress_thread.detach();
        timer = chrono::system_clock::now();
    }
    virtual void setProcess(ProcessCode code, int start, int stop, int step = 1)
    {
        processCode = code;
        processStart = start;
        processStop = stop;
        processStep = step;
        processCurrent = oldPersent = 0;
        cout << '\r' << sayProcess(processCode) << "... 0%" << flush;
        timer = chrono::system_clock::now();
    }
    virtual void addProgress(void)
    {
        stringstream ss;
        int persent = (processStop - processStart) ? int((100.0 * double(++processCurrent)) / double(processStop - processStart)) : 100;

        if (processCurrent == processStop)
        {
            ss << '\r' << sayProcess(processCode) << "... 100%";
            cout << ss.str() << flush;
            return;
        }
        if (persent == oldPersent)
            return;
        if (persent % processStep == 0)
        {
            ss << '\r' << sayProcess(processCode) << "... " << persent << "%";
            cout << ss.str() << flush;
        }
        oldPersent = persent;
    }
    void stopProcess(void)
    {
        stringstream ss;

        ss << '\r' << sayProcess(processCode) << "... 100%" << endl << S_MSG_TIMER << int(double(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count())) << S_MSG_SEC << endl;
        cout << ss.str() << flush;
    }
    virtual void stop(void)
    {
        isStopped = true;
        this_thread::sleep_for(std::chrono::milliseconds(200));
        cout << S_MSG_TIMER << int(double((static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()))) << S_MSG_SEC << endl;
    }
};

#endif // MESSAGE_H
