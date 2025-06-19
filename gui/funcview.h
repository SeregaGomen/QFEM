#ifndef FUNCTIONVIEW_H
#define FUNCTIONVIEW_H

#include "meshview.h"

using namespace std;

class TMesh;
class TResults;

class TFunctionView : public TMeshView
{
    Q_OBJECT
private:
    double maxTransformRatio{0};
    vector<double> *results;
    vector<vector<double>*> delta;
    QString expression;
    QVector<QColor> colorTable;

    int getColorIndex(float);
    void getColor(int, QVector<float>&);

    void showLegend() override;

    void createPolygon(const QVector<QVector4D>&, QVector<GLfloat>&);
    void createSegment(const QVector<QVector4D>&, QVector<GLfloat>&);
    void createTriangle3D(const QVector<QVector4D>&, QVector<GLfloat>&);
    void addTriangle(const QVector<QVector4D>&, QVector<GLfloat>&, const array<float, 3>&, int);

    double x(unsigned i, unsigned j) override;

    virtual double getValue(unsigned i)
    {
        return double((*results)[i]);
    }

protected:
    double min_u[2]{0, 0};
    double max_u[2]{0, 0};

    void initObject() override;
    void initColorTable();
    virtual void getMinMax();

public:
    TFunctionView(TMesh *mesh, vector<double> *results, vector<vector<double>*> delta, QString expression, QWidget *parent) :
        TMeshView(mesh, parent), results(results), delta(delta), expression(expression)
    {
        //initColorTable();
    }
    virtual ~TFunctionView() = default;
    QString getExpression(void)
    {
        return expression;
    }
    void refresh()
    {
        initObject();
        update();
    }
};

#endif // FUNCTIONVIEW_H
