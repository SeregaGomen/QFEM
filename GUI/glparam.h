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
    void getMinMax(float &min, float &max)
    {
        min = std::numeric_limits<float>::max();
        max = std::numeric_limits<float>::min();
        for (auto v: vertex)
        {
            min = qMin(min, v.w());
            max = qMax(max, v.w());
        }
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
