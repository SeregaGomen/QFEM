#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sstream>
#include <thread>
//#include <mutex>

extern int langCode;

#define S_ERR                           ((langCode) ? "Ошибка" : "Error")
#define S_NO_ERR                        ((langCode) ? "OK" : "OK")
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

#define S_MSG_START                     ((langCode) ? "******************************* Старт *******************************" : "******************************* Start *******************************")
#define S_MSG_STOP                      ((langCode) ? "******************************* Стоп  *******************************" : "******************************* Stop  *******************************")
#define S_MSG_IDLE                      ((langCode) ? "Простой" : "Idle")
#define S_MSG_CALC_LOAD                 ((langCode) ? "Расчет нагрузок" : "Computation of load")
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
#define S_MSG_BOUNDARY_PROCESS          ((langCode) ? "Учет краевых условий" : "Using of boundary conditions")
#define S_MSG_TIMER                     ((langCode) ? "Выполнено за: " : "Done in: ")
#define S_MSG_LEAD_TIME                 ((langCode) ? "Время выполнения: " : "Lead time: ")
#define S_MSG_SEC                       ((langCode) ? " сек." : " sec.")
#define S_MSG_SYSTEM_PREPARE            ((langCode) ? "Подготовка системы уравнений" : "Preparing the system of equations")
#define S_MSG_SYSTEM_SOLUTION           ((langCode) ? "Решение системы уравнений" : "Solution of the system of equations")
#define S_MSG_ITERATION                 ((langCode) ? "Выполнено итераций: " : "Iterate: ")
#define S_MSG_ITERATION_ERROR           ((langCode) ? "Невязка" : "Error")
#define S_MSG_SYSTEM_FACTORIZATION      ((langCode) ? "Факторизация системы уравнений" : "Factorization equations")
#define S_MSG_PRINT_RESULT              ((langCode) ? "Печать результатов расчета" : "Printing results")
#define S_MSG_LOAD                      ((langCode) ? "Нагрузка: " : "Load: ")
#define S_MSG_SI                        ((langCode) ? "Максимальная интенсивность напряжений: " : "Max stress intensity: ")
#define S_MSG_TIME_ITERATION            ((langCode) ? "Расчет по времени" : "Time calculation")
#define S_MSG_SE_MATRIX_GENERATION      ((langCode) ? "Формирование матрицы СЛАУ" : "Building of matrix SE")
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

using namespace std;


// ----------------------- Коды ошибок -------------------------------
enum ErrorCode
            {
                NO_ERR = 0,
                OPEN_FILE_ERR,
                READ_FILE_ERR,
                WRITE_FILE_ERR,
                FORMAT_FILE_ERR,
                UNKNOWN_FILE_ERR,
                UNKNOWN_FE_ERR,
                SYNTAX_ERR,
                CRAMP_ERR,
                NAME_ERR,
                UNDEF_VARIABLE_ERR,
                DEF_VARIABLE_ERR,
                INCORRECT_FE_ERR,
                EQUATION_NOT_SOLVED_ERR,
                ABORT_ERR,
                ALLOC_MEMORY_ERR,
                ERROR_ERR,
                NONLINEAR_PARAM_ERR,
                EMPTY_EXPRESSION_ERR,
                YOUNG_MODULUS_ERR,
                POISSON_RATIO_ERR,
                THICKNESS_ERR,
                TEMPERATURE_ERR,
                ALPHA_ERR,
                DENSITY_ERR,
                DAMPING_ERR
            };
enum ProcessCode
            {
                IDLE_PROCESS,
                GENERATE_FE_STATIC_PROCESS,
                CALC_BOUNDARY_CONDITION_PROCESS,
                CREATE_LOAD_PROCESS,
                SYSTEM_PREPARE_PROCESS,
                SYSTEM_FACTORIZATION_PROCESS,
                PRINT_RESULT_PROCESS,
                CALCULATION_LOAD_PROCESS,
                SYSTEM_ITERATION_PROCESS,
                SYSTEM_SOLUTION_PROCESS,
                GENERATE_FE_DYNAMIC_PROCESS,
                TIME_ITERATION_PROCESS,
                GENERATE_SE_MATRIX_PROCESS,
                MESH_ANALYSE_PROCESS,
                MESH_CHECK_PROCESS,
                MESH_RESTUCTURE_PROCESS,
                BC_CREATE_PROCESS,
                CALCULATION_CONCENTRATED_LOAD_PROCESS,
                CALCULATION_SURFACE_LOAD_PROCESS,
                CALCULATION_VOLUME_LOAD_PROCESS,
                CALCULATION_PRESSURE_LOAD_PROCESS,
                WRITE_RESULT_PROCESS,
                READ_RESULT_PROCESS,
                CALCULATION_STANDART_RESULT_PROCESS
            };

