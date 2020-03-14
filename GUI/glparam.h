#ifndef GLPARAM_H
#define GLPARAM_H

#include <QVector4D>
#include "glfun.h"

class TMesh;

class TGLParameter : public TGLFunction
{
    Q_OBJECT
private:
    int paramType;
    QVector<QVector4D> vertex;
    void getMinMax(float &min, float &max)
    {
        min = std::numeric_limits<float>::max();
        max = std::numeric_limits<float>::min();
        foreach (QVector4D v, vertex)
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
    TGLParameter(TMesh*, QVector<QVector4D>&, int, QWidget* = nullptr);
    ~TGLParameter(void) {}
    void redraw(QVector<QVector4D>& v)
    {
        vertex = v;
        initColorTable();
        repaint();
    }
};

#endif // GLPARAM_H
