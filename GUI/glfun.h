#ifndef GLFUN_H
#define GLFUN_H

#include "glmesh.h"

class TMesh;

class TGLFunction : public TGLMesh
{
    Q_OBJECT
private:
    float min_u = 0;
    float max_u = 0;
    vector<double> *results;    // Результаты расчета
    vector<double> *dx;         // Смещения
    vector<double> *dy;         // ...
    vector<double> *dz;         // ...
    QString expression;         // Выражение, с помощью которого вычислена визуализируемая функция
    QVector<QColor> colorTable;

    virtual void createObject(void);
    virtual void getMinMax(float &min, float &max)
    {
        min = float(*min_element(results->begin(), results->end()));
        max = float(*max_element(results->begin(), results->end()));
    }
    void drawSegment(QVector<QVector4D>&);
    void showLegend(void);
    float cX(unsigned);
    float cY(unsigned);
    float cZ(unsigned);

protected:
    virtual void displayObject(void);
    virtual void drawPolygon(QVector<QVector4D>&);
    virtual void drawTriangle3D(QVector<QVector4D>&);
    virtual double getValue(unsigned i)
    {
        return (*results)[i];
    }
    void initColorTable(void);
    void setColor(int i)
    {
        TGLMesh::setColor(float(colorTable[i].redF()), float(colorTable[i].greenF()), float(colorTable[i].blueF()), float(colorTable[i].alphaF()));
    }
    int getColorIndex(float);

public slots:
    void mouseDoubleClickEvent(QMouseEvent*);

public:
    TGLFunction(TMesh*, vector<double>*, vector<double>*, vector<double>*, vector<double>*, QString, QWidget* = nullptr);
    ~TGLFunction(void) {}
    QString getExpression(void)
    {
        return expression;
    }
};

#endif // GLFUN_H
