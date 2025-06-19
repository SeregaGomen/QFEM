#ifndef PARAMVIEW_H
#define PARAMVIEW_H

#include <QVector4D>
#include "funcview.h"

class TMesh;
enum class ParamType;

class TParameterView : public TFunctionView
{
    Q_OBJECT
private:
    int numPoints{0};
    QVector<QVector4D> *vertex;
    ParamType paramType;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    void createLoads(void);
    void createBoundaryConditions(void);
    double getValue(unsigned i) override
    {
        return double((*vertex)[int(i)].w());
    }

protected:
    void initObject(void) override;
    void getMinMax() override
    {
        min_u[0] = max_u[0] = (*vertex)[0].w();
        for (auto i = 1; i < vertex->size(); i++)
        {
            min_u[0] = qMin(min_u[0], (*vertex)[i].w());
            max_u[0] = qMax(max_u[0], (*vertex)[i].w());
        }
        min_u[1] = -qMax(abs(min_u[0]), abs(max_u[0]));
        max_u[1] = abs(min_u[1]);
    }
    void paintGL() override;

public:
    TParameterView(TMesh*, QVector<QVector4D>*, ParamType, QWidget* = nullptr);
    virtual ~TParameterView(void);
    void redraw(QVector<QVector4D> *v)
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

#endif // PARAMVIEW_H
