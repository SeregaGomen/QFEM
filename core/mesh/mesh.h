#ifndef MESH_H
#define MESH_H

#include <fstream>
#include <list>
#include "fe/fe.h"
#include "util/matrix.h"

using namespace std;

//------------------------------------------------
// Описание конечно-элементного разбиения объекта
//------------------------------------------------
class TMesh
{
private:
    // Признак наличия ошибки при работе с сеткой
    bool error;
    // Тип КЭ
    FEType feType;
    // Минимальное значение координат
    vector<double> minX = {0, 0, 0};
    vector<double> maxX = {0, 0, 0};
    // Координаты вершин сетки
    matrix<double> x;
    // Связи между узлами в КЭ
    matrix<unsigned> fe;
    // Связи между узлами граничных элементов
    matrix<unsigned> be;
    void createFE(void);
    void createMeshMap(void);
    void getMinMax(void);
    bool readTRP(string);
    bool readVOL(string);
    bool readMESH(string);
    bool readTRPA(string);
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
        feType = NOTYPE;
        error = false;
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
        return x(i, j);
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

        if (feType == FE2D6 || feType == FE2D6P || feType == FE3D6S)
            ret = 3;
        else if (feType == FE3D10)
            ret = 4;
        return ret;
    }
    unsigned getNumFE(void)
    {
        return fe.size1();
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

        if (feType == FE2D6)
            ret = 2;
        else if (feType == FE3D10 || feType == FE2D6P || feType == FE3D6S)
            ret = 3;
        return ret;
    }
    unsigned getNumBE(void)
    {
        return be.size1();
    }
    void getCoordVertex(unsigned i, vector<double>& coord)
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
            case FE1D2:
                ret = 1;
                break;
            case FE2D3:
            case FE2D4:
            case FE2D6:
                ret = 2;
                break;
            case FE2D3P:
            case FE2D4P:
            case FE2D6P:
            case FE3D4:
            case FE3D8:
            case FE3D10:
                ret = 3;
                break;
            case FE3D3S:
            case FE3D4S:
            case FE3D6S:
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
    void setMesh(FEType p_type, matrix<double>& p_x, matrix<unsigned>& p_fe, matrix<unsigned>& p_be)
    {
        clear();
        feType = p_type;
        x = p_x;
        fe = p_fe;
        be = p_be;
        getMinMax();
    }
    void clear(void)
    {
        feType = NOTYPE;
        error = false;
        x.clear();
        fe.clear();
        be.clear();
    }
    void getCoordFE(unsigned index, matrix<double>& coord)
    {
        coord.resize(fe.size2(), 3);

        for (unsigned i = 0; i < fe.size2(); i++)
            for (unsigned j = 0; j < x.size2(); j++)
                coord(i, j) = x(fe(index, i), j);
    }
    void getCenterFE(unsigned index, vector<double>& coord)
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
    void getCoordBE(unsigned index, matrix<double>& coord)
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
    bool is1D(void)
    {
        return (feType == FE1D2) ? true : false;
    }
    bool is2D(void)
    {
        return (feType == FE2D3 || feType == FE2D4 || feType == FE2D6) ? true : false;
    }
    bool is3D(void)
    {
        return (feType == FE3D4 || feType == FE3D8 || feType == FE3D10) ? true : false;
    }
    bool isPlate(void)
    {
        return (feType == FE2D3P || feType == FE2D4P || feType == FE2D6P) ? true : false;
    }
    bool isShell(void)
    {
        return (feType == FE3D3S || feType == FE3D4S || feType == FE3D6S) ? true : false;
    }
    void normal(unsigned, vector<double>&);
    vector<double> surfaceLoadShare(void);
    vector<double> volumeLoadShare(void);
    double beVolume(unsigned);
    double feVolume(unsigned);
};

#endif // MESH_H
