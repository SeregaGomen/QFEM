#include <QApplication>
#include <QFileDialog>
#include <QAbstractScrollArea>
#include <QOpenGLShader>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include "mesh/mesh.h"
#include "meshview.h"
#include "imageparams.h"

TMeshView::TMeshView(TMesh* mesh, QWidget *parent) : QOpenGLWidget(parent), mesh(mesh), params(new TImageParams)
{
    double minX{mesh->getMinX(0)}, minY{mesh->getMinX(1)}, minZ{mesh->getMinX(2)},
           maxX{mesh->getMaxX(0)}, maxY{mesh->getMaxX(1)}, maxZ{mesh->getMaxX(2)};
    x0 = (minX + maxX)*0.5f;
    y0 = (minY + maxY)*0.5f;
    z0 = (minZ + maxZ)*0.5f;
    radius = sqrt(pow(maxX - minX, 2) + pow(maxY - minY, 2) + pow(maxZ - minZ, 2));


    setAutoFillBackground(false);

    setContextMenuPolicy(Qt::CustomContextMenu);
    actionSaveImage = new QAction(tr("&Save image..."), this);
    menu.addAction(actionSaveImage);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu(QPoint)));


    for (auto i = 0; i < 3; i++)
        vbo[i] = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&TMeshView::slotRotate));
    timer->start(16);  // ~60 FPS
    params->init();

}

TMeshView::~TMeshView()
{
    makeCurrent();
    for (auto i = 0; i < 3; i++)
    {
        vbo[i].destroy();
        vao[i].destroy();
    }
    doneCurrent();
    delete timer;
    delete params;
}

void TMeshView::redraw()
{
    initObject();
    initMesh();
    update();
}

void TMeshView::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    initShaders();
    initObject();
    initMesh();
    initAxiss();

    view.lookAt(QVector3D(radius, radius, radius), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, radius));
    model.setToIdentity();
}

void TMeshView::slotRotate()
{
    if (params->isAutoRotate)
    {
        params->angle[0] = params->angle[0] < 360.0 ? params->angle[0] + 1.0 : params->angle[0] - 360.0;
        params->angle[1] = params->angle[1] < 360.0 ? params->angle[1] + 0.7 : params->angle[1] - 360.0;
        params->angle[2] = params->angle[2] < 360.0 ? params->angle[2] + 0.3 : params->angle[2] - 360.0;
        update();
    }
}

void TMeshView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    projection.setToIdentity();
    //projection.perspective(45.0f, float(w)/h, 0.1f, 100.0f);
    projection.perspective(45.0f, float(w)/h, 0.01f*radius, 10.0f*radius);
}


void TMeshView::paintGL()
{
    //QMatrix4x4 model;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(params->bkgColor.redF(), params->bkgColor.greenF(), params->bkgColor.blueF(), params->bkgColor.alphaF());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);  // Слегка отодвигаем назадglEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    model.setToIdentity();
    model.rotate(params->angle[0], 1, 0, 0);
    model.rotate(params->angle[1], 0, 1, 0);
    model.rotate(params->angle[2], 0, 0, 1);

    showObject();
    showAxiss();
    showLegend();
}

void TMeshView::showObject()
{
    shaderObject.bind();
    shaderObject.setUniformValue("model", model);
    shaderObject.setUniformValue("view", view);
    shaderObject.setUniformValue("projection", projection);
    shaderObject.setUniformValue("lightPos", QVector3D(0.5f, 0.7f, 1.0f));
    shaderObject.setUniformValue("scale", QVector4D(params->scale, params->scale, params->scale, 1.0f));
    shaderObject.setUniformValue("alpha", params->alpha);
    shaderObject.setUniformValue("isLight", params->isLight && !mesh->is1D() ? true : false);
    shaderObject.setUniformValue("translation", QVector4D(params->translate[0]*radius/10.0f, params->translate[1]*radius/10.0f, 0.0f, 0.0f));
    if (params->isSurface)
    {
        vao[0].bind();
        glDrawArrays(mesh->is1D() ? GL_LINES : GL_TRIANGLES, 0, numObjectPoints);
        vao[0].release();
    }
    if (params->isMesh)
    {
        vao[1].bind();
        glDrawArrays(GL_LINES, 0, numMeshPoints);
        vao[1].release();
    }
    shaderObject.release();
}

