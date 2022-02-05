#include <string>
#include <climits>
#include <algorithm>
#include <iomanip>
#include "msg/msg.h"
#include "fe/fe1d.h"
#include "mesh.h"

using namespace std;

extern TMessenger *msg;

// ------------- Чтение файла данных о КЭ-разбиении --------------
bool TMesh::read(string fname)
{
    size_t pos;
    string ext;

    clear();
    // Определяем тип файла
//    pos = fname.find_last_of('.');
    if ((pos = fname.find_last_of('.')) == string::npos)
    {
        cerr << sayError(ErrorCode::EUndefTypeFile) << endl;
        return (error = true);
    }
    ext = fname.substr(pos + 1);
    transform(ext.begin(), ext.end(), ext.begin(), static_cast<int (*)(int)>(toupper));

    // Читаем соответствующий формат
    if (ext == "TRP")
        error = readTRP(fname);
    else if (ext == "TRPA")
        error = readTRPA(fname);
    else if (ext == "VOL")
        error = readVOL(fname);
    else if (ext == "MESH")
        error = readMESH(fname);
    else if (ext == "NODE" or ext == "ELE" or ext == "FACE")
        error = readTetgen(fname);
    else
    {
        cerr << sayError(ErrorCode::EUndefTypeFile) << endl;
        return (error = true);
    }
    if (not error)
    {
        createMeshMap();
        getMinMax();
    }

    // writeTRPA("shell-tube.trpa");

    // writeTRPA("plate3.trpa");
    // writeTRPA("tank3s.trpa");
    // writeTRPA("tank3d.trp");
    // writeTRP("rod.trp");
    return error;
}
// ------------- Определение параметров КЭ ----------------------
FEType TMesh::getDataFE(unsigned type, unsigned& beSize, unsigned& feSize, unsigned& feDim)
{
    FEType ret;

    switch (type)
    {
        case 3:
            ret = FEType::fe2d3;
            beSize = 2;
            feSize = 3;
            feDim = 2;
            break;
        case 4:
            ret = FEType::fe3d4;
            beSize = 3;
            feSize = 4;
            feDim = 3;
            break;
        case 6:
            ret = FEType::fe2d6;
            beSize = 3;
            feSize = 6;
            feDim = 2;
            break;
        case 8:
            ret = FEType::fe3d8;
            beSize = 4;
            feSize = 8;
            feDim = 3;
            break;
        case 10:
            ret = FEType::fe3d10;
            beSize = 6;
            feSize = 10;
            feDim = 3;
            break;
        case 24:
            ret = FEType::fe2d4;
            beSize = 2;
            feSize = 4;
            feDim = 2;
            break;
        case 34:
            ret = FEType::fe1d2;
            beSize = 1;
            feSize = 2;
            feDim = 1;
            break;
        case 123:
            ret = FEType::fe2d3p;
            beSize = 0;
            feSize = 3;
            feDim = 2;
            break;
        case 124:
            ret = FEType::fe2d4p;
            beSize = 0;
            feSize = 4;
            feDim = 2;
            break;
        case 125:
            ret = FEType::fe2d6p;
            beSize = 0;
            feSize = 6;
            feDim = 2;
            break;
        case 223:
            ret = FEType::fe3d3s;
            beSize = 0;
            feSize = 3;
            feDim = 3;
            break;
        case 224:
            ret = FEType::fe3d4s;
            beSize = 0;
            feSize = 4;
            feDim = 3;
            break;
        case 225:
            ret = FEType::fe3d6s;
            beSize = 0;
            feSize = 6;
            feDim = 3;
            break;
        default:
            feSize = beSize = feDim = 0;
            ret = FEType::undefined;
    }
    return ret;
}
//-----------------------------------------------------------------
FEType TMesh::getDataFE(string type, unsigned& beSize, unsigned& feSize, unsigned& feDim)
{
    FEType ret = FEType::undefined;
    vector<tuple<string, FEType, int, int, int>> feTypeTable{
        { "fe1d2", FEType::fe1d2, 1, 2, 1 },
        { "fe2d3", FEType::fe2d3, 2, 3, 2 },
        { "fe2d4", FEType::fe2d4, 2, 4, 2 },
        { "fe2d6", FEType::fe2d6, 3, 6, 2 },
        { "fe2d3p", FEType::fe2d3p, 0, 3, 2 },
        { "fe2d4p", FEType::fe2d4p, 0, 4, 2 },
        { "fe2d6p", FEType::fe2d6p, 0, 6, 2 },
        { "fe3d4", FEType::fe3d4, 3, 4, 3 },
        { "fe3d8", FEType::fe3d8, 4, 8, 3 },
        { "fe3d10", FEType::fe3d8, 6, 10, 3 },
        { "fe3d3s", FEType::fe3d3s, 0, 3, 3 },
        { "fe3d4s", FEType::fe3d4s, 0, 4, 3 },
        { "fe3d6s", FEType::fe3d6s, 0, 6, 3 } };
    auto index = find_if(feTypeTable.begin(), feTypeTable.end(), [type](const auto &it) { return get<0>(it) == type; });

    if (index != feTypeTable.end())
    {
        ret = get<1>(*index);
        beSize = get<2>(*index);
        feSize = get<3>(*index);
        feDim = get<4>(*index);
    }
    return ret;
}
// -------------- Конвертация типов КЭ (для совместимости) -----------------
unsigned TMesh::convertFEType(FEType p)
{
    unsigned ret;

    switch (p)
    {
        case FEType::fe1d2:
            ret = 34;
            break;
        case FEType::fe2d3:
            ret = 3;
            break;
        case FEType::fe2d4:
            ret = 24;
            break;
        case FEType::fe2d6:
            ret = 6;
            break;
        case FEType::fe2d3p:
            ret = 123;
            break;
        case FEType::fe2d4p:
            ret = 124;
            break;
        case FEType::fe2d6p:
            ret = 125;
            break;
        case FEType::fe3d4:
            ret = 4;
            break;
        case FEType::fe3d8:
            ret = 8;
            break;
        case FEType::fe3d10:
            ret = 10;
            break;
        case FEType::fe3d3s:
            ret = 223;
            break;
        case FEType::fe3d4s:
            ret = 224;
            break;
        case FEType::fe3d6s:
            ret = 225;
            break;
        default:
            ret = 0;
    }
    return ret;
}
// -------------- Запись файла данных о конечно-элементном разбиении -----------------
bool TMesh::write(string fname)
{
    size_t pos;
    string ext;

    // Определяем тип файла
//    pos = fname.find_last_of('.');
    if ((pos = fname.find_last_of('.')) == string::npos)
    {
        cerr << sayError(ErrorCode::EUndefTypeFile) << endl;
        return (error = true);
    }
    ext = fname.substr(pos + 1);
    transform(ext.begin(), ext.end(), ext.begin(), static_cast<int (*)(int)>(toupper));

    // Записываем в нужном формате
    if (ext == "TRP")
        error = writeTRP(fname);
    else if (ext == "TRPA")
        error = writeTRPA(fname);
    else
    {
        cerr << sayError(ErrorCode::EUndefTypeFile) << endl;
        error = true;
    }
    return error;
}
// -------------------------------------------------------------
string TMesh::feName(void)
{
    string ret;

    switch (feType)
    {
        case FEType::undefined:
            ret = S_MSG_NO_TYPE;
            break;
        case FEType::fe1d2:
            ret = S_MSG_FE1D2;
            break;
        case FEType::fe2d3:
            ret = S_MSG_FE2D3;
            break;
        case FEType::fe2d4:
            ret = S_MSG_FE2D4;
            break;
        case FEType::fe2d6:
            ret = S_MSG_FE2D6;
            break;
        case FEType::fe2d3p:
            ret = S_MSG_FE2D3_PLATE;
            break;
        case FEType::fe2d4p:
            ret = S_MSG_FE2D4_PLATE;
            break;
        case FEType::fe2d6p:
            ret = S_MSG_FE2D6_PLATE;
            break;
        case FEType::fe3d4:
            ret = S_MSG_FE3D4;
            break;
        case FEType::fe3d8:
            ret = S_MSG_FE3D8;
            break;
        case FEType::fe3d10:
            ret = S_MSG_FE3D10;
            break;
        case FEType::fe3d3s:
            ret = S_MSG_FE3D3_SHELL;
            break;
        case FEType::fe3d4s:
            ret = S_MSG_FE3D4_SHELL;
            break;
        case FEType::fe3d6s:
            ret = S_MSG_FE3D6_SHELL;
            break;
    }
    return ret;
}
// -------------------------------------------------------------
ostream& operator << (ostream& out, TMesh& r)
{
//    out << S_FILENAME_MSG << r.fileName << endl;
    out << S_MSG_FE_TYPE << r.feName() << endl;
    out << S_MSG_NUM_NODES << r.getNumVertex() << endl;
    out << S_MSG_NUM_FE << r.getNumFE() << endl;
    return out;
}
// --------------- Чтение TRP-файла ----------------------
bool TMesh::readTRP(string fname)
{
    ifstream in(fname.c_str(), ios::in | ios::binary);
    char signature[7];
    unsigned temp,
             surfaceSize,
             feSize,
             feDim;


    clear();
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }

    // Cчитываем сигнатуру
    in.read(signature, 6*sizeof(char));
    signature[6] = 0;
    if (string(signature) not_eq "NTRout")
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    // Cчитываем тип КЭ
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    if ((feType = getDataFE(temp, surfaceSize, feSize, feDim)) == FEType::undefined)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    // Cчитываем кол-во граней
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    be.resize(temp,surfaceSize);
    // Считываем кол-во узлов
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    x.resize(temp,feDim);
    // ...
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));

    // Cчитываем узлы
    for (unsigned i = 0; i < x.size1(); i++)
        for (unsigned j = 0; j < x.size2(); j++)
            in.read(reinterpret_cast<char*>(&x(i, j)), sizeof(double));
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    // Cчитываем КЭ
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    fe.resize(temp,feSize);
    for (unsigned i = 0; i < fe.size1(); i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            in.read(reinterpret_cast<char*>(&fe(i, j)), sizeof(unsigned));
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    // Cчитываем грани
    for (unsigned i = 0; i < be.size1(); i++)
        for (unsigned j = 0; j < be.size2(); j++)
            in.read(reinterpret_cast<char*>(&be(i, j)), sizeof(unsigned));
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    in.close();
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    cout << *this << endl;
    return false;
}
// ------------------- Чтение TRPA-файла -----------------------
bool TMesh::readTRPA(string fname)
{
    stringstream ss;
    fstream in(fname.c_str(), ios::in | ios::binary);
    unsigned temp,
             surfaceSize,
             feSize,
             feDim;
    string type;

    clear();
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }
    in >> type;
    ss << type;
    ss >> temp;
    if (ss.fail())
    {
        if ((feType = getDataFE(type, surfaceSize, feSize, feDim)) == FEType::undefined)
        {
            in.close();
            cerr << sayError(ErrorCode::EFormatFile) << endl;
            return (error = true);
        }
    }
    else
    {
        if ((feType = getDataFE(temp, surfaceSize, feSize, feDim)) == FEType::undefined)
        {
            in.close();
            cerr << sayError(ErrorCode::EFormatFile) << endl;
            return (error = true);
        }

    }

    in >> temp;
    if (in.fail() or temp == 0 or feDim < 1 or feDim > 3)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    x.resize(temp, feDim);

    for (unsigned i = 0; i < temp; i++)
    {
        for (unsigned j = 0; j < feDim; j++)
            in >> x(i, j);
        if (in.fail())
        {
            in.close();
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    in >> temp;
    if (in.fail() or temp == 0)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    fe.resize(temp, feSize);
    for (unsigned i = 0; i < temp; i++)
        for (unsigned j = 0; j < feSize; j++)
        {
            in >> fe(i, j);
            if (in.fail())
            {
                in.close();
                cerr << sayError(ErrorCode::EReadFile) << endl;
                return (error = true);
            }
        }
    in >> temp;
    if (in.fail() or (temp == 0 and (feType == FEType::fe2d3 or feType == FEType::fe2d4 or feType == FEType::fe3d4 or feType == FEType::fe3d8)))
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    if (isPlate() or isShell())
        be = fe;
    else // if (feDim not_eq 1)
    {
        be.resize(temp, surfaceSize);
        for (unsigned i = 0; i < temp; i++)
            for (unsigned j = 0; j < surfaceSize; j++)
            {
                in >> be(i, j);
                if (in.fail())
                {
                    in.close();
                    cerr << sayError(ErrorCode::EReadFile) << endl;
                    return (error = true);
                }
            }
    }
    in.close();
    cout << *this << endl;
    return false;
}
// --------------- Запись TRP-файла --------------
bool TMesh::writeTRP(string fname)
{
    ofstream out(fname.c_str(), ios::out | ios::binary);
    const char* signature = "NTRout";
    unsigned temp = 0;

    if (not out.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }

    out.write(signature, 6*sizeof(char));
    out.write(reinterpret_cast<char*>(&(temp = convertFEType(feType))), sizeof(unsigned));
    out.write(reinterpret_cast<char*>(&(temp = be.size1())), sizeof(unsigned));
    out.write(reinterpret_cast<char*>(&(temp = x.size1())), sizeof(unsigned));
    out.write(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    for (unsigned i = 0; i < x.size1(); i++)
        for (unsigned j = 0; j < x.size2(); j++)
            out.write(reinterpret_cast<char*>(&x(i, j)), sizeof(double));
    out.write(reinterpret_cast<char*>(&(temp = fe.size1())), sizeof(unsigned));
    for (unsigned i = 0; i < fe.size1(); i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            out.write(reinterpret_cast<char*>(&(temp = fe(i, j))), sizeof(unsigned));
    for (unsigned i = 0; i < be.size1(); i++)
        for (unsigned j = 0; j < be.size2(); j++)
            out.write(reinterpret_cast<char*>(&(temp = be(i, j))), sizeof(unsigned));
    if (out.fail())
    {
        cerr << sayError(ErrorCode::EWriteFile) << endl;
        return (error = true);
    }
    out.close();
    return false;
}
// -----------------------------------------------------
bool TMesh::writeTRPA(string fname)
{
    ofstream out(fname.c_str(), ios::out);

    if (not out.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }
    out << convertFEType(feType) << endl;
    out << x.size1() << endl;
    for (unsigned i = 0; i < x.size1(); i++)
    {
        for (unsigned j = 0; j < x.size2(); j++)
            out << x(i, j) << ' ';
        out << endl;
    }
    out << fe.size1() << endl;
    for (unsigned i = 0; i < fe.size1(); i++)
    {
        for (unsigned j = 0; j < fe.size2(); j++)
            out << fe(i, j) << ' ';
        out << endl;
    }
    out << be.size1() << endl;
    for (unsigned i = 0; i < be.size1(); i++)
    {
        for (unsigned j = 0; j < be.size2(); j++)
            out << be(i, j) << ' ';
        out << endl;
    }
    if (out.fail())
    {
        cerr << sayError(ErrorCode::EWriteFile) << endl;
        return (error = true);
    }
    out.close();
    return false;
}
// -----------------------------------------------------
bool TMesh::write(ofstream& out)
{
    out << "Mesh" << endl;
    out << convertFEType(feType) << endl;
    out << x.size1() << endl;
    for (unsigned i = 0; i < x.size1(); i++)
    {
        for (unsigned j = 0; j < x.size2(); j++)
            out << x(i, j) << ' ';
        out << endl;
    }
    out << fe.size1() << endl;
    for (unsigned i = 0; i < fe.size1(); i++)
    {
        for (unsigned j = 0; j < fe.size2(); j++)
            out << fe(i, j) << ' ';
        out << endl;
    }
    if (isPlate() or isShell())
        out << 0 << endl;
    else
    {
        out << be.size1() << endl;
        for (unsigned i = 0; i < be.size1(); i++)
        {
            for (unsigned j = 0; j < be.size2(); j++)
                out << be(i, j) << ' ';
            out << endl;
        }
        if (out.fail())
        {
            cerr << sayError(ErrorCode::EWriteFile) << endl;
            return (error = true);
        }
    }
    return error;
}
// -------------------------------------------------------------
bool TMesh::read(ifstream& in)
{
    string str;
    unsigned val,
             surfaceSize,
             feSize,
             feDim;

    //    getline(in, str);
    in >> str;
    // Cчитываем тип КЭ
    in >> val;
    if ((feType = getDataFE(val, surfaceSize, feSize, feDim)) == FEType::undefined)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    // Считываем кол-во узлов
    in >> val;
    x.resize(val, feDim);
    // Cчитываем узлы
    for (unsigned i = 0; i < x.size1(); i++)
        for (unsigned j = 0; j < x.size2(); j++)
            in >> x(i, j);
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    // Cчитываем КЭ
    in >> val;
    fe.resize(val,feSize);
    for (unsigned i = 0; i < fe.size1(); i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            in >> fe(i, j);
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    if (isPlate() or isShell())
    {
        in >> val;
        be = fe;
    }
    else
    {
        // Cчитываем ГЭ
        in >> val;
        be.resize(val,surfaceSize);
        for (unsigned i = 0; i < be.size1(); i++)
            for (unsigned j = 0; j < be.size2(); j++)
                in >> be(i, j);
        if (in.fail())
        {
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    if (not error)
    {
        in.putback('\n');
        cout << *this << endl;
//        createMeshMap();
        getMinMax();
    }
    return error;
}
// --------------- Чтение VOL-файла (NETGEN) ----------------------
bool TMesh::readVOL(string fname)
{
    const int maxLen = 1001;
    ifstream in(fname.c_str(), ios::in);
    char str[maxLen];
    unsigned num,
             tmp,
             i1,
             i2,
             i3,
             i4;

    clear();
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }

    // Заголовок
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    // Количество граничных элементов
    in >> num;
    be.resize(num, 3);
    for (unsigned i = 0; i < num; i++)
    {
        in >> tmp;
        in >> tmp;
        in >> tmp;
        in >> tmp;
        in >> tmp;
        in >> i1;
        in >> i2;
        in >> i3;
        be(i, 0) = i1 - 1;
        be(i, 1) = i2 - 1;
        be(i, 2) = i3 - 1;
    }
    // Заголовок
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    // Количество конечных элементов
    in >> num;
    fe.resize(num, 4);
    for (unsigned i = 0; i < num; i++)
    {
        in >> tmp;
        in >> tmp;
        in >> i1;
        in >> i2;
        in >> i3;
        in >> i4;
        fe(i, 0) = i1 - 1;
        fe(i, 1) = i2 - 1;
        fe(i, 2) = i3 - 1;
        fe(i, 3) = i4 - 1;
    }
    // Заголовок
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in >> num;
    for (unsigned i = 0; i < num; i++)
        in.getline(str,maxLen - 1);
    // Заголовок
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);
    in.getline(str,maxLen - 1);

    // Координаты
    in >> num;
    x.resize(num,3);
    for (unsigned i = 0; i < num; i++)
        in >> x(i, 0) >> x(i, 1) >> x(i, 2);

    in.close();
    feType = FEType::fe3d4;
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    cout << *this << endl;
    return false;
}
// --------------- Чтение MESH-файла (GRUMMP) ----------------------
bool TMesh::readMESH(string fname)
{
    unsigned num1,
             num2,
             numFE,
             numVertex,
             v1,
             v2,
             v3;
    ifstream in(fname.c_str(), ios::in);
    matrix<unsigned> tmp;

    clear();
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }

    // Cчитываем размерности
    in >> numFE >> num1 >> num2 >> numVertex;
    // Cчитываем узлы
    x.resize(numVertex,2);
    for (unsigned i = 0; i < numVertex; i++)
        in >> x(i, 0) >> x(i, 1);

    // Cчитываем грани КЭ
    fe.resize(numFE, 3);
    be.resize(num2, 2);
    tmp.resize(num1 + num2, 4);
    for (unsigned i = 0; i < num1 + num2; i++)
    {
        in >> tmp(i, 0) >> tmp(i, 1) >> tmp(i, 2) >> tmp(i, 3);
        if (i >= num1)
            for (unsigned j = 0; j < 2; j++)
                be(i - num1, j) = tmp(i, j + 2);
    }

    // Формируем КЭ
    for (unsigned i = 0; i < numFE; i++)
        for (unsigned j = 0; j < 3; j++)
            fe(i, j) = UINT_MAX;

    for (unsigned i = 0; i < num1; i++)
    {
        v1 = tmp(i, 2);
        v2 = tmp(i, 3);
        v3 = tmp(i, 0);

        if (v3 > numFE - 1)
            continue;

        if (fe(v3, 0) == UINT_MAX)
        {
            fe(v3, 0) = v1;
            fe(v3, 1) = v2;
        }
        else
        {
            if (fe(v3, 0) == v1 or fe(v3, 1) == v1)
                v1 = UINT_MAX;
            if (fe(v3, 0) == v2 or fe(v3, 1) == v2)
                v2 = UINT_MAX;
            if (v1 not_eq UINT_MAX)
                fe(v3, 2) = v1;
            if (v2 not_eq UINT_MAX)
                fe(v3, 2) = v2;
        }
    }

    for (unsigned i = 0; i < num1; i++)
    {
        v1 = tmp(i, 2);
        v2 = tmp(i, 3);
        v3 = tmp(i, 1);

        if (v3 == UINT_MAX)
            continue;

        if (fe(v3, 0) == UINT_MAX)
        {
            fe(v3, 0) = v1;
            fe(v3, 1) = v2;
        }
        else
        {
            if (fe(v3, 0) == v1 or fe(v3, 1) == v1)
                v1 = UINT_MAX;
            if (fe(v3, 0) == v2 or fe(v3, 1) == v2)
                v2 = UINT_MAX;
            if (v1 not_eq UINT_MAX)
                fe(v3, 2) = v1;
            if (v2 not_eq UINT_MAX)
                fe(v3, 2) = v2;
        }
    }
    in.close();
    feType = FEType::fe2d3;
    if (in.fail())
    {
        cerr << sayError(ErrorCode::EReadFile) << endl;
        return (error = true);
    }
    cout << *this << endl;
    return false;
}
// ---------------------------------------------------------
// Определение минимальной и максимальной координаты объекта
// ----------------------------------------------------------------
void TMesh::getMinMax(void)
{
    array<double, 3> min_x{ x[0][0], (x.size2() > 1) ? x[0][1] : 0, (x.size2() > 2) ? x[0][2] : 0 },
                     max_x{ x[0][0], (x.size2() > 1) ? x[0][1] : 0, (x.size2() > 2) ? x[0][2] : 0 };

    for (unsigned i = 1; i < x.size1(); i++)
    {
        if (x[i][0] < min_x[0])
            min_x[0] = x[i][0];
        if (x[i][0] > max_x[0])
            max_x[0] = x[i][0];
        if (x.size2() > 1)
        {
            if (x[i][1] < min_x[1])
                min_x[1] = x[i][1];
            if (x[i][1] > max_x[1])
                max_x[1] = x[i][1];
        }
        if (x.size2() > 2)
        {
            if (x[i][2] < min_x[2])
                min_x[2] = x[i][2];
            if (x[i][2] > max_x[2])
                max_x[2] = x[i][2];
        }
    }
    minX = min_x;
    maxX = max_x;
}
// ----------------------------------------------------------------
//             Построение вектора нормали к заданному ГЭ
// ----------------------------------------------------------------
void TMesh::normal(unsigned index, vector<double>& v)
{
    double len;

    if (not v.size())
        v.resize(3);
    if (x.size2() == 1)
    {
        v[0] = 1;
        v[1] = v[2] = 0;
        return;
    }
    if (x.size2() == 2 and isPlate())
    {
        v[0] = v[1] = 0;
        v[2] = 1;
        return;
    }
    if (x.size2() == 2 and not isPlate())
    {
        v[0] = x(be(index, 0), 1) - x(be(index, 1), 1);
        v[1] = x(be(index, 1), 0) - x(be(index, 0), 0);
        v[2] = 0;
    }
    else
    {
        v[0] = (x(be(index, 1), 1) - x(be(index, 0), 1))*(x(be(index, 2), 2) - x(be(index, 0), 2)) - (x(be(index, 2), 1) - x(be(index, 0), 1))*(x(be(index, 1), 2) - x(be(index, 0), 2));
        v[1] = (x(be(index, 2), 0) - x(be(index, 0), 0))*(x(be(index, 1), 2) - x(be(index, 0), 2)) - (x(be(index, 1), 0) - x(be(index, 0), 0))*(x(be(index, 2), 2) - x(be(index, 0), 2));
        v[2] = (x(be(index, 1), 0) - x(be(index, 0), 0))*(x(be(index, 2), 1) - x(be(index, 0), 1)) - (x(be(index, 2), 0) - x(be(index, 0), 0))*(x(be(index, 1), 1) - x(be(index, 0), 1));
    }
    // Нормализируем вектор
    len = pow(v[0] * v[0] + v[1] * v[1] + v[2] * v[2], 0.5);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}
// ----------------------------------------------------------------
vector<double> TMesh::surfaceLoadShare(void)
{
    switch (feType)
    {
        case FEType::fe1d2:
            return { 1.0 };
        case FEType::fe2d3:
        case FEType::fe2d4:
            return { 0.5, 0.5 };
        case FEType::fe2d6:
            return { 0.16666666666, 0.16666666666, 0.66666666666 };
        case FEType::fe2d3p:
        case FEType::fe3d3s:
        case FEType::fe3d4:
            return { 0.333333333333, 0.333333333333, 0.333333333333 };
        case FEType::fe2d4p:
        case FEType::fe3d4s:
        case FEType::fe3d8:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FEType::fe2d6p:
        case FEType::fe3d6s:
        case FEType::fe3d10:
            return { 0, 0, 0, 0.333333333333, 0.333333333333, 0.333333333333 };
        default:
            break;
    }
    return {};
}
// ----------------------------------------------------------------
vector<double> TMesh::volumeLoadShare(void)
{
    switch (feType)
    {
        case FEType::fe1d2:
            return { 0.5, 0.5 };
        case FEType::fe2d3:
        case FEType::fe2d3p:
        case FEType::fe3d3s:
            return { 0.33333333333, 0.33333333333, 0.33333333333 };
        case FEType::fe2d4:
        case FEType::fe2d4p:
        case FEType::fe3d4s:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FEType::fe2d6:
        case FEType::fe2d6p:
        case FEType::fe3d6s:
            return { 0, 0, 0, 0.33333333333, 0.33333333333, 0.33333333333 };
        case FEType::fe3d4:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FEType::fe3d8:
            return { 0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125 };
        case FEType::fe3d10:
            return { -0.05, -0.05, -0.05, -0.05, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2 };
        default:
            break;
    }
    return {};
}
// ----------------------------------------------------------------
double TMesh::beVolume(unsigned index)
{
    double ret = 0;
    matrix<double> be_coord;

    getCoordBE(index, be_coord);
    switch (feType)
    {
        case FEType::fe1d2:
            ret = 1.0;
            break;
        case FEType::fe2d3:
        case FEType::fe2d4:
        case FEType::fe2d6:
            ret = volume1d2(be_coord);
            break;
        case FEType::fe2d3p:
        case FEType::fe3d3s:
        case FEType::fe2d6p:
        case FEType::fe3d6s:
        case FEType::fe3d4:
        case FEType::fe3d10:
            ret = volume2d3(be_coord);
            break;
        case FEType::fe2d4p:
        case FEType::fe3d4s:
        case FEType::fe3d8:
            ret = volume2d4(be_coord);
            break;
        default:
            break;
    }
    return ret;
}
// ----------------------------------------------------------------
double TMesh::feVolume(unsigned index)
{
    double ret = 0;
    matrix<double> fe_coord;

    getCoordFE(index, fe_coord);

    switch (feType)
    {
        case FEType::fe1d2:
            ret = volume1d2(fe_coord);
            break;
        case FEType::fe2d3:
        case FEType::fe2d3p:
        case FEType::fe2d6:
        case FEType::fe2d6p:
        case FEType::fe3d3s:
        case FEType::fe3d6s:
            ret = volume2d3(fe_coord);
            break;
        case FEType::fe2d4:
        case FEType::fe2d4p:
        case FEType::fe3d4s:
            ret = volume2d4(fe_coord);
            break;
        case FEType::fe3d4:
        case FEType::fe3d10:
            ret = volume3d4(fe_coord);
            break;
        case FEType::fe3d8:
            ret = volume3d8(fe_coord);
            break;
        default:
            break;
    }
    return ret;
}
// ----------------------------------------------------------------
// Длина отрезка
double TMesh::volume1d2(const matrix<double>& px)
{
    return sqrt(pow(px(0, 0) - px(1, 0), 2));
}
// ----------------------------------------------------------------
// Площадь треугольника
double TMesh::volume2d3(const matrix<double>& px)
{
    double a = sqrt(pow(px(0, 0) - px(1, 0), 2) + pow(px(0, 1) - px(1, 1), 2) + pow(px(0, 2) - px(1, 2), 2)),
           b = sqrt(pow(px(0, 0) - px(2, 0), 2) + pow(px(0, 1) - px(2, 1), 2) + pow(px(0, 2) - px(2, 2), 2)),
           c = sqrt(pow(px(2, 0) - px(1, 0), 2) + pow(px(2, 1) - px(1, 1), 2) + pow(px(2, 2) - px(1, 2), 2)),
           p = 0.5 * (a + b + c);

    return sqrt(p * (p - a) * (p - b) * (p - c));
}
// ----------------------------------------------------------------
// Площадь четырехугольника
double TMesh::volume2d4(const matrix<double>& px)
{
    double a = sqrt(pow(px(0, 0) - px(1, 0), 2) + pow(px(0, 1) - px(1, 1), 2) + pow(px(0, 2) - px(1, 2), 2)),
           b = sqrt(pow(px(2, 0) - px(1, 0), 2) + pow(px(2, 1) - px(1, 1), 2) + pow(px(2, 2) - px(1, 2), 2)),
           c = sqrt(pow(px(0, 0) - px(2, 0), 2) + pow(px(0, 1) - px(2, 1), 2) + pow(px(0, 2) - px(2, 2), 2)),
           p = 0.5 * (a + b + c),
           v = sqrt(p * (p - a) * (p - b) * (p - c));

    a = sqrt(pow(px(0, 0) - px(3, 0), 2) + pow(px(0, 1) - px(3, 1), 2) + pow(px(0, 2) - px(3, 2), 2));
    b = sqrt(pow(px(2, 0) - px(3, 0), 2) + pow(px(2, 1) - px(3, 1), 2) + pow(px(2, 2) - px(3, 2), 2));
    p = 0.5 * (a + b + c);
    return v + sqrt(p * (p - a) * (p - b) * (p - c));
}
// ----------------------------------------------------------------
// Объем пирамиды
double TMesh::volume3d4(const matrix<double>& px)
{
    matrix<double> m = {{ px(1, 0) - px(0, 0), px(1, 1) - px(0, 1), px(1, 2) - px(0, 2) },
                        { px(2, 0) - px(0, 0), px(2, 1) - px(0, 1), px(2, 2) - px(0, 2) },
                        { px(3, 0) - px(0, 0), px(3, 1) - px(0, 1), px(3, 2) - px(0, 2) }};
    return fabs(det3x3(m)) / 6;
}
// ----------------------------------------------------------------
// Объем четырехугольного шестигранника
double TMesh::volume3d8(const matrix<double>& px)
{
    matrix<int> ref = {{0, 1, 4, 7}, {4, 1, 5, 7}, {1, 2, 6, 7}, {1, 5, 6, 7}, {1, 2, 3, 7}, {0, 3, 1, 7}};
    matrix<double> m;
    double v = 0;

    for (int i = 0; i < 6; i++)
    {
        m = {{ px(ref(i, 1), 0) - px(ref(i, 0), 0), px(ref(i, 1), 1) - px(ref(i, 0), 1), px(ref(i, 1), 2) - px(ref(i, 0), 2) },
             { px(ref(i, 2), 0) - px(ref(i, 0), 0), px(ref(i, 2), 1) - px(ref(i, 0), 1), px(ref(i, 2), 2) - px(ref(i, 0), 2) },
             { px(ref(i, 3), 0) - px(ref(i, 0), 0), px(ref(i, 3), 1) - px(ref(i, 0), 1), px(ref(i, 3), 2) - px(ref(i, 0), 2) }};
        v += fabs(det3x3(m)) / 6;
    }
    return v;
}
// -------------------------------------------------------
bool TMesh::readTetgen(string fname)
{
    fstream in;
    int num,
        tmp;
    string name;

    clear();

    // Чтение узлов (*.node)
    name = fname.substr(0, fname.find_last_of('.')) + ".node";
    in.open(name, ios::in);
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }
    in >> num >> tmp >> tmp >> tmp;
    if (in.fail() or num == 0)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    x.resize(num, 3);
    for (auto i = 0; i < num; i++)
    {
        in >> tmp >> x(i, 0) >> x(i, 1) >> x(i, 2);
        if (in.fail())
        {
            in.close();
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    in.close();

    // Чтение КЭ (*.ele)
    name = fname.substr(0, fname.find_last_of('.')) + ".ele";
    in.open(name, ios::in);
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }

    in >> num >> tmp >> tmp;
    if (in.fail() or num == 0)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    fe.resize(num, 4);
    for (auto i = 0; i < num; i++)
    {
        in >> tmp >> fe(i, 0) >> fe(i, 1) >> fe(i, 2) >> fe(i, 3);
        fe(i, 0)--; fe(i, 1)--; fe(i, 2)--; fe(i, 3)--;
        if (in.fail())
        {
            in.close();
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    in.close();

    // Чтение ГЭ (*.face)
    name = fname.substr(0, fname.find_last_of('.')) + ".face";
    in.open(name, ios::in);
    if (not in.is_open())
    {
        cerr << sayError(ErrorCode::EOpenFile) << endl;
        return (error = true);
    }
    in >> num >> tmp;
    if (in.fail() or num == 0)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    be.resize(num, 3);
    for (auto i = 0; i < num; i++)
    {
        in >> tmp >> be(i, 0) >> be(i, 1) >> be(i, 2) >> tmp;
        be(i, 0)--; be(i, 1)--; be(i, 2)--;
        if (in.fail())
        {
            in.close();
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    in.close();
    feType = FEType::fe3d4;
    cout << *this << endl;
    return false;
}
// -------------------------------------------------------
//          Создание списка связей узлов сетки
// -------------------------------------------------------
void TMesh::createMeshMap(void)
{
    meshMap.resize(x.size1());
    msg->setProcess(ProcessCode::AnalysingMesh, 1, int(fe.size1()));
    for (unsigned i = 0; i < fe.size1(); msg->addProgress(), i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            for (unsigned k = 0; k < fe.size2(); k++)
                // if (k not_eq j)
                    if (find(meshMap[fe(i, j)].begin(), meshMap[fe(i, j)].end(), fe(i, k)) == meshMap[fe(i, j)].end())
                        meshMap[fe(i, j)].push_back(fe(i, k));

    for (unsigned i = 0; i < meshMap.size(); i++)
        sort (meshMap[i].begin(), meshMap[i].end(), [](unsigned k, unsigned l) -> bool{ return (k < l); });
    msg->stopProcess();
}
// ----------------------------------------------------------------
