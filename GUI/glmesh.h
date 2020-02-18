#ifndef GLMESH_H
#define GLMESH_H

#include <QGLWidget>
#include <QOpenGLFunctions_3_0>
// #include <QOpenGLFunctions>
#include "setuptaskdialog.h"
#include "imageparams.h"

class TSetupTaskDialog;
class TMesh;

class TGLMesh : public QGLWidget, protected QOpenGLFunctions_3_0
// class TGLMesh : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

private:
    QPoint lastPos;
    QMenu menu; // Контекстное меню
    QAction* action1;

    float diffuse;
    float ambient;
    float specular;
    QVector<QVector3D>* vertex;
    bool isRotate;
    bool isScale;
    bool isTranslate;
    bool isLeftBtn;
    bool isMouseDown;

    void scale(int,int);
    void translate(int,int);
    void rotate(int,int,int);
    void displayMesh(void);
    void drawMesh1D(void);
    void drawMesh2D(void);
    void drawMesh3D(void);
    void drawLoad(void);
    void drawLimit(void);
    void drawNormal(void);

protected:
    TMesh* mesh;
    TImageParams params;

    vector<float> minX{0, 0, 0};
    vector<float> maxX{0, 0, 0};
    vector<float> x0{0, 0, 0};
    unsigned xList1;
    unsigned xList2;
    float radius;
    bool isIdle;

    void initializeGL(void);
    virtual void paintGL(void);
    virtual void displayObject(void);
    virtual void displaySkeleton(void);
    void resizeGL(int,int);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void wheelEvent(QWheelEvent* pe);
    void setupCamera(int,int);
    void setupLight(void);
    void drawCoordinateCross(void);
    void makeMaterial(float,float,float,float);
    void saveImage(const QString&);

public:
    TGLMesh(TMesh*, QVector<QVector3D>* = nullptr, QWidget* = nullptr);
    ~TGLMesh(void);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void restore(void);
    void repaint(void);
    void setImageParams(TImageParams& r)
    {
        params = r;
    }
    void setColor(float r,float g,float b,float a = 1);
    void loadPaint(void);
    bool isSelectedVertex(void);
    bool getRotate(void)
    {
        return isRotate;
    }
    bool getScale(void)
    {
        return isScale;
    }
    bool getTranslate(void)
    {
        return isTranslate;
    }
    TImageParams& getImageParams(void)
    {
        return params;
    }

public slots:
    void setRotate(void);
    void setScale(void);
    void setTranslate(void);
    void showContextMenu(const QPoint&);

signals:
    void xRotationChanged(int);
    void yRotationChanged(int);
    void zRotationChanged(int);
};

#endif
