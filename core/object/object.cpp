#include "fe/fe1d.h"
#include "fe/fe2d.h"
#include "fe/fe3d.h"
#include "fe/fe2dp.h"
#include "fe/fe3ds.h"
#include "object.h"
#include "fem/femstatic_mvs.h"
#include "fem/femdynamic.h"
#include "solver/psolver.h"
//#include "solver/bccsolver.h"
//#include "solver/eigensolver.h"
//#include "solver/lzhsolver.h"

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
    cout << endl << S_MSG_MESH_NAME << n << endl;
    fileName = n;
    objName = n.substr(n.find_last_of("/\\") + 1,n.find_last_of(".") - n.find_last_of("/\\") - 1);
    return !mesh.read(fileName);
}
//---------------------------------------------------------
template<typename SOLVER> TFEM *TFEMObject::createProblem(void)
{
    TFEM *fem = nullptr;

    switch (mesh.getTypeFE())
    {
    case FEType::fe1d2:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE1D<TShape1D2>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE1D<TShape1D2>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE1D<TShape1D2>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d3:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2D<TShape2D3>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2D<TShape2D3>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2D<TShape2D3>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d4:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2D<TShape2D4>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2D<TShape2D4>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2D<TShape2D4>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d6:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2D<TShape2D6>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2D<TShape2D6>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2D<TShape2D6>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d3p:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2DP<TShape2D3>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2DP<TShape2D3>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2DP<TShape2D3>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d4p:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2DP<TShape2D4>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2DP<TShape2D4>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2DP<TShape2D4>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe2d6p:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE2DP<TShape2D6>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE2DP<TShape2D6>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE2DP<TShape2D6>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d4:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3D<TShape3D4>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE3D<TShape3D4>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE3D<TShape3D4>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d8:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3D<TShape3D8>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE3D<TShape3D8>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE3D<TShape3D8>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d10:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3D<TShape3D10>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER,  TFE3D<TShape3D10>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER,  TFE3D<TShape3D10>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d3s:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3DS<TShape2D3>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE3DS<TShape2D3>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE3DS<TShape2D3>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d4s:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3DS<TShape2D4>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE3DS<TShape2D4>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE3DS<TShape2D4>>(objName, &mesh, &results, &notes);
        break;
    case FEType::fe3d6s:
        if (params.fType == FEMType::StaticProblem)
        {
            if (params.pMethod == PlasticityMethod::Linear)
                fem = new TFEMStatic<SOLVER, TFE3DS<TShape2D6>>(objName, &mesh, &results, &notes);
            else if (params.pMethod == PlasticityMethod::MVS)
                fem = new TFEMStaticMVS<SOLVER, TFE3DS<TShape2D6>>(params.loadStep, objName, &mesh, &results, &notes);
        }
        else if (params.fType == FEMType::DynamicProblem)
            fem = new TFEMDynamic<SOLVER, TFE3DS<TShape2D6>>(objName, &mesh, &results, &notes);
        break;
    default:
        break;
    }
    return fem;
}
//---------------------------------------------------------
bool TFEMObject::start(void)
{
//    Eigen::initParallel();
//    Eigen::setNbThreads(std::thread::hardware_concurrency() - 1);

    results.clear();
    notes.clear();
    cout << endl << S_MSG_START << endl;
    try
    {
        fem = createProblem<TPardisoSolver>();
//        fem = createProblem<TBCCSolver>();
//        fem = createProblem<TEigenSolver>();
//        fem = createProblem<TLZHSolver>();
        // Задание количества потоков
        fem->setNumThread(numThread);
        // Задание параметров расчета
        fem->setParams(params);
        // Запуск расчета
        isProcessStarted = true;
        fem->startProcess();
        isProcessCalculated = fem->isCalculated();
    }
    catch (ErrorCode &err)
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
    ofstream out(fname.c_str(), ios::out);
    bool ret;

    if (not out.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return false;
    }
    out.precision(16);

    msg->setProcess(ProcessCode::WritingResult);
    // Запись подписи
    out << "Core QFEM results file" << endl;
    // Запись сетки
    mesh.write(out);
    if (out.fail())
    {
        out.close();
        msg->stop();
        cerr << sayError(ErrorCode::EWriteFile) << endl;
        return false;
    }
    // Запись результатов
    if (not results.write(out))
    {
        out.close();
        msg->stop();
        cerr << sayError(ErrorCode::EWriteFile) << endl;
        return false;
    }
    // Запись параметров
    if (not params.write(out))
        cerr << sayError(ErrorCode::EWriteFile) << endl;

    // Запись примечаний к расчету
    out << notes.size() << endl;
    for (auto &it : notes)
        out << it << endl;

    ret = out.fail();
    out.close();
    msg->stop();
    return !ret;
}
//-------------------------------------------------------------
bool TFEMObject::loadResult(string fname)
{
    unsigned len;
    ifstream in(fname.c_str(), ios::in);
    string str;

    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return false;
    }
    msg->setProcess(ProcessCode::ReadingResult);
    // Считывание заголовка
    getline(in, str);
    if (str != "Core QFEM results file" && str != "FEM Solver Results File")
    {
        msg->stop();
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return false;
    }
    // Загрузка сетки
    mesh.read(in);
    if (in.fail())
    {
        msg->stop();
        in.close();
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return false;
    }
    // Загрузка результатов
    if (not results.read(in))
    {
        msg->stop();
        in.close();
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return false;
    }
    // Считывание параметров
    if (not params.read(in))
    {
        msg->stop();
        in.close();
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return false;
    }
    // Считывание примечаний к расчету
    in >> len;
    getline(in, str);
    notes.clear();
    for (unsigned i = 0;  i < len; i++)
    {
        getline(in, str);
        notes.push_back(str);
    }

    isProcessCalculated = true;
    in.close();
    msg->stop();
    mesh.createMeshMap();
    return true;
}
//-------------------------------------------------------------
//       Формирование файла с результатами расчетов
//-------------------------------------------------------------
void TFEMObject::printResult(string fname)
{
    size_t lenStr,
           lenNo;
    double t = (params.fType == FEMType::StaticProblem) ? 0 : params.t0 + params.th,
           tmp = -1.0123456789e-15;
    unsigned size = 0,
             index = 0,
             counter_size = 1;
    string name;
    stringstream ss;
    ofstream out(fname.c_str(), ios::out);

    if (not out.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
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
    if (not size)
        return;
    size++;

    if (params.fType == FEMType::DynamicProblem)
        counter_size = unsigned((params.t1 - params.t0) / params.th);
    msg->setProcess(ProcessCode::PrintingResult, 1, int(counter_size * mesh.getNumVertex()));
    do
    {
        if (params.fType == FEMType::DynamicProblem)
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
                cerr << sayError(ErrorCode::EWriteFile) << endl;
                return;
            }
        }
        out << endl;
        out << endl;


        // Печать итогов
        if (params.fType == FEMType::DynamicProblem)
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
        if (params.fType == FEMType::StaticProblem)
            break;
        t += params.th;
        if (fabs(t - params.t1) < params.eps)
            t = params.t1;
        index += size;
    }
    while (t <= params.t1);
    out.close();
    msg->stopProcess();
    if (out.fail())
        cerr << sayError(ErrorCode::EWriteFile) << endl;
}
//-------------------------------------------------------------
void TFEMObject::setLanguage(int lang)
{
    langCode = lang;
}
//-------------------------------------------------------------
