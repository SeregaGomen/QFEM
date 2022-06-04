#ifndef GLPARAM_H
#define GLPARAM_H

#include <QVector4D>
#include "glfun.h"

class TMesh;
enum class ParamType;

class TGLParameter : public TGLFunction
{
    Q_OBJECT
private:
    ParamType paramType;
    QVector<QVector4D> vertex;
    void getMinMax(double min[2], double max[2])
    {
        min[0] = std::numeric_limits<float>::max();
        max[0] = std::numeric_limits<float>::min();
        for (auto v: vertex)
        {
            min[0] = qMin(min[0], v.w());
            max[0] = qMax(max[0], v.w());
        }
        min[1] = -qMax(abs(min[0]), abs(max[0]));
        max[1] = abs(min[1]);
    }
    void createLoads(void);
    void createBoundaryConditions(void);
    double getValue(unsigned i)
    {
        return double(vertex[int(i)].w());
    }

protected:
    void createObject(void);

public:
    TGLParameter(TMesh*, QVector<QVector4D>&, ParamType, QWidget* = nullptr);
    ~TGLParameter(void) {}
    void redraw(QVector<QVector4D>& v)
    {
        vertex = v;
        initColorTable();
        repaint();
    }
    ParamType getType(void) const
    {
        return paramType;
    }
};

#endif // GLPARAM_H