void TMeshView::showAxiss()
{
    auto worldToScreen = [this](const QVector3D &worldPos, const QMatrix4x4 &model, const QMatrix4x4 &view, const QMatrix4x4 &projection, const QVector4D &translation) {
        QMatrix4x4 mvp = projection*view*model;
        QVector4D clipSpacePos = mvp*QVector4D(worldPos, 1.0) - translation;

        if (clipSpacePos.w() == 0.0f)
            return QVector3D(); // ошибка деления

        // NDC [-1, 1]
        QVector3D ndc = clipSpacePos.toVector3DAffine();  // делит на w

        // Viewport transform -> [0, width], [0, height]
        float x = (ndc.x()*0.5f + 0.5f)*width();
        float y = (1.0f - (ndc.y()*0.5f + 0.5f))*height(); // инверсия Y
        float z = ndc.z();

        return QVector3D(x, y, z);
    };

    if (params->isAxis)
    {
        shaderAxiss.bind();
        shaderAxiss.setUniformValue("model", model);
        shaderAxiss.setUniformValue("view", view);
        shaderAxiss.setUniformValue("projection", projection);
        //shaderAxiss.setUniformValue("translation", QVector4D(1.5f*mesh->radius, 1.0f*mesh->radius, 0.0f, 0.0f));
        shaderAxiss.setUniformValue("translation", QVector4D(1.6f*radius, 1.4f*radius, 0.0f, 0.0f));
        shaderAxiss.setUniformValue("scale", QVector4D(1, 1, 1, 0.0f));
        shaderAxiss.setUniformValue("alpha", 1.0f);
        vao[2].bind();
        glDrawArrays(GL_LINES, 0, 6);
        vao[2].release();
        shaderAxiss.release();

        showText(worldToScreen(QVector3D(0.06*radius, 0.0, 0.0), model, view, projection, QVector4D(1.6f*radius, 1.4f*radius, 0.0f, 0.0f)), Qt::red, "X");
        showText(worldToScreen(QVector3D(0.0, 0.06*radius, 0.0), model, view, projection, QVector4D(1.6f*radius, 1.4f*radius, 0.0f, 0.0f)), Qt::green, "Y");
        showText(worldToScreen(QVector3D(0.0, 0.0, 0.06*radius), model, view, projection, QVector4D(1.6f*radius, 1.4f*radius, 0.0f, 0.0f)), Qt::blue, "Z");
    }
}


void TMeshView::showText(const QVector3D &pos, const QColor &color, const QString &text)
{
    QPainter painter(this);

    painter.setFont(QFont("Aral", 10));
    painter.setPen(color);
    painter.drawText(pos.x(), pos.y(), text);
    painter.setPen(Qt::green);
    painter.end();
}

void TMeshView::showLegend()
{
    if (params->isShowLegend)
    {
        QRect rc = rect();
        QPainter painter(this);
        QFont font("Aral", 10);
        QFontMetrics fm(font);
        int fontH = fm.height(), cy = rc.top() + 20;

        painter.setFont(font);
        painter.setPen(QColor(255 - float(params->bkgColor.red()), 255 - float(params->bkgColor.green()), 255 - float(params->bkgColor.blue())));
        painter.drawText(10, int(cy), tr("Mesh info"));
        painter.drawText(10, int(cy) + fontH, tr("FE type: %1").arg(mesh->feName()));
        painter.drawText(10, int(cy) + 2*fontH, tr("Vertex: %1").arg(mesh->getNumVertex()));
        painter.drawText(10, int(cy) + 3*fontH, tr("FE: %1").arg(mesh->getNumFE()));
        painter.drawText(10, int(cy) + 4*fontH, tr("BE: %1").arg(mesh->getNumBE()));

        painter.end();
    }
}