inline string sayProcess(ProcessCode code)
{
    string ret;

    switch (code)
    {
        case GENERATE_FE_STATIC_PROCESS:
            ret = S_MSG_FE_STATIC_PROCESS;
            break;
        case CALC_BOUNDARY_CONDITION_PROCESS:
            ret = S_MSG_BOUNDARY_PROCESS;
            break;
        case CREATE_LOAD_PROCESS:
            ret = S_MSG_CREATE_LOAD;
            break;
        case SYSTEM_PREPARE_PROCESS:
            ret = S_MSG_SYSTEM_PREPARE;
            break;
        case SYSTEM_FACTORIZATION_PROCESS:
            ret = S_MSG_SYSTEM_FACTORIZATION;
            break;
        case PRINT_RESULT_PROCESS:
            ret = S_MSG_PRINT_RESULT;
            break;
        case CALCULATION_LOAD_PROCESS:
            ret = S_MSG_CALC_LOAD;
            break;
        case CALCULATION_CONCENTRATED_LOAD_PROCESS:
            ret = S_MSG_CALC_CONCENTRATED_LOAD;
            break;
        case CALCULATION_SURFACE_LOAD_PROCESS:
            ret = S_MSG_CALC_SURFACE_LOAD;
            break;
        case CALCULATION_VOLUME_LOAD_PROCESS:
            ret = S_MSG_CALC_VOLUME_LOAD;
            break;
        case CALCULATION_PRESSURE_LOAD_PROCESS:
            ret = S_MSG_CALC_PRESSURE_LOAD;
            break;
        case CALCULATION_STANDART_RESULT_PROCESS:
            ret = S_MSG_CALC_STANDART_RESULTS;
            break;
        case SYSTEM_ITERATION_PROCESS:
            ret = S_MSG_ITERATION;
            break;
        case SYSTEM_SOLUTION_PROCESS:
            ret = S_MSG_SYSTEM_SOLUTION;
            break;
        case GENERATE_FE_DYNAMIC_PROCESS:
            ret = S_MSG_FE_DYNAMIC_PROCESS;
            break;
        case TIME_ITERATION_PROCESS:
            ret = S_MSG_TIME_ITERATION;
            break;
        case GENERATE_SE_MATRIX_PROCESS:
            ret = S_MSG_SE_MATRIX_GENERATION;
            break;
        case MESH_ANALYSE_PROCESS:
            ret = S_MSG_MESH_ANALYSE;
            break;
        case MESH_CHECK_PROCESS:
            ret = S_MSG_CHECK_MESH;
            break;
        case MESH_RESTUCTURE_PROCESS:
            ret = S_MSG_RESTRUCTURE_MESH;
            break;
        case BC_CREATE_PROCESS:
            ret = S_MSG_CREATE_BC;
            break;
        case WRITE_RESULT_PROCESS:
            ret = S_MSG_WRITE_RESULT;
            break;
        case READ_RESULT_PROCESS:
            ret = S_MSG_READ_RESULT;
            break;
        default:
            ret = S_MSG_IDLE;
    }
    return ret;
}

