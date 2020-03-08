#ifndef GLFUN_H
#define GLFUN_H

#include <QVector4D>
#include "glmesh.h"
#include "util/matrix.h"

class TMesh;
class TResultList;

class TGLFunction : public TGLMesh
{
    Q_OBJECT
private:
    vector<double> *results;    // Результаты расчета
    vector<double> *dx;         // Смещения
    vector<double> *dy;         // ...
    vector<double> *dz;         // ...
    QString expression;         // Выражение, с помощью которого вычислена визуализируемая функция
    QVector<QVector4D> vertex;  // Данные для визуализации нагрузок, гр. условий, etc
    QVector<QColor> colorTable;
    float min_u;
    float max_u;

    void displayObject(void);
    void createFunction(void);
    void drawFun1D(void);
    void drawFun2D(void);
    void drawFun3D(void);
    void drawSegment(QVector<QVector4D>&);
    void drawPolygon(QVector<QVector4D>&);
    void drawTriangle3D(QVector<QVector4D>&);
    void initColorTable(void);
    void showLegend(void);
    void setColor(int i)
    {
        TGLMesh::setColor(float(colorTable[i].redF()), float(colorTable[i].greenF()), float(colorTable[i].blueF()), float(colorTable[i].alphaF()));
    }
    int getColorIndex(float);
    float cX(unsigned);
    float cY(unsigned);
    float cZ(unsigned);
    float cZ2D(unsigned);
public slots:
    void mouseDoubleClickEvent(QMouseEvent*);

public:
    TGLFunction(TMesh*, vector<double>*, vector<double>*, vector<double>*, vector<double>*, QString, QWidget* = nullptr);
    TGLFunction(TMesh*, QVector<QVector4D>&, QWidget* = nullptr);
    ~TGLFunction(void) {}
    QString getExpression(void)
    {
        return expression;
    }
    void redraw(QVector<QVector4D>& v)
    {
        vertex = v;
        initColorTable();
        repaint();
    }
};

#endif // GLFUN_H