// QVector3D TMeshView::worldToScreen(const QVector3D &worldPos, const QMatrix4x4 &model, const QMatrix4x4 &view, const QMatrix4x4 &projection, const QVector4D &translation)
// {
//     QMatrix4x4 mvp = projection*view*model;
//     QVector4D clipSpacePos = mvp*QVector4D(worldPos, 1.0) - translation;

//     if (clipSpacePos.w() == 0.0f)
//         return QVector3D(); // ошибка деления

//     // NDC [-1, 1]
//     QVector3D ndc = clipSpacePos.toVector3DAffine();  // делит на w

//     // Viewport transform -> [0, width], [0, height]
//     float x = (ndc.x()*0.5f + 0.5f)*width();
//     float y = (1.0f - (ndc.y()*0.5f + 0.5f))*height(); // инверсия Y
//     float z = ndc.z();

//     return QVector3D(x, y, z);
// }

void TMeshView::initShaders()
{
    shaderObject.addShaderFromSourceCode(QOpenGLShader::Vertex,
        R"(
            #version 330 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec3 aColor;
            layout(location = 2) in vec3 aNormal;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform vec4 scale;
            uniform vec4 translation;

            out vec3 FragPos;
            out vec3 Normal;
            out vec3 Color;

            void main()
            {
                FragPos = vec3(model*vec4(aPos, 1.0)*scale);
                Normal = mat3(transpose(inverse(model)))*aNormal;
                Color = aColor;
                gl_Position = projection*view*vec4(FragPos, 1.0) + translation;
            }
        )");

    shaderObject.addShaderFromSourceCode(QOpenGLShader::Fragment,
        R"(
            #version 330 core
            in vec3 Normal;
            in vec3 Color;

            uniform vec3 lightPos;
            uniform float alpha;
            uniform bool isLight;

            out vec4 FragColor;

            void main()
            {
                FragColor = vec4(Color, alpha);
                FragColor.rgb *= isLight ? abs(dot(Normal, lightPos)) : 1.0;
            }
        )");
    shaderObject.link();

    shaderAxiss.addShaderFromSourceCode(QOpenGLShader::Vertex,
        R"(
            #version 330 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec3 aColor;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform vec4 translation;

            out vec3 FragPos;
            out vec3 Color;

            void main()
            {
                FragPos = vec3(model*vec4(aPos, 1.0));
                Color = aColor;
                gl_Position = projection*view*vec4(FragPos, 1.0) - translation;
            }
        )");
    shaderAxiss.addShaderFromSourceCode(QOpenGLShader::Fragment,
        R"(
            #version 330 core
            in vec3 Color;

            out vec4 FragColor;

            void main()
            {
                FragColor = vec4(Color, 1);
            }
        )");
    shaderAxiss.link();

}

double TMeshView::x(unsigned i)
{
    return mesh->getX(i, 0);
}

double TMeshView::y(unsigned i)
{
    return mesh->getX(i, 1);
}

double TMeshView::z(unsigned i)
{
    return mesh->getX(i, 2);
}

