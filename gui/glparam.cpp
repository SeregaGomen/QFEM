#include <QVector4D>
#include "glparam.h"
#include "mesh/mesh.h"
#include "object/params.h"

/*******************************************************************/
TGLParameter::TGLParameter(TMesh *m, QVector<QVector4D>& v, ParamType t, QWidget *parent) : TGLFunction(m, nullptr, nullptr, nullptr, nullptr, "", parent)
{
    vertex = v;
    paramType = t;
}
/*******************************************************************/
void TGLParameter::createObject(void)
{
    if (paramType == ParamType::Pressure_load or paramType == ParamType::SurfaceLoad or paramType == ParamType::VolumeLoad or paramType == ParamType::ConcentratedLoad)
        createLoads();
    else if (paramType == ParamType::BoundaryCondition)
        createBoundaryConditions();
    else
        TGLFunction::createObject();
}
/*******************************************************************/
void TGLParameter::createLoads(void)
{
    double maxValue = 0,
           w = 0.01f * radius;
    QVector4D x1,
              x2;

    // Определение максимального значения нагрузки
    for (auto it: vertex)
        if (it.length() > maxValue)
            maxValue = it.length();

    TGLMesh::createObject();

    TGLMesh::setColor(0, 1, 0, params.alpha);
    glPointSize(2);
    for (int i = 0; i < vertex.size(); i++)
    {
        if (not vertex[i].length())
            continue;
        x1 = QVector4D(0, 0, 0, 0);
        x2 = QVector4D(0, 0, 0, 0);
        if (mesh->getDimension() == 1)
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x2.setX(mesh->getX(i, 0) + vertex[i].x() * w / maxValue - x0[0]);
        }
        else if (mesh->getDimension() == 2)
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x1.setY(mesh->getX(i, 1) - x0[1]);
            if (mesh->isPlate())
            {
                x2.setX(mesh->getX(i, 0) + vertex[i].x() * w / maxValue - x0[0]);
                x2.setY(mesh->getX(i, 1) + vertex[i].y() * w / maxValue - x0[1]);
                x2.setZ(vertex[i].z() * w / maxValue - x0[2]);
            }
            else
            {
                x2.setX(mesh->getX(i, 0) + vertex[i].x() * w / maxValue - x0[0]);
                x2.setY(mesh->getX(i, 1) + vertex[i].y() * w / maxValue - x0[1]);
            }
        }
        else
        {
            x1.setX(mesh->getX(i, 0) - x0[0]);
            x1.setY(mesh->getX(i, 1) - x0[1]);
            x1.setZ(mesh->getX(i, 2) - x0[2]);
            x2.setX(mesh->getX(i, 0) + vertex[i].x() * w / maxValue - x0[0]);
            x2.setY(mesh->getX(i, 1) + vertex[i].y() * w / maxValue - x0[1]);
            x2.setZ(mesh->getX(i, 2) + vertex[i].z() * w / maxValue - x0[2]);
        }

        glBegin(GL_POINTS);
            glVertex3f(x1.x(), x1.y(), x1.z());
        glEnd();
        glBegin(GL_LINES);
            glVertex3f(x1.x(), x1.y(), x1.z());
            glVertex3f(x2.x(), x2.y(), x2.z());
        glEnd();
    }
}
/*******************************************************************/
void TGLParameter::createBoundaryConditions(void)
{
    TGLMesh::createObject();
    TGLMesh::setColor(1, 0, 0, params.alpha);
    glPointSize(4);
    glBegin(GL_POINTS);
        for (int i = 0; i < vertex.size(); i++)
            if (vertex[i].w())
            {
                if (mesh->is1D())
                    glVertex2f(float(mesh->getX(i, 0)) - x0[0], 0);
                else if (mesh->is2D() or mesh->isPlate())
                    glVertex2f(float(mesh->getX(i, 0)) - x0[0], float(mesh->getX(i, 1)) - x0[1]);
                else
                    glVertex3f(float(mesh->getX(i, 0)) - x0[0], float(mesh->getX(i, 1)) - x0[1], float(mesh->getX(i, 2)) - x0[2]);
            }
    glEnd();
}
/*******************************************************************/
