#include <QFileDialog>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QtGui>
#include <QWheelEvent>
#include <cmath>
#include "glmesh.h"
#include "mesh/mesh.h"

/*******************************************************************/
TGLMesh::TGLMesh(TMesh* m, QWidget* parent) : QOpenGLWidget(parent)
{
    mesh = m;

    params.init();

    xList1 =
    xList2 = 0;
    radius = 0;

    diffuse = 0.8f;
    ambient = 0.8f;
    specular = 0.6f;

    isIdle =
    isLeftBtn =
    isRotate = true;
    isMouseDown =
    isScale =
    isTranslate = false;

    if (mesh->getDimension() < 3)
        params.isLight = false;

    minX = { float(mesh->getMinX(0)), float(mesh->getMinX(1)), float(mesh->getMinX(2)) };
    maxX = { float(mesh->getMaxX(0)), float(mesh->getMaxX(1)), float(mesh->getMaxX(2)) };
    x0   = { (maxX[0] + minX[0]) * 0.5f, (maxX[1] + minX[1]) * 0.5f, (maxX[2] + minX[2]) * 0.5f };
    radius = float(sqrt(pow(maxX[0] - minX[0], 2) + pow(maxX[1] - minX[1], 2) + pow(maxX[2] - minX[2], 2)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    action1 = new QAction(tr("&Save image..."), this);
    menu.addAction(action1);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(showContextMenu(QPoint)));

    setFocusPolicy(Qt::ClickFocus);
}
/*******************************************************************/
TGLMesh::~TGLMesh(void)
{
    delete action1;
    if (xList1)
        glDeleteLists(xList1, 1);
    if (xList2)
        glDeleteLists(xList2, 1);
}
/*******************************************************************/
void TGLMesh::restore(void)
{
    params.angleX =
    params.angleY =
    params.angleZ = 0;

    diffuse = 0.8f;
    ambient = 0.8f;
    specular = 0.6f;

    params.translateX =
    params.translateY =
    params.translateZ = 0.0;
    params.scale = 1.0;

    update();
}
/*******************************************************************/
void TGLMesh::repaint(void)
{
//    qDebug() << params.bkgColor.redF() << ' ' << params.bkgColor.greenF() << ' ' << params.bkgColor.blueF() << ' ' << params.bkgColor.alphaF();
//    glClearColor(params.bkgColor.redF(), params.bkgColor.greenF(), params.bkgColor.blueF(), params.bkgColor.alphaF());
    if (xList1)
        glDeleteLists(xList1, 1);
    if (xList2)
        glDeleteLists(xList2, 1);
    xList1 = xList2 = 0;
    if (params.isLight)
        setupLight();
    update();
}
/*******************************************************************/
QSize TGLMesh::minimumSizeHint() const
{
    return QSize(50, 50);
}
/*******************************************************************/
QSize TGLMesh::sizeHint() const
{
    return QSize(400, 400);
}
/*******************************************************************/
void TGLMesh::setRotate(void)
{
    isRotate = true;
    isScale = isTranslate = false;
}
/*******************************************************************/
void TGLMesh::setScale(void)
{
    isScale = true;
    isRotate = isTranslate = false;
}
/*******************************************************************/
void TGLMesh::setTranslate(void)
{
    isTranslate = true;
    isScale = isRotate = false;
}
/*******************************************************************/
void TGLMesh::initializeGL()
{
//    initializeOpenGLFunctions();
//    QOpenGLContext::currentContext()->extraFunctions()->glClearColor(params.bkgColor.red(), params.bkgColor.green(), params.bkgColor.blue(), params.bkgColor.alpha());
//    qDebug() << params.bkgColor.redF() << ' ' << params.bkgColor.greenF() << ' ' << params.bkgColor.blueF() << ' ' << params.bkgColor.alphaF();
//    glClearColor(params.bkgColor.redF(), params.bkgColor.greenF(), params.bkgColor.blueF(), params.bkgColor.alphaF());

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
/*******************************************************************/
void TGLMesh::setupLight(void)
{
    GLfloat vals[3],
            lightdiff = 0.7f,
            lightspec = 1.0f;


    vals[0] = vals[1] = vals[2] = 0.3f;
    glLightfv(GL_LIGHT0, GL_AMBIENT, vals);

    vals[0] = vals[1] = vals[2] = lightdiff;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, vals);
    vals[0] = vals[1] = vals[2] = lightspec;
    glLightfv(GL_LIGHT0, GL_SPECULAR, vals);

//    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, (params.isDoubleSided) ? GL_TRUE : GL_FALSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}
/*******************************************************************/
void TGLMesh::resizeGL(int width, int height)
{
    setupCamera(width, height);
    if (params.isLight)
        setupLight();
    else
        glEnable(GL_COLOR_MATERIAL);
}
/*******************************************************************/
void TGLMesh::mousePressEvent(QMouseEvent *event)
{
    isMouseDown = true;
// Раскоментировать, если будет тормозить
    isIdle = false;
    lastPos = event->pos();
}
/*******************************************************************/
void TGLMesh::mouseReleaseEvent(QMouseEvent*)
{
    isMouseDown = false;
    isIdle = true;
//    repaint();
    update();
}
/*******************************************************************/
void TGLMesh::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->position().x() - lastPos.x(),
        dy = event->position().y() - lastPos.y();

    if ((event->buttons() & Qt::LeftButton) and (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == false))
    {
        if (isRotate)
            rotate(dy, dx, 0);
        else if (isScale)
            scale(dx, dy);
        else if (isTranslate)
            translate(dx, dy);

    }
    else if ((event->buttons() & Qt::LeftButton) and (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true))
    {
        if (isRotate)
            rotate(dy, 0, dx);
    }
    else if (event->buttons() & Qt::RightButton)
    {
        translate(dx, dy);
    }
    lastPos = event->pos();
}
/*******************************************************************/
void TGLMesh::rotate(int dx, int dy, int dz)
{
    params.angleX += dx/2;
    params.angleY += dy/2;
    params.angleZ += dz/2;
    update();
}
/*******************************************************************/
void TGLMesh::scale(int dx, int dy)
{
    if (dx > 0 or dy > 0)
        params.scale *= 1.05f;
    else
        params.scale /= 1.05f;
    update();
}
/*******************************************************************/
void TGLMesh::translate(int dx, int dy)
{
    params.translateX += dx*radius*0.001f;
    params.translateY -= dy*radius*0.001f;
    update();
}
/*******************************************************************/
void TGLMesh::setupCamera(int width, int height)
{
    double fnear = 0.01*double(radius),
           ffar = 10*GLdouble(radius),
           angle = 60,
           h = tan(angle*M_PI/360.0)*fnear,
           a = (height == 0) ? 1.0 : double(width)/double(height),
           w = a*h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    gluPerspective(60.0, a, 0.01*GLdouble(radius), 10*GLdouble(radius));
//    gluLookAt(0, 0, GLdouble(radius), 0, 0, 0, 0, 1, 0);
    glFrustum(-w, w, -h, h, fnear, ffar);
    glTranslatef(0, 0, -radius);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_MODELVIEW);
}
/*******************************************************************/
void TGLMesh::makeMaterial(float r,float g,float b,float a)
{
  GLfloat val[] = { r, g, b, a },
          shininess = 50;

  val[0] = r*diffuse;
  val[1] = g*diffuse;
  val[2] = b*diffuse;
  val[3] = a;
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, val);

  val[0] = r*ambient;
  val[1] = g*ambient;
  val[2] = b*ambient;
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, val);

  val[0] = specular;
  val[1] = specular;
  val[2] = specular;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, val);

  val[0] = 0;
  val[1] = 0;
  val[2] = 0;
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, val);

  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}
