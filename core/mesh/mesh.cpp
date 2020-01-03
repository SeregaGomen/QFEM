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
    pos = fname.find_last_of('.');
    if ((pos = fname.find_last_of('.')) == string::npos)
    {
        cerr << sayError(UNKNOWN_FILE_ERR) << endl;
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
    else
    {
        cerr << sayError(UNKNOWN_FILE_ERR) << endl;
        return (error = true);
    }
    if (!error)
        getMinMax();


    // writeTRPA("shell-tube.trpa");

    // writeTRPA("plate3.trpa");
    // writeTRPA("tank3s.trpa");
    // writeTRPA("tank3d.trp");
    // writeTRP("rod.trp");
    return error;
}
// ------------- Определение параметров КЭ ----------------------
FEType TMesh::getDataFE(unsigned type, unsigned& feSize, unsigned& beSize, unsigned& feDim)
{
    FEType ret;

    switch (type)
    {
        case 3:  /* FE2D3 */
            ret = FE2D3;
            beSize = 2;
            feSize = 3;
            feDim = 2;
            break;
        case 4:  /* FE3D4 */
            ret = FE3D4;
            beSize = 3;
            feSize = 4;
            feDim = 3;
            break;
        case 6:  /* FE2D6 */
            ret = FE2D6;
            beSize = 3;
            feSize = 6;
            feDim = 2;
            break;
        case 8:  /* FE3D8 */
            ret = FE3D8;
            beSize = 4;
            feSize = 8;
            feDim = 3;
            break;
        case 10:  /* FE3D10 */
            ret = FE3D10;
            beSize = 6;
            feSize = 10;
            feDim = 3;
            break;
        case 24: /* FE2D4 */
            ret = FE2D4;
            beSize = 2;
            feSize = 4;
            feDim = 2;
            break;
        case 34: /* FE1D2 */
            ret = FE1D2;
            beSize = 1;
            feSize = 2;
            feDim = 1;
            break;
        case 123:  /* FE2D3P */
            ret = FE2D3P;
            beSize = 0;
            feSize = 3;
            feDim = 2;
            break;
        case 124:  /* FE2D4P */
            ret = FE2D4P;
            beSize = 0;
            feSize = 4;
            feDim = 2;
            break;
        case 125:  /* FE2D6P */
            ret = FE2D6P;
            beSize = 0;
            feSize = 6;
            feDim = 2;
            break;
        case 223:  /* FE3D3S */
            ret = FE3D3S;
            beSize = 0;
            feSize = 3;
            feDim = 3;
            break;
        case 224:  /* FE3D4S */
            ret = FE3D4S;
            beSize = 0;
            feSize = 4;
            feDim = 3;
            break;
        case 225:  /* FE3D6S */
            ret = FE3D6S;
            beSize = 0;
            feSize = 6;
            feDim = 3;
            break;
        default:
            feSize = beSize = feDim = 0;
            ret = NOTYPE;
    }
    return ret;
}
// -------------- Конвертация типов КЭ (для совместимости) -----------------
unsigned TMesh::convertFEType(FEType p)
{
    unsigned ret;

    switch (p)
    {
        case FE1D2:
            ret = 34;
            break;
        case FE2D3:
            ret = 3;
            break;
        case FE2D4:
            ret = 24;
            break;
        case FE2D6:
            ret = 6;
            break;
        case FE2D3P:
            ret = 123;
            break;
        case FE2D4P:
            ret = 124;
            break;
        case FE2D6P:
            ret = 125;
            break;
        case FE3D4:
            ret = 4;
            break;
        case FE3D8:
            ret = 8;
            break;
        case FE3D10:
            ret = 10;
            break;
        case FE3D3S:
            ret = 223;
            break;
        case FE3D4S:
            ret = 224;
            break;
        case FE3D6S:
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
    pos = fname.find_last_of('.');
    if ((pos = fname.find_last_of('.')) == string::npos)
    {
        cerr << sayError(UNKNOWN_FILE_ERR) << endl;
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
        cerr << sayError(UNKNOWN_FILE_ERR) << endl;
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
        case NOTYPE:
            ret = S_MSG_NO_TYPE;
            break;
        case FE1D2:
            ret = S_MSG_FE1D2;
            break;
        case FE2D3:
            ret = S_MSG_FE2D3;
            break;
        case FE2D4:
            ret = S_MSG_FE2D4;
            break;
        case FE2D6:
            ret = S_MSG_FE2D6;
            break;
        case FE2D3P:
            ret = S_MSG_FE2D3_PLATE;
            break;
        case FE2D4P:
            ret = S_MSG_FE2D4_PLATE;
            break;
        case FE2D6P:
            ret = S_MSG_FE2D6_PLATE;
            break;
        case FE3D4:
            ret = S_MSG_FE3D4;
            break;
        case FE3D8:
            ret = S_MSG_FE3D8;
            break;
        case FE3D10:
            ret = S_MSG_FE3D10;
            break;
        case FE3D3S:
            ret = S_MSG_FE3D3_SHELL;
            break;
        case FE3D4S:
            ret = S_MSG_FE3D4_SHELL;
            break;
        case FE3D6S:
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
    ifstream in;
    char signature[7];
    unsigned temp,
             surfaceSize,
             feSize,
             feDim;


    clear();
    in.open(fname.c_str(), ios::in | ios::binary);
    if (in.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
        return (error = true);
    }

    // Cчитываем сигнатуру
    in.read(signature, 6*sizeof(char));
    signature[6] = 0;
    if (string(signature) != "NTRout")
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
        return (error = true);
    }
    // Cчитываем тип КЭ
    in.read(reinterpret_cast<char*>(&temp), sizeof(unsigned));
    if ((feType = getDataFE(temp, feSize, surfaceSize, feDim)) == NOTYPE)
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
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
        cerr << sayError(READ_FILE_ERR) << endl;
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
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    // Cчитываем грани
    for (unsigned i = 0; i < be.size1(); i++)
        for (unsigned j = 0; j < be.size2(); j++)
            in.read(reinterpret_cast<char*>(&be(i, j)), sizeof(unsigned));
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    in.close();
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    cout << *this << endl;
    return false;
}
// ------------------- Чтение TRPA-файла -----------------------
bool TMesh::readTRPA(string fname)
{
    fstream in;
    unsigned temp,
             surfaceSize,
             feSize,
             feDim;

    clear();
    in.open(fname.c_str(), ios::in | ios::binary);
    if (in.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
        return (error = true);
    }
    in >> temp;
    if ((feType = getDataFE(temp, feSize, surfaceSize, feDim)) == NOTYPE)
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
        return (error = true);
    }
    in >> temp;
    if (in.fail() || temp == 0 || feDim < 1 || feDim > 3)
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
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
            cerr << sayError(READ_FILE_ERR) << endl;
            return (error = true);
        }
    }
    in >> temp;
    if (in.fail() || temp == 0)
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
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
                cerr << sayError(READ_FILE_ERR) << endl;
                return (error = true);
            }
        }
    in >> temp;
    if (in.fail() || (temp == 0 && (feType == FE2D3 || feType == FE2D4 || feType == FE3D4 || feType == FE3D8)))
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
        return (error = true);
    }
    if (isPlate() || isShell())
        be = fe;
    else // if (feDim != 1)
    {
        be.resize(temp, surfaceSize);
        for (unsigned i = 0; i < temp; i++)
            for (unsigned j = 0; j < surfaceSize; j++)
            {
                in >> be(i, j);
                if (in.fail())
                {
                    in.close();
                    cerr << sayError(READ_FILE_ERR) << endl;
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
    ofstream out;
    const char* signature = "NTRout";
    unsigned temp = 0;

    out.open(fname.c_str(), ios::out | ios::binary);
    if (out.bad())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
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
        cerr << sayError(WRITE_FILE_ERR) << endl;
        return (error = true);
    }
    out.close();
    return false;
}
// -----------------------------------------------------
bool TMesh::writeTRPA(string fname)
{
    ofstream out;

    out.open(fname.c_str(), ios::out);
    if (out.bad())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
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
        cerr << sayError(WRITE_FILE_ERR) << endl;
        return (error = true);
    }
    out.close();
    return false;
}
// -----------------------------------------------------
bool TMesh::write(ofstream& out)
{
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
    if (isPlate() || isShell())
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
            cerr << sayError(WRITE_FILE_ERR) << endl;
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

    // Cчитываем тип КЭ
    in >> val;
    getline(in, str);
    if ((feType = getDataFE(val, feSize, surfaceSize, feDim)) == NOTYPE)
    {
        in.close();
        cerr << sayError(FORMAT_FILE_ERR) << endl;
        return (error = true);
    }
    // Считываем кол-во узлов
    in >> val;
    getline(in, str);
    x.resize(val, feDim);
    // Cчитываем узлы
    for (unsigned i = 0; i < x.size1(); i++)
    {
        for (unsigned j = 0; j < x.size2(); j++)
            in >> x(i, j);
        getline(in, str);
    }
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    // Cчитываем КЭ
    in >> val;
    getline(in, str);
    fe.resize(val,feSize);
    for (unsigned i = 0; i < fe.size1(); i++)
    {
        for (unsigned j = 0; j < fe.size2(); j++)
            in >> fe(i, j);
        getline(in, str);
    }
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    if (isPlate() || isShell())
        be = fe;
    else
    {
        // Cчитываем ГЭ
        in >> val;
        getline(in, str);
        be.resize(val,surfaceSize);
        for (unsigned i = 0; i < be.size1(); i++)
        {
            for (unsigned j = 0; j < be.size2(); j++)
                in >> be(i, j);
            getline(in, str);
        }
        if (in.fail())
        {
            cerr << sayError(READ_FILE_ERR) << endl;
            return (error = true);
        }
    }
    if (!error)
    {
        cout << *this << endl;
        getMinMax();
    }
    return error;
}
// --------------- Чтение VOL-файла (NETGEN) ----------------------
bool TMesh::readVOL(string fname)
{
    const int maxLen = 1001;
    ifstream in;
    char str[maxLen];
    unsigned num,
             tmp,
             i1,
             i2,
             i3,
             i4;

    clear();
    in.open(fname.c_str(), ios::in);
    if (in.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
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
    feType = FE3D4;
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
        return (error = true);
    }
    cout << *this << endl;
    return false;
}
// --------------- Чтение MESH-файла (GRUMMP) ----------------------
bool TMesh::readMESH(string fname)
{
    ifstream in;
    unsigned num1,
             num2,
             numFE,
             numVertex,
             v1,
             v2,
             v3;
    matrix<unsigned> tmp;

    clear();
    in.open(fname.c_str(), ios::in);
    if (in.fail())
    {
        cerr << sayError(OPEN_FILE_ERR) << endl;
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
            if (fe(v3, 0) == v1 || fe(v3, 1) == v1)
                v1 = UINT_MAX;
            if (fe(v3, 0) == v2 || fe(v3, 1) == v2)
                v2 = UINT_MAX;
            if (v1 != UINT_MAX)
                fe(v3, 2) = v1;
            if (v2 != UINT_MAX)
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
            if (fe(v3, 0) == v1 || fe(v3, 1) == v1)
                v1 = UINT_MAX;
            if (fe(v3, 0) == v2 || fe(v3, 1) == v2)
                v2 = UINT_MAX;
            if (v1 != UINT_MAX)
                fe(v3, 2) = v1;
            if (v2 != UINT_MAX)
                fe(v3, 2) = v2;
        }
    }
    in.close();
    feType = FE2D3;
    if (in.fail())
    {
        cerr << sayError(READ_FILE_ERR) << endl;
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
    vector<double> min_x{ x[0][0], (x.size2() > 1) ? x[0][1] : 0, (x.size2() > 2) ? x[0][2] : 0 },
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

    if (!v.size())
        v.resize(3);
    if (x.size2() == 1)
    {
        v[0] = 1;
        v[1] = v[2] = 0;
        return;
    }
    if (x.size2() == 2 && isPlate())
    {
        v[0] = v[1] = 0;
        v[2] = 1;
        return;
    }
    if (x.size2() == 2 && !isPlate())
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
        case FE1D2:
            return { 1.0 };
        case FE2D3:
        case FE2D4:
            return { 0.5, 0.5 };
        case FE2D6:
            return { 0.16666666666, 0.16666666666, 0.66666666666 };
        case FE2D3P:
        case FE3D3S:
        case FE3D4:
            return { 0.333333333333, 0.333333333333, 0.333333333333 };
        case FE2D4P:
        case FE3D4S:
        case FE3D8:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FE2D6P:
        case FE3D6S:
        case FE3D10:
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
        case FE1D2:
            return { 0.5, 0.5 };
        case FE2D3:
        case FE2D3P:
        case FE3D3S:
            return { 0.33333333333, 0.33333333333, 0.33333333333 };
        case FE2D4:
        case FE2D4P:
        case FE3D4S:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FE2D6:
        case FE2D6P:
        case FE3D6S:
            return { 0, 0, 0, 0.33333333333, 0.33333333333, 0.33333333333 };
        case FE3D4:
            return { 0.25, 0.25, 0.25, 0.25 };
        case FE3D8:
            return { 0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125 };
        case FE3D10:
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
        case FE1D2:
            ret = 1.0;
            break;
        case FE2D3:
        case FE2D4:
        case FE2D6:
            ret = volume1d2(be_coord);
            break;
        case FE2D3P:
        case FE3D3S:
        case FE2D6P:
        case FE3D6S:
        case FE3D4:
        case FE3D10:
            ret = volume2d3(be_coord);
            break;
        case FE2D4P:
        case FE3D4S:
        case FE3D8:
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
        case FE1D2:
            ret = volume1d2(fe_coord);
            break;
        case FE2D3:
        case FE2D3P:
        case FE2D6:
        case FE2D6P:
        case FE3D3S:
        case FE3D6S:
            ret = volume2d3(fe_coord);
            break;
        case FE2D4:
        case FE2D4P:
        case FE3D4S:
            ret = volume2d4(fe_coord);
            break;
        case FE3D4:
        case FE3D10:
            ret = volume3d4(fe_coord);
            break;
        case FE3D8:
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
    double a = (px(1, 0) - px(0, 0)) * (px(2, 1) - px(0, 1)) * (px(3, 2) - px(0, 2)) + (px(3, 0) - px(0, 0)) * (px(1, 1) - px(0, 1)) * (px(2, 2) - px(0, 2)) + (px(2, 0) - px(0, 0)) * (px(3, 1) - px(0, 1)) * (px(1, 2) - px(0, 2)),
           b = (px(3, 0) - px(0, 0)) * (px(2, 1) - px(0, 1)) * (px(1, 2) - px(0, 2)) + (px(2, 0) - px(0, 0)) * (px(1, 1) - px(0, 1)) * (px(3, 2) - px(0, 2)) + (px(1, 0) - px(0, 0)) * (px(3, 1) - px(0, 1)) * (px(2, 2) - px(0, 2));

    return (a - b) / 6.0;
}
// ----------------------------------------------------------------
// Объем четырехугольного шестигранника
double TMesh::volume3d8(const matrix<double>& px)
{
    double v = 0;
    unsigned ref[][6] = {{0, 1, 4, 7}, {4, 1, 5, 7}, {1, 2, 6, 7}, {1, 5, 6, 7}, {1, 2, 3, 7}, {0, 3, 1, 7}};

    for (int i = 0; i < 6; i++)
        v += (px(ref[i][1], 0) - px(ref[i][0], 0)) * (px(ref[i][2], 1) - px(ref[i][0], 1)) * (px(ref[i][3], 2) - px(ref[i][0], 2)) +
             (px(ref[i][1], 1) - px(ref[i][0], 1)) * (px(ref[i][2], 2) - px(ref[i][0], 2)) * (px(ref[i][3], 0) - px(ref[i][0], 0)) +
             (px(ref[i][1], 2) - px(ref[i][0], 2)) * (px(ref[i][2], 0) - px(ref[i][0], 0)) * (px(ref[i][3], 1) - px(ref[i][0], 1)) -
             (px(ref[i][1], 2) - px(ref[i][0], 2)) * (px(ref[i][2], 1) - px(ref[i][0], 1)) * (px(ref[i][3], 0) - px(ref[i][0], 0)) -
             (px(ref[i][1], 1) - px(ref[i][0], 1)) * (px(ref[i][2], 0) - px(ref[i][0], 0)) * (px(ref[i][3], 2) - px(ref[i][0], 2)) -
             (px(ref[i][1], 0) - px(ref[i][0], 0)) * (px(ref[i][2], 2) - px(ref[i][0], 2)) * (px(ref[i][3], 1) - px(ref[i][0], 1)) / 6.0;
    return v;
}
// ----------------------------------------------------------------
