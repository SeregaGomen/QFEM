#include <QVector>
#include <QVector4D>
#include <QFileDialog>
#include <QAbstractScrollArea>
#include <QInputDialog>
#include <QToolTip>
#include <QMouseEvent>
#include <QMessageBox>
#include <cmath>
#include "glfun.h"
#include "fem/fem.h"
#include "analyse/analyse.h"
#include "mesh/mesh.h"

/*******************************************************************/
TGLFunction::TGLFunction(TMesh *m, vector<double> *r, vector<double> *x, vector<double> *y, vector<double> *z, QString exp, QWidget *parent) : TGLMesh(m, parent)
{
    results = r;
    dx = x;
    dy = y;
    dz = z;
    expression = exp;
    isIdle = true;

    minX = { float(mesh->getMinX(0)), float(mesh->getMinX(1)), float(mesh->getMinX(2)) };
    maxX = { float(mesh->getMaxX(0)), float(mesh->getMaxX(1)), float(mesh->getMaxX(2)) };
    x0   = { (maxX[0] + minX[0]) * 0.5f, (maxX[1] + minX[1]) * 0.5f, (maxX[2] + minX[2]) * 0.5f };
    radius = float(sqrt(pow(maxX[0] - minX[0], 2) + pow(maxX[1] - minX[1], 2) + pow(maxX[2] - minX[2], 2)));

    if (mesh->getFreedom() < 3)
        params.isLight = false;
}
/*******************************************************************/
TGLFunction::TGLFunction(TMesh *m, QVector<QVector4D> &v, QWidget *parent) : TGLMesh(m, parent)
{
    results = dx = dy = dz = nullptr;
    vertex = v;
    isIdle = true;

    minX = { float(mesh->getMinX(0)), float(mesh->getMinX(1)), float(mesh->getMinX(2)) };
    maxX = { float(mesh->getMaxX(0)), float(mesh->getMaxX(1)), float(mesh->getMaxX(2)) };
    x0   = { (maxX[0] + minX[0]) * 0.5f, (maxX[1] + minX[1]) * 0.5f, (maxX[2] + minX[2]) * 0.5f };
    radius = float(sqrt(pow(maxX[0] - minX[0], 2) + pow(maxX[1] - minX[1], 2) + pow(maxX[2] - minX[2], 2)));

    if (mesh->getFreedom() < 3)
        params.isLight = false;
}
/*******************************************************************/
void TGLFunction::displayObject(void)
{
    if (mesh->getTypeFE() == NOTYPE)
        return;

    if (!xList1)
        createFunction();
    glCallList(xList1);
    if (params.isShowLegend)
        showLegend();
}
/*******************************************************************/
void TGLFunction::createFunction(void)
{
    initColorTable();

    xList1 = glGenLists(1);
    setupCamera(width(),height());
    glNewList (xList1, GL_COMPILE);

    switch (mesh->getTypeFE())
    {
        case FE1D2:
            drawFun1D();
            break;
        case FE2D3:
        case FE2D4:
        case FE2D6:
        case FE2D3P:
        case FE2D4P:
        case FE2D6P:
            drawFun2D();
            break;
        case FE3D4:
        case FE3D8:
        case FE3D10:
        case FE3D3S:
        case FE3D4S:
        case FE3D6S:
            drawFun3D();
            break;
        default:
            break;
    }
    glEndList();
}
/*******************************************************************/
void TGLFunction::drawFun1D(void)
{
    QVector<QVector4D> data(int(mesh->getSizeFE()));

    for (unsigned i = 0; i < mesh->getNumFE(); i++)
    {
        for (unsigned j = 0; j < mesh->getSizeFE(); j++)
        {
            data[int(j)].setX(cX(mesh->getFE(i, j)) - x0[0]);
            data[int(j)].setY(0);
            data[int(j)].setZ(0);
            data[int(j)].setW( (results) ? float((*results)[mesh->getFE(i, j)]) : vertex[mesh->getFE(i, j)].w());
        }
        drawSegment(data);
    }
}
/*******************************************************************/
void TGLFunction::drawFun2D(void)
{
    QVector<QVector4D> data(int(mesh->getSizeFE()));

    for (unsigned i = 0; i < mesh->getNumFE(); i++)
    {
//        if (i != 212 && i != 213)
//            continue;
        for (unsigned j = 0; j < mesh->getSizeFE(); j++)
        {
            data[int(j)].setX(cX(mesh->getFE(i, j)) - x0[0]);
            data[int(j)].setY(cY(mesh->getFE(i, j)) - x0[1]);
            data[int(j)].setZ(cZ2D(mesh->getFE(i, j)) - x0[2]);
            data[int(j)].setW( (results) ? float((*results)[mesh->getFE(i, j)]) : vertex[mesh->getFE(i, j)].w());
        }
        drawPolygon(data);
    }
}
/*******************************************************************/
void TGLFunction::drawFun3D(void)
{
    QVector<QVector4D> data(int(mesh->getSizeBE()));

    for (unsigned i = 0; i < mesh->getNumBE(); i++)
    {
        for (unsigned j = 0; j < mesh->getSizeBE(); j++)
        {
            data[int(j)].setX(cX(mesh->getBE(i, j)) - x0[0]);
            data[int(j)].setY(cY(mesh->getBE(i, j)) - x0[1]);
            data[int(j)].setZ(cZ(mesh->getBE(i, j)) - x0[2]);
            data[int(j)].setW( (results) ? float((*results)[mesh->getBE(i, j)]) : vertex[mesh->getBE(i, j)].w());
        }
        drawPolygon(data);
    }
}
/*******************************************************************/
void TGLFunction::drawSegment(QVector<QVector4D>& vertex)
{
    int iColor1 = getColorIndex(vertex[0].w()),
        iColor2 = getColorIndex(vertex[1].w()),
        iColor,
        numSteps = abs(iColor1 - iColor2);
    float x,
          h = (numSteps > 0) ? (vertex[1].x() - vertex[0].x())/float(numSteps) : 0;


//    if (params.isFace)
    {
        glNormal3d(0, 0, 1);
        glBegin(GL_LINE_STRIP);
            setColor(iColor1);
            glVertex2f(vertex[0].x(), 0);
            x = vertex[0].x();
            iColor = iColor1;
            for (int i = 0; i < numSteps; i++)
            {
                x += h;
                if (iColor1 < iColor2)
                    iColor++;
                else
                    iColor--;
                setColor(iColor);
                glVertex2f(x, 0);
            }
            setColor(iColor2);
            glVertex2f(vertex[1].x(), 0);
        glEnd();
    }
    if (params.isVertex)
    {
        glPointSize(3);
        glBegin(GL_POINTS);
            for (int i = 0; i < vertex.size(); i++)
            {
                iColor = getColorIndex(vertex[i].w());
                setColor(iColor);
                glPushMatrix();
                    glTranslatef(vertex[i].x(),0,0);
                    glVertex3f(vertex[i].x(), 0, 0);
                glPopMatrix();
            }
        glEnd();
    }
}
/*******************************************************************/
void TGLFunction::drawPolygon(QVector<QVector4D>& vertex)
{
    QVector<QVector4D> vtx(3);
    int ind[] = { 0, 1, 2, 3 }; // Порядок обхода вершин четырехугольника
    int size = (vertex.size() == 4) ? 4 : 3;


    if (params.isFace)
        switch (vertex.size())
        {
            case 3: // Рисование линейного треугольника
                drawTriangle3D(vertex);
                break;
            case 4: // Рисование четырехугольника
                vtx[0] = vertex[ind[1]];
                vtx[1] = vertex[ind[0]];
                vtx[2] = vertex[ind[2]];
                drawTriangle3D(vtx);
                vtx[0] = vertex[ind[0]];
                vtx[1] = vertex[ind[3]];
                vtx[2] = vertex[ind[2]];
                drawTriangle3D(vtx);
                break;
            case 6: // Рисование квадратичного треугольника
                vtx[0] = vertex[0];
                vtx[1] = vertex[3];
                vtx[2] = vertex[5];
                drawTriangle3D(vtx);
                vtx[0] = vertex[3];
                vtx[1] = vertex[1];
                vtx[2] = vertex[4];
                drawTriangle3D(vtx);
                vtx[0] = vertex[3];
                vtx[1] = vertex[4];
                vtx[2] = vertex[5];
                drawTriangle3D(vtx);
                vtx[0] = vertex[5];
                vtx[1] = vertex[4];
                vtx[2] = vertex[2];
                drawTriangle3D(vtx);
        }
    if (params.isMesh)
    {
        TGLMesh::setColor(0, 0, 0, params.alpha);
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < size; j++)
            glVertex3f(vertex[j].x(), vertex[j].y(), vertex[j].z());
        glEnd();
    }
    if (params.isVertex)
    {
        glPointSize(3);
        glBegin(GL_POINTS);
        TGLMesh::setColor(0, 0, 0, params.alpha);
        for (int i = 0; i < vertex.size(); i++)
            glVertex3f(vertex[i].x(), vertex[i].y(), vertex[i].z());
        glEnd();
    }
}
/*******************************************************************/
void TGLFunction::initColorTable(void)
{
    float step = float(params.numColor) / 6.0f,
          h = 256.0f / step,
          red = 148.0f, //255,
          green = 0.0f,
          blue = 255.0f;

    colorTable.clear();
    if (results)
    {
        min_u = float(*min_element(results->begin(), results->end()));
        max_u = float(*max_element(results->begin(), results->end()));
    }
    else
    {
        min_u = std::numeric_limits<float>::max();
        max_u = std::numeric_limits<float>::min();
        foreach (QVector4D v, vertex)
        {
            min_u = qMin(min_u, v.w());
            max_u = qMax(max_u, v.w());
        }
    }
//    if (min_u == max_u)
//        max_u += 1.0f;

    if (params.isBW)
    {
        step = float(params.numColor);
        h = (256.0f - 50.0f)/step;
        red = green = blue = 25;
        for (int i = 0; i < params.numColor; i++)
        {
            colorTable.append(QColor(int(red), int(green), int(blue), int(params.alpha*255.0f)));
            red += h;
            green = blue = red;
        }
    }
    else
        for (int i = 0; i < params.numColor; i++)
            if (i < step)
            {
                // фиолетовый-синий
                colorTable.append(QColor(int(red)/2, 0, /*211*/255, int(params.alpha*255.0f)));
                red -= h;
                if (red < 0)
                    red = 0;
            }
            else if (step <= i && i < 2*step)
            {
                // синий-голубой
                colorTable.append(QColor(0, int(green), 255, int(params.alpha*255.0f)));
                green += h;
                if (green > 255)
                    green = 255;
            }
            else if (2*step <= i && i < 3*step)
            {
                // голубой-зеленый
                colorTable.append(QColor(0, 255, int(blue), int(params.alpha*255.0f)));
                blue -= h;
                if (blue < 0)
                    blue = 0;
            }
            else if (3*step <= i && i < 4*step)
            {
                // зеленый-желтый
                colorTable.append(QColor(int(red), 255, 0, int(params.alpha*255.0f)));
                red += h;
                if (red > 255)
                    red = 255;
            }
            else if (i > 4*step)
            {
                // желтый-оранжевый-красный
                colorTable.append(QColor(255, int(green), 0, int(params.alpha*255.0f)));
                green -= 0.5f*h;
                if (green < 0 || i == params.numColor - 2)
                    green = 0;
            }
    if (params.isNegative)
        for (int i = 0; i < params.numColor; i++)
        {
            colorTable[i].setRed(255 - colorTable[i].red());
            colorTable[i].setGreen(255 - colorTable[i].green());
            colorTable[i].setBlue(255 - colorTable[i].blue());
        }

}
/*******************************************************************/
void TGLFunction::showLegend(void)
{
    QRect rc = rect();
    QFont glFont("Courier", 12, QFont::Normal);
    QFontMetrics fm(glFont);
    QString val;
    int num = (fabs(min_u - max_u) < 1.0E-20f) ? 1 : 8,
        h = colorTable.size()/num,
        fontW1 = fm.horizontalAdvance("12"),
        fontW2 = fm.horizontalAdvance("1234567890"),
        fontH = fm.height();
    float start = max_u,
          stop = min_u,
          step = (max_u - min_u) / num,
          v = start,
          cy = rc.top() + 20;

    if (!(mesh && mesh->getTypeFE() != NOTYPE) || rc.height() < 9*fontH)
        return;

    for (int k = colorTable.size() - 1; k >= 0; k -= h)
    {
        if (k - h < 0)
        {
            v = stop;
            k = 0;
        }
        glColor3f(float(colorTable[k].redF()), float(colorTable[k].greenF()), float(colorTable[k].blueF()));
        glFont.setStyleStrategy(QFont::OpenGLCompatible);
        renderText(rc.width() - fontW1 - fontW2 - 10, int(cy) ,"█",glFont);
        glColor3f(1.0f - float(params.bkgColor.redF()), 1.0f - float(params.bkgColor.greenF()), 1.0f - float(params.bkgColor.blueF()));
        renderText(rc.width() - fontW2 - 10, int(cy), val.asprintf("%+5.3E", double(v)), glFont);
        cy += fontH;
        v -= step;
    }
}
/*******************************************************************/
int sort(QVector<QVector4D>& vtx)
{
    QVector4D temp;
    int ret = 1,
        min_i = 0;
    float min_u = vtx[0].w();

    for (int i = 1; i < 3; i++)
        if (vtx[i].w() <min_u)
        {
            min_i = i;
            min_u = vtx[i].w();
        }
    for (int i = 0; i < min_i; i++)
    {
        temp = vtx[0];
        vtx.pop_front();
        vtx.append(temp);
    }
    if (vtx[1].w() > vtx[2].w())
    {
        ret = -1;
        temp = vtx[1];
        vtx.remove(1);
        vtx.push_back(temp);
    }
    return ret;
}
/*******************************************************************/
void TGLFunction::drawTriangle3D(QVector<QVector4D>& tri)
{
    QVector<QVector4D> p02, p012;
    QVector4D x, h;
    float step,
          a,
          b,
          c;
    int color_index,
        color_ind[3],
        inv = sort(tri); // Сортируем треугольник по возрастанию узловых значений

    glFrontFace((inv == 1) ? GL_CCW : GL_CW);
    for (int i = 0; i < 3; i++)
        color_ind[i] = getColorIndex(tri[i].w());

    if (params.isLight)
    {
        a  = (tri[1].y() - tri[0].y())*(tri[2].z() - tri[0].z()) - (tri[2].y() - tri[0].y())*(tri[1].z() - tri[0].z());
        b  = (tri[2].x() - tri[0].x())*(tri[1].z() - tri[0].z()) - (tri[1].x() - tri[0].x())*(tri[2].z() - tri[0].z());
        c  = (tri[1].x() - tri[0].x())*(tri[2].y() - tri[0].y()) - (tri[2].x() - tri[0].x())*(tri[1].y() - tri[0].y());
        glNormal3f(float(inv)*a, float(inv)*b, float(inv)*c);
    }

    if (color_ind[0] == color_ind[1] && color_ind[1] == color_ind[2])
    {
        // Треугольник одного цвета
        glBegin(GL_TRIANGLES);
            setColor(int(color_ind[0]));
            glVertex3f(tri[0].x(), tri[0].y(), tri[0].z());
            glVertex3f(tri[1].x(), tri[1].y(), tri[1].z());
            glVertex3f(tri[2].x(), tri[2].y(), tri[2].z());
        glEnd();
    }
    else
    {
        // Изолинии проходят по треугольнику
        step = color_ind[2] - color_ind[0] + 1;
        x = QVector4D(tri[0].x(), tri[0].y(), tri[0].z(), color_ind[0]);
        h = QVector4D((tri[2].x() - tri[0].x())/step, (tri[2].y() - tri[0].y())/step, (tri[2].z() - tri[0].z())/step, (color_ind[2] - color_ind[0])/step);

        for (int i = 0; i < step; i++)
            p02.push_back(QVector4D(x.x() + i*h.x(), x.y() + i*h.y(), x.z() + i*h.z(), color_ind[0] + i*h.w()));
        p02.push_back(QVector4D(tri[2].x(), tri[2].y(), tri[2].z(), color_ind[2]));

        step = color_ind[1] - color_ind[0] + 1;
        x = QVector4D(tri[0].x(), tri[0].y(), tri[0].z(), color_ind[0]);
        h = QVector4D((tri[1].x() - tri[0].x())/step, (tri[1].y() - tri[0].y())/step, (tri[1].z() - tri[0].z())/step, (color_ind[1] - color_ind[0])/step);
        for (int i = 1; i < step; i++)
            p012.push_back(QVector4D(x.x() + i*h.x(), x.y() + i*h.y(), x.z() + i*h.z(), color_ind[0] + i*h.w()));
        p012.push_back(QVector4D(tri[1].x(), tri[1].y(), tri[1].z(), color_ind[1]));

        step = color_ind[2] - color_ind[1] + 1;
        x = QVector4D(tri[1].x(), tri[1].y(), tri[1].z(), color_ind[1]);
        h = QVector4D((tri[2].x() - tri[1].x())/step, (tri[2].y() - tri[1].y())/step, (tri[2].z() - tri[1].z())/step, (color_ind[2] - color_ind[1])/step);
        for (int i = 1; i < step; i++)
            p012.push_back(QVector4D(x[0] + i*h[0], x[1] + i*h[1], x[2] + i*h[2], color_ind[1] + i*h[3]));

        for (int i = 0; i < p02.length() - 1; i++)
            if (i < p012.length())
            {
                color_index = int(round(min(min(p02[i].w(), p02[i + 1].w()), p012[i].w())));
                setColor(color_index);
                glBegin(GL_TRIANGLES);
                    glVertex3f(p02[i + 1].x(), p02[i + 1].y(), p02[i + 1].z());
                    glVertex3f(p02[i].x(), p02[i].y(), p02[i].z());
                    glVertex3f(p012[i].x(), p012[i].y(), p012[i].z());
                glEnd();
                if (i + 1 < p012.length())
                {
                    color_index = int(round(min(min(p02[i + 1].w(), p012[i].w()), p012[i + 1].w())));
                    setColor(color_index);
                    glBegin(GL_TRIANGLES);
                        glVertex3f(p02[i + 1].x(), p02[i + 1].y(), p02[i + 1].z());
                        glVertex3f(p012[i].x(), p012[i].y(), p012[i].z());
                        glVertex3f(p012[i + 1].x(), p012[i + 1].y(), p012[i + 1].z());
                    glEnd();
                }
            }
    }
}
/*******************************************************************/
int TGLFunction::getColorIndex(float u)
{
    int ret = int(floor((u - min_u) / (max_u - min_u) * float(params.numColor)));

    return (ret < 0) ? 0 : ((ret > params.numColor - 1) ? params.numColor - 1 : ret);
}
/*******************************************************************/
float TGLFunction::cX(unsigned i)
{
    return float(mesh->getX(i, 0)) + params.koff * ((dx) ? float((*dx)[i]) : 0);
}
/*******************************************************************/
float TGLFunction::cY(unsigned i)
{
    return float(mesh->getX(i, 1)) + params.koff * ((dy) ? float((*dy)[i]) : 0);
}
/*******************************************************************/
float TGLFunction::cZ(unsigned i)
{
    return float(mesh->getX(i, 2)) + params.koff * ((dz) ? float((*dz)[i]) : 0);
}
/*******************************************************************/
float TGLFunction::cZ2D(unsigned i)
{
    return (mesh->isPlate()) ? params.koff * ((dz) ? float((*dz)[i]) : 0) : 0;
}
/*******************************************************************/
void TGLFunction::mouseDoubleClickEvent(QMouseEvent* e)
{
    float colors[3] = { 0.0f, 0.0f, 0.0f };
    float u = min_u,
          h = (max_u - min_u)/float(colorTable.size()),
          eps = 0.01f;

    if (params.isLight)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Turn off lights!"));
        return;
    }
    glReadPixels(e->x(), size().height()- e->y(), 1, 1, GL_RGB, GL_FLOAT, colors);

    for (int i = 0; i < colorTable.size(); i++)
    {
        if (fabs(float(colorTable[i].redF()) - colors[0]) < eps && fabs(float(colorTable[i].greenF()) - colors[1]) < eps && fabs(float(colorTable[i].blueF()) - colors[2]) < eps)
        {
            QToolTip::showText(QPoint(e->x(), size().height()-e->y()), QString::number(double(u),'E'));
            return;
        }
        u += h;
    }
}
/*******************************************************************/

// Si=((Sxx - Syy) ** 2 + (Sxx - Szz) ** 2 + (Syy - Szz) ** 2 + 6 * (Sxy ** 2 + Sxz ** 2 + Syz ** 2)) ** 0.5 / 1.41