/*******************************************************************/
void TGLMesh::drawCoordinateCross(void)
{
    GLint viewport[4];


    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glGetIntegerv(GL_VIEWPORT, viewport);

    glTranslatef(-1, -1, 0.0);
    glScalef (40.0f/viewport[2], 40.0f/viewport[3], 1);
    glTranslatef(1.25, 1.25, 0.0);

    glRotatef(params.angleX, 1.0, 0.0, 0.0);
    glRotatef(params.angleY, 0.0, 1.0, 0.0);
    glRotatef(params.angleZ, 0.0, 0.0, 1.0);

    if (params.isLight)
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
    }

    glLineWidth (1.0f);

    glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.9f, 0.0f, 0.0f);
        glColor3f(0,1,0);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.9f, 0.0f);
        glColor3f(0,0,1);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.9f);
    glEnd ();

    glColor3f(1, 0, 0);
    glPushMatrix();
    glTranslatef(0.5, 0, 0);
    glRotatef(90, 0, 1, 0);
    glPopMatrix();

    glColor3f(0, 1, 0);
    glPushMatrix();
    glTranslatef(0, 0.5, 0);
    glRotatef(-90, 1, 0, 0);
    glPopMatrix();

    glColor3f(0, 0, 1);
    glPushMatrix();
    glTranslatef(0, 0, 0.5);
    glPopMatrix();

    renderText(1.1, 0.0, 0.0, "X", Qt::red);
    renderText(0.1, 1.0, 0.0, "Y", Qt::green);
    renderText(0.1, 0.0, 1.0, "Z", Qt::blue);

    if (params.isLight)
    {
        glEnable(GL_LIGHTING);
        glDisable (GL_COLOR_MATERIAL);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
/*******************************************************************/
void TGLMesh::setColor(float r, float g, float b, float a)
{
    if (params.isLight)
        makeMaterial(r, g, b, a);
    else
        glColor4f(r, g, b, a);
}
/*******************************************************************/
void TGLMesh::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F5)
        repaint();
}
/*******************************************************************/
// Вращение колёсика мыши
void TGLMesh::wheelEvent(QWheelEvent* pe)
{
    isIdle = false;
    scale(pe->angleDelta().x(), pe->angleDelta().y());
    isIdle = true;
    update();
}
/*******************************************************************/
void TGLMesh::loadPaint(void)
{
    isIdle = true;
    paintGL();
}
/*******************************************************************/
void TGLMesh::paintGL(void)
{
    glClearColor(params.bkgColor.redF(), params.bkgColor.greenF(), params.bkgColor.blueF(), params.bkgColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (not mesh)
        return;

    glLoadIdentity();
    if (params.isMesh)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0,1.0);
    }
    else
        glDisable(GL_POLYGON_OFFSET_FILL);
    if (params.isLight)
    {
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glScalef(params.scale, params.scale, params.scale);
    glTranslatef(params.translateX*radius, params.translateY*radius, params.translateZ*radius);
    glRotatef(params.angleX, 1.0, 0.0, 0.0);
    glRotatef(params.angleY, 0.0, 1.0, 0.0);
    glRotatef(params.angleZ, 0.0, 0.0, 1.0);

    if (isIdle or mesh->getDimension() == 1)
        displayObject();
    else
        displaySkeleton();
    if (params.isCoord)
        drawCoordinateCross();
}
/*******************************************************************/
void TGLMesh::displaySkeleton(void)
{
    if (not xList2)
    {
        xList2 = glGenLists(1);
        glNewList(xList2, GL_COMPILE);

        setColor(1.0f - float(params.bkgColor.redF()), 1.0f - float(params.bkgColor.greenF()), 1.0f - float(params.bkgColor.blueF()), params.alpha);
        glPointSize(2);
        glBegin(GL_POINTS);
        for (unsigned i = 0; i < mesh->getNumVertex(); i++)
            glVertex3f(float(mesh->getX(i, 0)) - x0[0], float(mesh->getX(i, 1)) - x0[1], (mesh->getDimension() == 3) ? float(mesh->getX(i, 2)) - x0[2] : 0);
        glEnd();
        glEndList();
    }
    glCallList(xList2);
}
/*******************************************************************/
void TGLMesh::displayObject(void)
{
    if (not xList1)
    {
        xList1 = glGenLists(1);
        glNewList(xList1, GL_COMPILE);
        createObject();
        glEndList();
    }
    glCallList(xList1);
}
/*******************************************************************/
void TGLMesh::createObject(void)
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

    switch (mesh->getTypeFE())
    {
        case FEType::fe1d2:
            drawMesh1D();
            break;
        case FEType::fe2d3:
        case FEType::fe2d4:
        case FEType::fe2d6:
        case FEType::fe2d3p:
        case FEType::fe2d4p:
        case FEType::fe2d6p:
            drawMesh2D();
            break;
        case FEType::fe3d4:
        case FEType::fe3d8:
        case FEType::fe3d10:
        case FEType::fe3d3s:
        case FEType::fe3d4s:
        case FEType::fe3d6s:
            drawMesh3D();
            break;
        default:
            break;
    }
    if (params.isNormal)
        drawNormal();
    QApplication::restoreOverrideCursor();
}
/*******************************************************************/
void TGLMesh::drawMesh1D(void)
{
    if (params.isFace)
    {
        setColor(0.8f, 0.8f, 0.8f, params.alpha);
        glNormal3d(0, 0, 1);
        glBegin(GL_LINE_LOOP);
        for (unsigned i = 0; i < mesh->getNumFE(); i++)
            for (unsigned j = 0; j < mesh->getSizeFE(); j++)
                glVertex2f(float(mesh->getX(mesh->getFE(i, j), 0)) - x0[0], 0);
        glEnd();
    }
    if (params.isVertex)
    {
        setColor(0, 1, 0, params.alpha);
        glPointSize(3);
        glBegin(GL_POINTS);
            for (unsigned i = 0; i < mesh->getNumVertex(); i++)
                glVertex2f(float(mesh->getX(i, 0)) - x0[0], 0);
        glEnd();
    }
}
/*******************************************************************/
void TGLMesh::drawMesh2D(void)
{
    unsigned size = (mesh->getSizeFE() == 3 or mesh->getSizeFE() == 6) ? 3 : 4;

    glNormal3d(0, 0, 1);
    for (unsigned i = 0; i < mesh->getNumFE(); i++)
    {
        if (params.isFace)
        {
            setColor(0.8f, 0.8f, 0.8f, params.alpha);
            glBegin((mesh->getSizeFE() == 3 or mesh->getSizeFE() == 6) ? GL_TRIANGLES : GL_QUADS);
            for (unsigned j = 0; j < size; j++)
                glVertex2f(float(mesh->getX(mesh->getFE(i, j), 0)) - x0[0], float(mesh->getX(mesh->getFE(i, j), 1)) - x0[1]);
            glEnd();
        }

        if (params.isMesh)
        {
            setColor(0, 0, 0,params.alpha);
            glBegin(GL_LINE_LOOP);
            for (unsigned j = 0; j < size; j++)
                glVertex2f(float(mesh->getX(mesh->getFE(i, j), 0)) - x0[0], float(mesh->getX(mesh->getFE(i, j), 1)) - x0[1]);
            glEnd();
        }
    }

    if (params.isVertex)
    {
        setColor(0, 0, 0,params.alpha);
        glPointSize(2);
        glBegin(GL_POINTS);
        for (unsigned i = 0; i < mesh->getNumVertex(); i++)
            glVertex2f(float(mesh->getX(i, 0)) - x0[0], float(mesh->getX(i, 1)) - x0[1]);
        glEnd();
    }
}
/*******************************************************************/
void TGLMesh::drawMesh3D(void)
{
    vector <double> v(3);
    unsigned size = (mesh->getSizeBE() == 4) ? 4 : 3;

    for (unsigned i = 0; i < mesh->getNumBE(); i++)
    {
        if (params.isLight)
        {
            mesh->normal(i, v);
            glNormal3f(float(v[0]), float(v[1]), float(v[2]));
        }
        if (params.isFace)
        {
            setColor(0.8f, 0.8f, 0.8f, params.alpha);
//            setColor(0, 1, 0, params.alpha);
            glBegin((mesh->getSizeBE() == 4) ? GL_QUADS : GL_TRIANGLES);
            for (unsigned j = 0; j < mesh->getSizeBE(); j++)
                glVertex3f(float(mesh->getX(mesh->getBE(i, j), 0)) - x0[0], float(mesh->getX(mesh->getBE(i, j), 1)) - x0[1], float(mesh->getX(mesh->getBE(i, j), 2)) - x0[2]);
            glEnd();
        }

        if (params.isMesh)
        {
            setColor(0,0,0,params.alpha);
            glBegin(GL_LINE_LOOP);
            for (unsigned j = 0; j < size; j++)
                glVertex3f(float(mesh->getX(mesh->getBE(i, j), 0)) - x0[0], float(mesh->getX(mesh->getBE(i, j), 1)) - x0[1], float(mesh->getX(mesh->getBE(i, j), 2)) - x0[2]);
            glEnd();
        }

    }
    if (params.isVertex)
    {
        setColor(0,0,0,params.alpha);
        glPointSize(2);
        glBegin(GL_POINTS);
        for (unsigned i = 0; i < mesh->getNumVertex(); i++)
            glVertex3f(float(mesh->getX(i, 0)) - x0[0], float(mesh->getX(i, 1)) - x0[1], float(mesh->getX(i, 2)) - x0[2]);
        glEnd();
    }
}
/*******************************************************************/
void TGLMesh::showContextMenu(const QPoint &pos)
{
    QPoint globalPos;
    QAction* selectedItem;
    QString fileName;

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if ((selectedItem = menu.exec(globalPos)))
    {
        if (selectedItem == action1)
        {
            fileName = QFileDialog::getSaveFileName(this, tr("Saving the image"), fileName, tr("Image files (*.png)"));

            if (not fileName.isEmpty())
                saveImage(fileName);
        }
    }
}
/*******************************************************************/
void TGLMesh::saveImage(const QString& fileName)
{
    float colors[3];
    QColor color;
    QImage image(size().width(),size().height(),QImage::Format_RGB32);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int x = 0; x < size().width(); x++)
        for (int y = 0; y < size().height(); y++)
        {
            glReadPixels(x, size().height()-y, 1, 1, GL_RGB, GL_FLOAT, colors);
            color.setRgb(int(255.0f*colors[0]), int(255.0f*colors[1]), int(255.0f*colors[2]));
            image.setPixel(x,y,color.rgb());
        }
    image.save(fileName);
    QApplication::restoreOverrideCursor();
}
/*******************************************************************/
void TGLMesh::drawNormal(void)
{
    float coeff = 0.01f * radius,
          e[] = { 0, 0, 0 };
    vector<double> v(3),
                   c;

    for (unsigned i = 0; i < mesh->getNumBE(); i++)
    {
        // Строим нормаль к ГЭ
        mesh->normal(i, v);
        // Вычисляем координаты центра ГЭ
        mesh->getCenterBE(i, c);

        // Изображаем нормаль
        for (unsigned k = 0; k < c.size(); k++)
            e[k] = float(c[k]) + float(v[k]) * coeff;
        setColor(1, 0, 0, params.alpha);
        glBegin(GL_LINES);
            glVertex3f(float(c[0]) - x0[0], float(c[1]) - x0[1], float(c[2]) - x0[2]);
            glVertex3f(e[0] - x0[0], e[1] - x0[1], e[2] - x0[2]);
        glEnd();
    }
}
/*******************************************************************/
void TGLMesh::renderText(double x, double y, double z, QString text, QColor color, const QFont &font)
{
    QPainter painter(this);
    double textPosX = 0,
           textPosY = 0,
           textPosZ = 0,
           model[16],
           proj[16];
    int view[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);
    project(x, y, z, model, proj, view, &textPosX, &textPosY, &textPosZ);

    textPosY = height() - textPosY; // y is inverted
    painter.setPen(color);
    painter.setFont(font);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(textPosX, textPosY, text); // z = pointT4.z + distOverOp / 4
//    painter.end();
}
/*******************************************************************/
inline int TGLMesh::project(double objx, double objy, double objz, const double model[16], const double proj[16], const int viewport[4], double *winx, double *winy, double *winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;
    transformPoint(out, model, in);
    transformPoint(in, proj, out);

    if (in[3] == 0.0)
        return 0;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

    *winz = (1 + in[2]) / 2;
    return 1;
}
/*******************************************************************/
inline void TGLMesh::transformPoint(double out[4], const double m[16], const double in[4])
{
    out[0] = m[0] * in[0] + m[4] * in[1] + m[8] * in[2] + m[12] * in[3];
    out[1] = m[1] * in[0] + m[5] * in[1] + m[9] * in[2] + m[13] * in[3];
    out[2] = m[2] * in[0] + m[6] * in[1] + m[10] * in[2] + m[14] * in[3];
    out[3] = m[3] * in[0] + m[7] * in[1] + m[11] * in[2] + m[15] * in[3];
}
/*******************************************************************/