inline string sayError(ErrorCode code)
{
    string ret;

    switch (code)
    {
        case OPEN_FILE_ERR:
            ret = S_ERR_OPEN_FILE;
            break;
        case READ_FILE_ERR:
            ret = S_ERR_READ_FILE;
            break;
        case WRITE_FILE_ERR:
            ret = S_ERR_WRITE_FILE;
            break;
        case FORMAT_FILE_ERR:
            ret = S_ERR_FORMAT_FILE;
            break;
        case UNKNOWN_FILE_ERR:
            ret = S_ERR_FORMAT_FILE;
            break;
        case UNKNOWN_FE_ERR:
            ret = S_ERR_UNKNOWN_FE;
            break;
        case SYNTAX_ERR:
            ret = S_ERR_SYNTAX;
            break;
        case CRAMP_ERR:
            ret = S_ERR_CRAMP;
            break;
        case NAME_ERR:
            ret = S_ERR_NAME;
            break;
        case UNDEF_VARIABLE_ERR:
            ret = S_ERR_UNDEF_VARIABLE;
            break;
        case DEF_VARIABLE_ERR:
            ret = S_ERR_DEF_VARIABLE;
            break;
        case INCORRECT_FE_ERR:
            ret = S_ERR_INCORRECT_FE;
            break;
        case EQUATION_NOT_SOLVED_ERR:
            ret = S_ERR_EQUATION_NOT_SOLVED;
            break;
        case ABORT_ERR:
            ret = S_ERR_ABORT;
            break;
        case ALLOC_MEMORY_ERR:
            ret = S_ERR_MEMORY;
            break;
        case NONLINEAR_PARAM_ERR:
            ret = S_ERR_NONLINEAR;
            break;
        case YOUNG_MODULUS_ERR:
            ret = S_ERR_YOUNG_MODULUS;
            break;
        case POISSON_RATIO_ERR:
            ret = S_ERR_POISSON_RATIO;
            break;
        case THICKNESS_ERR:
            ret = S_ERR_THICKNESS;
            break;
        case TEMPERATURE_ERR:
            ret = S_ERR_TEMPERATURE;
            break;
        case ALPHA_ERR:
            ret = S_ERR_ALPHA;
            break;
        case DENSITY_ERR:
            ret = S_ERR_DENSITY;
            break;
        case DAMPING_ERR:
            ret = S_ERR_DAMPING;
            break;
        default:
            ret = S_NO_ERR;
    }
    return ret;
}


//-------------------------------------------------------------------
// Реализация системы сообщений о ходе выполнения текущего процесса
//-------------------------------------------------------------------
class TMessenger
{
private:
    time_t timer;
    bool isStopped = false;
    thread progress_thread;
    //mutex mtx;
    void backgroundRun(bool& isStopped)
    {
        char chr[] = { '|', '/', '-', '\\' };
        int i = 0;

        while (!isStopped)
        {
            cout << '\r' << sayProcess(processCode) << "... " << chr[i++ % 4] << flush;
            //std::this_thread::yield();
            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        cout << '\r' << sayProcess(processCode) << "... 100%" << endl << flush;
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
        processCode = IDLE_PROCESS;
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
        timer = clock();
    }
    virtual void setProcess(ProcessCode code, int start, int stop, int step = 1)
    {
        processCode = code;
        processStart = start;
        processStop = stop;
        processStep = step;
        processCurrent = oldPersent = 0;
        cout << '\r' << sayProcess(processCode) << "... 0%" << flush;
        timer = clock();
    }
    virtual void addProgress(void)
    {
        stringstream ss;
        int persent = (processStop - processStart) ? int((100.0 * double(++processCurrent)) / double(processStop - processStart)) : 100;

        if (processCurrent == processStop)
        {
            ss << '\r' << sayProcess(processCode) << "... 100%"/* << endl << S_MSG_TIMER << int(double((clock() - timer) / CLOCKS_PER_SEC)) << S_MSG_SEC << endl << endl*/;
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

//        if (processCurrent == processStop)
//            return;
        ss << '\r' << sayProcess(processCode) << "... 100%" << endl << S_MSG_TIMER << int(double((clock() - timer) / CLOCKS_PER_SEC)) << S_MSG_SEC << endl << endl;
        cout << ss.str() << flush;
    }
    virtual void stop(void)
    {
        isStopped = true;
        this_thread::sleep_for(std::chrono::milliseconds(200));
        cout << S_MSG_TIMER << int(double((clock() - timer) / CLOCKS_PER_SEC)) << S_MSG_SEC << endl << endl;
    }
};

#endif // MESSAGE_H
