#include <QVector4D>
#include "glparam.h"
#include "mesh/mesh.h"

/*******************************************************************/
TGLParameter::TGLParameter(TMesh *m, QVector<QVector4D>& v, int t, QWidget *parent) : TGLFunction(m, nullptr, nullptr, nullptr, nullptr, "", parent)
{
    vertex = v;
    paramType = t;
}
/*******************************************************************/