void TMeshView::initObject()
{
    int len = mesh->getTypeFE() == FEType::fe3d8 || mesh->getTypeFE() == FEType::fe3d4s || mesh->getTypeFE() == FEType::fe2d4 || mesh->getTypeFE() == FEType::fe2d4p ? 2 : 1;
    int index[][3]{{0, 1, 2}, {2, 3, 0}};
    matrix<unsigned> &elm = mesh->is2D() ? mesh->getFE() : mesh->getBE();
    QVector<float> vertices, /*color{0.7f, 0.2f, 0.3f}*/color{0.0f, 1.0f, 0.0f};
    vector<double> normal{0, 0, 1};

    if (mesh->is1D())
    {
        vertices.reserve(elm.size1()*18);
        for (auto i = 0u; i < elm.size1(); i++)
            for (auto j = 0; j < 2; j++)
                vertices.append({float(x(elm(i, j)) - x0), 0, 0, color[0], color[1], color[2], 1, 0, 0}); // coordinates
        numObjectPoints = 2*mesh->getNumFE();
    }
    else
    {
        vertices.reserve(elm.size1()*27*len);
        for (auto i = 0u; i < elm.size1(); i++)
        {
            if (mesh->is3D() || mesh->isShell())
                normal = mesh->normal(i);
            for (auto l = 0; l < len; l++)
            {
                for (auto j = 0; j < 3; j++)
                {
                    vertices.append({float(x(elm(i, index[l][j])) - x0), float(y(elm(i, index[l][j])) - y0), float(z(elm(i, index[l][j])) - z0)}); // coordinates
                    vertices.append(color); // colors
                    vertices.append({float(normal[0]), float(normal[1]), float(normal[2])}); // normals
                }
            }
        }
        numObjectPoints = (mesh->is2D() ? mesh->getNumFE() : mesh->getNumBE())*3*(mesh->getTypeFE() == FEType::fe3d8 || mesh->getTypeFE() == FEType::fe3d4s || mesh->getTypeFE() == FEType::fe2d4 || mesh->getTypeFE() == FEType::fe2d4p ? 2 : 1);
    }

    vao[0].create();
    vao[0].bind();

    vbo[0].create();
    vbo[0].bind();
    vbo[0].allocate(vertices.data(), vertices.size()*sizeof(GLfloat));

    shaderObject.enableAttributeArray(0);
    shaderObject.setAttributeBuffer(0, GL_FLOAT, 0, 3, 9*sizeof(GLfloat));

    shaderObject.enableAttributeArray(1);
    shaderObject.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3, 9*sizeof(GLfloat));

    shaderObject.enableAttributeArray(2);
    shaderObject.setAttributeBuffer(2, GL_FLOAT, 6*sizeof(GLfloat), 3, 9*sizeof(GLfloat));

    vbo[0].release();
    vao[0].release();

    //qDebug() << glGetError();
}

void TMeshView::initMesh()
{
    int len = mesh->getTypeFE() == FEType::fe3d8 || mesh->getTypeFE() == FEType::fe3d4s || mesh->getTypeFE() == FEType::fe2d4 || mesh->getTypeFE() == FEType::fe2d4p ? 4 : 3,
        index3[][2]{{0, 1}, {1, 2}, {2, 0}},
        index4[][2]{{0, 1}, {1, 2}, {2, 3}, {3, 0}},
        (*index)[2] = mesh->getTypeFE() == FEType::fe3d8 || mesh->getTypeFE() == FEType::fe3d4s || mesh->getTypeFE() == FEType::fe2d4 || mesh->getTypeFE() == FEType::fe2d4p ? index4 : index3;
    QVector<float> vertices, color{0.0, 0.0, 0.0};
    matrix<unsigned> &elm = mesh->is2D() ? mesh->getFE() : mesh->getBE();

    vertices.reserve(elm.size1()*12*len);
    for (auto i = 0u; i < elm.size1(); i++)
    {
        for (auto j = 0; j < len; j++)
        {
            vertices.append({float(x(elm(i, index[j][0])) - x0), float(y(elm(i, index[j][0])) - y0), float(z(elm(i, index[j][0])) - z0)});
            vertices.append(color);
            vertices.append({float(x(elm(i, index[j][1])) - x0), float(y(elm(i, index[j][1])) - y0), float(z(elm(i, index[j][1])) - z0)});
            vertices.append(color);
        }
    }

    numMeshPoints = (mesh->is2D() ? mesh->getNumFE() : mesh->getNumBE())*(mesh->getTypeFE() == FEType::fe3d8 || mesh->getTypeFE() == FEType::fe3d4s || mesh->getTypeFE() == FEType::fe2d4 || mesh->getTypeFE() == FEType::fe2d4p ? 8 : 6);

    if (!vao[1].isCreated())
        vao[1].create();
    vao[1].bind();

    if (!vbo[1].isCreated())
        vbo[1].create();
    vbo[1].bind();
    vbo[1].allocate(vertices.data(), vertices.size()*sizeof(GLfloat));

    shaderObject.enableAttributeArray(0);
    shaderObject.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6*sizeof(GLfloat));

    shaderObject.enableAttributeArray(1);
    shaderObject.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));


    vbo[1].release();
    vao[1].release();
}

