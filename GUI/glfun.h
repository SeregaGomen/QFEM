#ifndef GLFUN_H
#define GLFUN_H

#include "glmesh.h"
#include "util/matrix.h"

class TMesh;
class TResultList;

class TGLFunction : public TGLMesh
{
    Q_OBJECT
private:
    TResultList* results; // Результаты расчета
    unsigned funIndex; // Индекс функции в списке
    unsigned deltaIndex; // Индекс перемещений (U) в списке
    QVector<QColor> colorTable;
    QString expression; // Выражение, с помощью которого вычислена визуализируемая функция
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
    float cZ_2D(unsigned);
public slots:
    void mouseDoubleClickEvent(QMouseEvent*);

public:
    TGLFunction(TMesh*, TResultList*, unsigned, unsigned, QString, QWidget* parent = nullptr);
    ~TGLFunction(void) {}
    void clearObject(void)
    {
        mesh = nullptr;
        results = nullptr;
    }
    QString getExpression(void)
    {
        return expression;
    }
};

#endif // GLFUN_H
