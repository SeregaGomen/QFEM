#ifndef MESHVIEW_H
#define MESHVIEW_H

#include <QObject>
#include <QMenu>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>

class TMesh;
class TImageParams;


class TMeshView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    TMeshView(TMesh*, QWidget *parent = nullptr);
    virtual ~TMeshView();
    TImageParams* getImageParams()
    {
        return params;
    }
    void loadPaint()
    {
        paintGL();
    }
    void redraw();
    void startTimer();
    void stopTimer();

protected slots:
    void slotRotate();
    void slotShowContextMenu(const QPoint&);

protected:
    bool isMouseDown{false};
    float radius{0};
    int numObjectPoints{0};
    int numMeshPoints{0};
    double x0{0};
    double y0{0};
    double z0{0};
    TMesh *mesh;
    TImageParams *params;
    QPoint lastPos;
    QOpenGLBuffer vbo[3];
    QOpenGLVertexArrayObject vao[3];
    QOpenGLShaderProgram shaderObject;
    QOpenGLShaderProgram shaderAxiss;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int, int) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

    virtual void initObject();
    virtual void initShaders();
    virtual void initMesh();
    virtual void initAxiss();

    virtual void showLegend();
    virtual void showObject();
    virtual void showAxiss();

    virtual void showText(const QVector3D&, const QColor&, const QString&);

    virtual double x(unsigned);
    virtual double y(unsigned);
    virtual double z(unsigned);

private:
    QMenu menu;
    QAction* actionSaveImage;
    QTimer *timer;
    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;


    void saveImage(const QString&);
        QVector3D worldToScreen(const QVector3D&, const QMatrix4x4&, const QMatrix4x4&, const QMatrix4x4&, const QVector4D&);
};

#endif // MESHVIEW_H
