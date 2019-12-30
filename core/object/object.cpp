#include "object.h"
#include "fem/femtstatic_t.h"
#include "fem/femstatic_mvs.h"
#include "fem/femdynamic.h"
#include "solver/bccsolver.h"
#include "solver/eigensolver.h"

int langCode = 0; // Код языка (0 - английский, 1 - русский)

//---------------------------------------------------------
void TFEMObject::clear(void)
{
    fem = nullptr;
    mesh.clear();
    params.clear();
    notes.clear();
    isProcessStarted = isProcessCalculated = false;
}
//---------------------------------------------------------
bool TFEMObject::setMeshFile(string n)
{
    cout << endl << s_MSG_MESH_NAME << n << endl;
    fileName = n;
    objName = n.substr(n.find_last_of("/\\") + 1,n.find_last_of(".") - n.find_last_of("/\\") - 1);
    return !mesh.read(fileName);
}
//---------------------------------------------------------
bool TFEMObject::start(void)
{
    results.clear();
    notes.clear();
    cout << endl << S_MSG_START << endl;
    try
    {
        switch (params.fType)
        {
            case StaticProblem:
                if (params.pMethod == Linear) // Упругий расчет
                    // fem = new TFEMStatic<TBCCSolver>(objName, &mesh, &results, &notes);
                    fem = new TFEMStaticThread<TBCCSolver>(objName, &mesh, &results, &notes);
                    // fem = new TFEMStaticThread<TEigenSolver>(objName, &mesh, &results, &notes);
                else if (params.pMethod == MVS)
                    fem = new TFEMStaticMVS<TBCCSolver>(params.forceStep, objName, &mesh, &results, &notes);
                break;
            case DynamicProblem:
                fem = new TFEMDynamic<TBCCSolver>(objName, &mesh, &results, &notes);
        }
        // Задание параметров расчета
        fem->setParams(params);
        // Запуск расчета
        isProcessStarted = true;
        fem->startProcess();
        isProcessCalculated = fem->isCalculated();
    }
    catch (ErrorCode& err)
    {
        cerr << endl << sayError(err) << endl;
        isProcessCalculated = false;
    }
    isProcessStarted = false;
    cout << S_MSG_STOP << endl;

    delete fem;
    return isProcessCalculated;
}
//---------------------------------------------------------
void TFEMObject::setTaskParam(FEMType t)
{
    params.fType = t;
}
//---------------------------------------------------------
bool TFEMObject::saveResult(string fname)
{
    ofstream out;
    bool ret;

    out.open(fname.c_str(),ios::out);
    if (out.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
        return false;
    }
    out.precision(16);

    msg->setProcess(WRITE_RESULT_PROCESS);
    // Запись подписи
    out << "QFEM results file" << endl;
    // Запись сетки
    mesh.write(out);
    if (out.fail())
    {
        out.close();
        msg->stop();
        cerr << sayError(WRITE_FILE_ERR) << endl;
        return false;
    }
    // Запись результатов
    if (!results.write(out))
    {
        out.close();
        msg->stop();
        cerr << sayError(WRITE_FILE_ERR) << endl;
        return false;
    }
//    // Запись параметров
//    if (!params.write(out))
//        msg->setError(WRITE_FILE_ERR);

    ret = out.fail();
    out.close();
    msg->stop();
    return !ret;
}
//-------------------------------------------------------------
bool TFEMObject::loadResult(string fname)
{
    ifstream in;
    string str;

    in.open(fname.c_str(),ios::in);
    if (in.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
        return false;
    }
    msg->setProcess(READ_RESULT_PROCESS);
    // Считывание заголовка
    getline(in, str);
    if (str != "QFEM results file")
    {
        msg->stop();
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
        return false;
    }
    // Загрузка сетки
    mesh.read(in);
    if (in.fail())
    {
        msg->stop();
        in.close();
        cerr << sayError(READ_FILE_ERR) << endl;
        return false;
    }
    // Загрузка результатов
    if (!results.read(in))
    {
        msg->stop();
        in.close();
        cerr << sayError(READ_FILE_ERR) << endl;
        return false;
    }
//    // Считывание параметров
//    if (!params.read(in))
//    {
//        msg->stop();
//        in.close();
//        msg->setError(READ_FILE_ERR);
//        return false;
//    }
    isProcessCalculated = true;
    in.close();
    msg->stop();
    return true;
}
//-------------------------------------------------------------
//       Формирование файла с результатами расчетов
//-------------------------------------------------------------
void TFEMObject::printResult(string fname)
{
    double tmp = -1.0123456789e-15;
    stringstream ss;
    size_t lenStr,
           lenNo;
    ofstream out;
    double t = (params.fType == StaticProblem) ? 0 : params.t0 + params.th;
    unsigned size = 0,
             index = 0,
             counter_size = 1;
    string name;

    out.open(fname.c_str(),ios::out);
    if (out.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
        return;
    }

    // Вычисление параметров печати
    ss << mesh.getNumVertex();
    lenNo = ss.str().length();
    ss.str(std::string());
    ss.clear();
    ss << setiosflags(std::ios_base::showpos | std::ios_base::scientific) << setw(int(params.width)) << setprecision(int(params.precision)) << tmp;
    lenStr = ss.str().length();

    // Подсчет количества выводимых значений для одного момента времени
    for (unsigned i = 1; i < results.size(); i++)
        if (fabs(results[i].getTime() - results[0].getTime()) < params.eps)
            size++;
        else
            break;
    if (!size)
        return;
    size++;

    if (params.fType == DynamicProblem)
        counter_size = unsigned((params.t1 - params.t0) / params.th);
    msg->setProcess(PRINT_RESULT_PROCESS, 0, int(counter_size * mesh.getNumVertex()) - 1, 10);
    do
    {
        if (params.fType == DynamicProblem)
            out << "t = " << t << endl;
        //////////////////
        // Печать заголовка
        out << "| " << setw(int(lenNo)) << "N" << " (";
        for (unsigned i = 0; i < mesh.getDimension(); i++)
        {
            out << setw(int(lenStr)) << params.names[i];
            if (i <  mesh.getDimension() - 1)
                out << ",";
        }
        out << ") | ";
        for (unsigned i = index; i < index + size; i++)
        {
            name = results[i].getName();
            if (name.find("(") != string::npos)
                name = name.substr(0, name.find("("));
            out << setw(int(lenStr)) << name.substr(name.find(">") + 1) << " | ";
        }
        out << endl;


        out.setf(ios::floatfield,ios::scientific);
        for (unsigned i = 0; i < mesh.getNumVertex(); i++)
        {
            msg->addProgress();

            out << "| ";
            out.width(int(lenNo));
            out << (i + 1);
            out << " (";

            ss.str(std::string());
            ss.clear();
            ss << setw(int(params.width)) << setprecision(int(params.precision)) << ((fabs(mesh.getX(i,0)) < params.eps) ? 0 : mesh.getX(i,0));
            out << ss.str();
            if (mesh.getDimension() > 1)
            {
                out << ',';
                ss.str(std::string());
                ss.clear();
                ss << setw(int(params.width)) << setprecision(int(params.precision)) << ((fabs(mesh.getX(i,1)) < params.eps) ? 0 : mesh.getX(i,1));
                out << ss.str();
            }
            if (mesh.getDimension() > 2)
            {
                out << ',';
                ss.str(std::string());
                ss.clear();
                ss << setw(int(params.width)) << setprecision(int(params.precision)) << ((fabs(mesh.getX(i,2)) < params.eps) ? 0 : mesh.getX(i,2));
                out << ss.str();
            }
            out << ") | ";
            for (unsigned k = index; k < index + size; k++)
            {
                ss.str(std::string());
                ss.clear();
                ss << setw(int(params.width)) << setprecision(int(params.precision)) << results[k].getResults()[i];
                out << ss.str() << " | ";
            }
            out << endl;
            if (out.fail())
            {
                out.close();
                cerr << sayError(WRITE_FILE_ERR) << endl;
                return;
            }
        }
        out << endl;
        out << endl;


        // Печать итогов
        if (params.fType == DynamicProblem)
            out << "t = " << t << endl;
        out << "| " << setw(int(lenNo)) << " " << "  ";
        for (unsigned i = 0; i < mesh.getDimension(); i++)
        {
            out << setw(int(lenStr)) << " ";
            if (i <  mesh.getDimension() - 1)
                out << " ";
        }
        out << "  | ";
        for (unsigned i = index; i < index + size; i++)
        {
            name = results[i].getName();
            if (name.find("(") != string::npos)
                name = name.substr(0, name.find("("));
            out << setw(int(lenStr)) << name.substr(name.find(">") + 1) << " | ";
        }
        out << endl;

        out << "| ";
        out << std::setw(int(mesh.getDimension() * lenStr + mesh.getDimension() + 2 + lenNo)) << "min:";
        out << " | ";

        for (unsigned k = index; k < index + size; k++)
        {
            ss.str(std::string());
            ss.clear();
            ss << setw(int(params.width)) << setprecision(int(params.precision)) << *std::min_element(results[k].getResults().begin(),results[k].getResults().end());
            out << ss.str() << " | ";
        }
        out << endl;
        out << "| ";


        out << setw(int(mesh.getDimension() * lenStr + mesh.getDimension() + 2 + lenNo)) << "max:";
        out << " | ";

        for (unsigned k = index; k < index + size; k++)
        {
            ss.str(std::string());
            ss.clear();
            ss << setw(int(params.width)) << setprecision(int(params.precision)) << *std::max_element(results[k].getResults().begin(),results[k].getResults().end());
            out << ss.str() << " | ";
        }
        out << endl << endl;
        /////////////////
        if (params.fType == StaticProblem)
            break;
        t += params.th;
        if (fabs(t - params.t1) < params.eps)
            t = params.t1;
        index += size;
    }
    while (t <= params.t1);
    out.close();
    if (out.fail())
        cerr << sayError(WRITE_FILE_ERR) << endl;
}
//-------------------------------------------------------------
void TFEMObject::setLanguage(int lang)
{
    langCode = lang;
}
//-------------------------------------------------------------
