#include <QVector4D>
#include "paramview.h"
#include "mesh/mesh.h"
#include "object/plist.h"

TParameterView::TParameterView(TMesh *mesh, QVector<QVector4D> *vertex, ParamType paramType, QWidget *parent) :
    TFunctionView(mesh, nullptr, {nullptr, nullptr, nullptr}, QString(), parent)
{
    this->vertex = vertex;
    this->paramType = paramType;
    vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
}

TParameterView::~TParameterView(void)
{
    makeCurrent();
    vbo.destroy();
    vao.destroy();
    doneCurrent();
}


void TParameterView::paintGL()
{
    TFunctionView::paintGL();

    glEnable(GL_DEPTH_TEST);

    shaderObject.bind();
    vao.bind();
    if (paramType == ParamType::PressureLoad || paramType == ParamType::SurfaceLoad || paramType == ParamType::VolumeLoad || paramType == ParamType::ConcentratedLoad)
        glDrawArrays(GL_LINES, 0, numPoints);
    else if (paramType == ParamType::BoundaryCondition)
    {
        glPointSize(2.0f);
        glDrawArrays(GL_POINTS, 0, numPoints);
        glPointSize(1.0f);
    }
    vao.release();
    shaderObject.release();

}

void TParameterView::initObject()
{
    if (paramType == ParamType::PressureLoad || paramType == ParamType::SurfaceLoad || paramType == ParamType::VolumeLoad || paramType == ParamType::ConcentratedLoad)
        createLoads();
    else if (paramType == ParamType::BoundaryCondition)
        createBoundaryConditions();
    else
        TFunctionView::initObject();
}

void TParameterView::createLoads(void)
{
    double maxValue = 0, w = 0.025f*radius;
    float color[]{0.0f, 0.0f, 1.0f};
    QVector<float> vertices;
    QVector4D x1, x2;


    // Определение максимального значения нагрузки
    for (auto &it: *vertex)
        if (it.length() > maxValue)
            maxValue = it.length();

    numPoints = 0;
    vertices.reserve(vertex->size()*12);
    for (int i = 0; i < vertex->size(); i++)
    {
        if ((*vertex)[i].length() == 0)
            continue;
        x1 = QVector4D(0, 0, 0, 0);
        x2 = QVector4D(0, 0, 0, 0);
        /*if (mesh->getDimension() == 1)
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x2.setX(mesh->getX(i, 0) + (*vertex)[i].x() * w / maxValue - x0[0]);
        }
        else*/ if (mesh->getDimension() == 2)
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x1.setY(mesh->getX(i, 1) - x0[1]);
            if (mesh->isPlate())
            {
                x2.setX(mesh->getX(i, 0) + (*vertex)[i].x()*w/maxValue - x0[0]);
                x2.setY(mesh->getX(i, 1) + (*vertex)[i].y()*w/maxValue - x0[1]);
                x2.setZ((*vertex)[i].z()*w/maxValue - x0[2]);
            }
            else
            {
                x2.setX(mesh->getX(i, 0) + (*vertex)[i].x()*w/maxValue - x0[0]);
                x2.setY(mesh->getX(i, 1) + (*vertex)[i].y()*w/maxValue - x0[1]);
            }
        }
        else
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x1.setY(mesh->getX(i, 1) - x0[1]);
            x1.setZ(mesh->getX(i, 2) - x0[2]);
            x2.setX(mesh->getX(i, 0) + (*vertex)[i].x()*w/maxValue - x0[0]);
            x2.setY(mesh->getX(i, 1) + (*vertex)[i].y()*w/maxValue - x0[1]);
            x2.setZ(mesh->getX(i, 2) + (*vertex)[i].z()*w/maxValue - x0[2]);
        }
        vertices.append({x1.x(), x1.y(), x1.z(), color[0], color[1], color[2]});
        vertices.append({x2.x(), x2.y(), x2.z(), color[0], color[1], color[2]});
        numPoints += 2;
    }



    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size()*sizeof(GLfloat));

    shaderObject.enableAttributeArray(0);
    shaderObject.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6*sizeof(GLfloat));
    shaderObject.enableAttributeArray(1);
    shaderObject.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));

    vbo.release();
    vao.release();
    TMeshView::initObject();
}

void TParameterView::createBoundaryConditions(void)
{
    float color[]{1.0f, 0.0f, 0.0f};
    QVector<float> vertices;

    vertices.reserve(vertex->size()*12);
    for (int i = 0; i < vertex->size(); i++)
        if ((*vertex)[i].w())
        {
            /*if (mesh->is1D())
                vertices.append({float(mesh->getX(i, 0) - x0[0]), 0, 0, color[0], color[1], color[2]});
            else */if (mesh->is2D() or mesh->isPlate())
                vertices.append({float(mesh->getX(i, 0) - x0[0]), float(mesh->getX(i, 1) - x0[1]), 0, color[0], color[1], color[2]});
            else
                vertices.append({float(mesh->getX(i, 0) - x0[0]), float(mesh->getX(i, 1) - x0[1]), float(mesh->getX(i, 2) - x0[2]), color[0], color[1], color[2]});
            numPoints++;
            //(*vertex)[i].setW(0);
        }

    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size()*sizeof(GLfloat));

    shaderObject.enableAttributeArray(0);
    shaderObject.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6*sizeof(GLfloat));

    vbo.release();
    vao.release();

    TMeshView::initObject();
}


