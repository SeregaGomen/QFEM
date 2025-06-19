#ifndef MESH_H
#define MESH_H

#include <fstream>
#include <array>
#include "fe/fe.h"
#include "util/matrix.h"

using namespace std;

//------------------------------------------------
// Описание конечно-элементного разбиения объекта
//------------------------------------------------
class TMesh
{
private:

    bool error;                         // Признак наличия ошибки при работе с сеткой
    FEType feType;                      // Тип КЭ
    array<double, 3> minX{0, 0, 0};     // Минимальное значение координат
    array<double, 3> maxX{0, 0, 0};
    vector<vector<unsigned>> meshMap;   // Структура сетки
    matrix<double> x;                   // Координаты вершин сетки
    matrix<unsigned> fe;                // Связи между узлами в КЭ
    matrix<unsigned> be;                // Связи между узлами граничных элементов
    void createFE(void);
    void getMinMax(void);
    bool readTRP(string);
    bool readVOL(string);
    bool readMESH(string);
    bool readMSH(string);
    bool readTRPA(string);
    bool readTetgen(string);
    bool writeTRP(string);
    bool writeTRPA(string);
    double volume1d2(const matrix<double>&);
    double volume2d3(const matrix<double>&);
    double volume2d4(const matrix<double>&);
    double volume3d4(const matrix<double>&);
    double volume3d8(const matrix<double>&);
public:
    TMesh(void)
    {
        feType = FEType::undefined;
        error = false;
    }
    TMesh(FEType type, matrix<double> &x, matrix<unsigned> &fe, matrix<unsigned> &be)
    {
        setMesh(type, x, fe, be);
    }
    ~TMesh(void)
    {
        clear();
    }
    bool isError(void)
    {
        return error;
    }
    FEType getTypeFE(void)
    {
        return feType;
    }
    unsigned getNumVertex(void)
    {
        return unsigned(x.size1());
    }
    double getX(unsigned i, unsigned j)
    {
        return j < x.size2() ? x(i, j) : 0;
    }
    unsigned* getDataFE(void)
    {
        return fe.data();
    }
    double getMinX(unsigned i)
    {
        return minX[i];
    }
    double getMaxX(unsigned i)
    {
        return maxX[i];
    }
    unsigned getSizeFE(void)
    {
        return fe.size2();
    }
    unsigned getBaseSizeFE(void)
    {
        unsigned ret = fe.size2();

        if (feType == FEType::fe2d6 or feType == FEType::fe2d6p or feType == FEType::fe3d6s)
            ret = 3;
        else if (feType == FEType::fe3d10)
            ret = 4;
        return ret;
    }
    unsigned getNumFE(void)
    {
        return fe.size1();
    }
    matrix<unsigned>& getFE(void)
    {
        return fe;
    }
    matrix<unsigned>& getBE(void)
    {
        return be;
    }
    unsigned getFE(unsigned i, unsigned j)
    {
        return fe(i, j);
    }
    unsigned getSizeBE(void)
    {
        return be.size2();
    }
    unsigned getBaseSizeBE(void)
    {
        unsigned ret = be.size2();

        if (feType == FEType::fe2d6)
            ret = 2;
        else if (feType == FEType::fe3d10 or feType == FEType::fe2d6p or feType == FEType::fe3d6s)
            ret = 3;
        return ret;
    }
    unsigned getNumBE(void)
    {
        return be.size1();
    }
    void createMeshMap(void);
    void getCoordVertex(unsigned i, vector<double> &coord)
    {
//        coord.resize(x.size2());
        coord.resize(3);
        for (unsigned j = 0; j < x.size2(); j++)
            coord[j] = x(i, j);
    }
    unsigned getBE(unsigned i, unsigned j)
    {
        return be(i, j);
    }
    unsigned getDimension(void)
    {
        return x.size2();
    }
    unsigned getFreedom(void)
    {
        unsigned ret = 0;

        switch (feType)
        {
            case FEType::fe1d2:
                ret = 1;
                break;
            case FEType::fe2d3:
            case FEType::fe2d4:
            case FEType::fe2d6:
                ret = 2;
                break;
            case FEType::fe2d3p:
            case FEType::fe2d4p:
            case FEType::fe2d6p:
            case FEType::fe3d4:
            case FEType::fe3d8:
            case FEType::fe3d10:
                ret = 3;
                break;
            case FEType::fe3d3s:
            case FEType::fe3d4s:
            case FEType::fe3d6s:
                ret = 6;
                break;
            default:
                ret = 0;
        }
        return ret;
    }
    unsigned convertFEType(FEType);
    bool read(ifstream&);
    bool write(ofstream&);
    bool read(string);
    bool write(string);
    friend ostream& operator << (ostream&, TMesh&);
    string feName(void);
    bool setMesh(string file)
    {
        return read(file);
    }
    void setMesh(FEType type, matrix<double> &x, matrix<unsigned> &fe, matrix<unsigned> &be)
    {
        clear();
        this->feType = type;
        this->x = x;
        this->fe = fe;
        this->be = be;
        getMinMax();
        createMeshMap();
    }
    void clear(void)
    {
        feType = FEType::undefined;
        error = false;
        x.clear();
        fe.clear();
        be.clear();
        meshMap.clear();
    }
    void getCoordFE(unsigned index, matrix<double> &coord)
    {
        coord.resize(fe.size2(), 3);

        for (unsigned i = 0; i < fe.size2(); i++)
            for (unsigned j = 0; j < x.size2(); j++)
                coord(i, j) = x(fe(index, i), j);
    }
    void getCenterFE(unsigned index, vector<double> &coord)
    {
        double c;

//        coord.resize(x.size2());
        coord.resize(3);
        for (unsigned j = 0; j < x.size2(); j++)
        {
            c = 0;
            for (unsigned i = 0; i < getBaseSizeFE(); i++)
                c += x(fe(index, i), j);
            coord[j] = c / double(getBaseSizeFE());
        }
    }
    void getCoordBE(unsigned index, matrix<double> &coord)
    {
        coord.resize(be.size2(), 3);

        for (unsigned i = 0; i < be.size2(); i++)
            for (unsigned j = 0; j < x.size2(); j++)
                coord(i, j) = x(be(index, i), j);
    }
    void getCenterBE(unsigned index, vector<double>& coord)
    {
        double c;

//        coord.resize(x.size2());
        coord.resize(3);
        for (unsigned j = 0; j < x.size2(); j++)
        {
            c = 0;
            for (unsigned i = 0; i < getBaseSizeBE(); i++)
                c += x(be(index, i), j);
            coord[j] = c / double(getBaseSizeBE());
        }
    }
    FEType getDataFE(unsigned, unsigned&, unsigned&, unsigned&);
    FEType getDataFE(string, unsigned&, unsigned&, unsigned&);
    bool is1D(void)
    {
        return (feType == FEType::fe1d2) ? true : false;
    }
    bool is2D(void)
    {
        return (feType == FEType::fe2d3 or feType == FEType::fe2d4 or feType == FEType::fe2d6) ? true : false;
    }
    bool is3D(void)
    {
        return (feType == FEType::fe3d4 or feType == FEType::fe3d8 or feType == FEType::fe3d10) ? true : false;
    }
    bool isPlate(void)
    {
        return (feType == FEType::fe2d3p or feType == FEType::fe2d4p or feType == FEType::fe2d6p) ? true : false;
    }
    bool isShell(void)
    {
        return (feType == FEType::fe3d3s or feType == FEType::fe3d4s or feType == FEType::fe3d6s) ? true : false;
    }
    vector<double> normal(unsigned);
    vector<double> surfaceLoadShare(void);
    vector<double> volumeLoadShare(void);
    vector<unsigned> &getMeshMap(unsigned i)
    {
        return meshMap[i];
    }
    vector<vector<unsigned>> &getMeshMap(void)
    {
        return meshMap;
    }
    double beVolume(unsigned);
    double feVolume(unsigned);
};

#endif // MESH_H