void TMeshView::initAxiss()
{
    float color[][3]{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}, coord[][3]{{0.05, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.05}};
    QVector<float> vertices;

    vertices.reserve(mesh->getNumBE()*36);
    for (auto i= 0; i < 3; i++)
    {
        vertices.append({0, 0, 0});
        vertices.append({color[0][i], color[1][i], color[2][i]});
        vertices.append({coord[i][0]*radius, coord[i][1]*radius, coord[i][2]*radius});
        vertices.append({color[0][i], color[1][i], color[2][i]});
    }

    vao[2].create();
    vao[2].bind();

    vbo[2].create();
    vbo[2].bind();
    vbo[2].allocate(vertices.data(), 36*sizeof(GLfloat));

    shaderAxiss.enableAttributeArray(0);
    shaderAxiss.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6*sizeof(GLfloat));
    shaderAxiss.enableAttributeArray(1);
    shaderAxiss.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));

    vbo[2].release();
    vao[2].release();
}

void TMeshView::wheelEvent(QWheelEvent* pe)
{
    if (pe->angleDelta().x() > 0 or pe->angleDelta().y() > 0)
        params->scale = params->scale * 1.05f > 5.0f ? 5.0f : params->scale * 1.05f;
    else
        params->scale = params->scale / 1.05f < 0.5f ? 0.5f : params->scale / 1.05f;
    update();
}

void TMeshView::mousePressEvent(QMouseEvent *event)
{
    isMouseDown = true;
    lastPos = event->pos();
}

void TMeshView::mouseReleaseEvent(QMouseEvent*)
{
    isMouseDown = false;
    update();
}

void TMeshView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->position().x() - lastPos.x(),
        dy = event->position().y() - lastPos.y();

    if (params->isAutoRotate)
        return;
    if ((event->buttons() & Qt::LeftButton) and (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == false))
    {
        params->angle[0] = params->angle[0] < 360.0 ? params->angle[0] + dy : params->angle[0] - 360.0;
        params->angle[1] = params->angle[1] < 360.0 ? params->angle[1] + dx : params->angle[1] - 360.0;
        update();

    }
    else if ((event->buttons() & Qt::LeftButton) and (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true))
    {
        params->angle[0] = params->angle[0] < 360.0 ? params->angle[0] + dy : params->angle[0] - 360.0;
        params->angle[2] = params->angle[2] < 360.0 ? params->angle[2] + dx : params->angle[2] - 360.0;
        update();
    }
    lastPos = event->pos();
}

void TMeshView::slotShowContextMenu(const QPoint &pos)
{
    QPoint globalPos;
    QAction *selectedItem;
    QString fileName;

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if ((selectedItem = menu.exec(globalPos)))
    {
        if (selectedItem == actionSaveImage)
        {
            if (params->isAutoRotate)
                timer->stop();
            fileName = QFileDialog::getSaveFileName(this, tr("Saving the image"), fileName, tr("Image files (*.png)"));
            if (not fileName.isEmpty())
            {
                QFileInfo info(fileName);
                if (info.suffix().isEmpty())
                    fileName += ".png";
                saveImage(fileName);
            }
            if (params->isAutoRotate)
                timer->start();
        }
    }
}

void TMeshView::saveImage(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QImage image = this->grabFramebuffer();
    image.save(fileName);
    QApplication::restoreOverrideCursor();

}

void TMeshView::startTimer()
{
    timer->start();
}

void TMeshView::stopTimer()
{
    timer->stop();
}
