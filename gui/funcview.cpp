#include <QPainter>
#include "imageparams.h"
#include "funcview.h"
#include "mesh/mesh.h"

void TFunctionView::getMinMax()
{
    min_u[0] = *min_element(results->begin(), results->end());
    max_u[0] = *max_element(results->begin(), results->end());
    min_u[1] = abs(min_u[0]) > abs(max_u[0]) ? -abs(min_u[0]) : -abs(max_u[0]);
    max_u[1] = abs(min_u[1]);


    maxTransformRatio = 0;
    for (auto i = 0u; i < delta.size(); i++)
        if (delta[i] != nullptr)
        {
            double maxU = std::max(fabs(*min_element(delta[i]->begin(), delta[i]->end())), fabs(*max_element(delta[i]->begin(), delta[i]->end())));

            if (maxU > maxTransformRatio)
                maxTransformRatio = maxU;
        }
}


void TFunctionView::initColorTable(void)
{
    float step = float(params->numColor) / 6.0f,
          h = 256.0f / step,
          red = 148.0f, //255,
          green = 0.0f,
          blue = 255.0f;

    colorTable.clear();
    getMinMax();
    for (int i = 0; i < params->numColor; i++)
        if (i < step)
        {
            // фиолетовый-синий
            colorTable.append(QColor(int(red)/2, 0, /*211*/255, int(params->alpha*255.0f)));
            red -= h;
            if (red < 0)
                red = 0;
        }
        else if (step <= i and i < 2*step)
        {
            // синий-голубой
            colorTable.append(QColor(0, int(green), 255, int(params->alpha*255.0f)));
            green += h;
            if (green > 255)
                green = 255;
        }
        else if (2*step <= i and i < 3*step)
        {
            // голубой-зеленый
            colorTable.append(QColor(0, 255, int(blue), int(params->alpha*255.0f)));
            blue -= h;
            if (blue < 0)
                blue = 0;
        }
        else if (3*step <= i and i < 4*step)
        {
            // зеленый-желтый
            colorTable.append(QColor(int(red), 255, 0, int(params->alpha*255.0f)));
            red += h;
            if (red > 255)
                red = 255;
        }
        else if (i > 4*step)
        {
            // желтый-оранжевый-красный
            colorTable.append(QColor(255, int(green), 0, int(params->alpha*255.0f)));
            green -= 0.5f*h;
            if (green < 0 or i == params->numColor - 2)
                green = 0;
        }

}

void TFunctionView::showLegend()
{
    QPainter painter(this);
    QRect rc = rect();
    QFont font("Courier", 12, QFont::Normal);
    QFontMetrics fm(font);
    QString val;
    QColor color;
    int fontW1 = fm.horizontalAdvance("12"), fontW2 = fm.horizontalAdvance("1234567890"), fontH = fm.height();
    int start = getColorIndex(min_u[0]), stop = getColorIndex(max_u[0]), cy = rc.top() + 20;
    double step = double(stop - start) / 6.0, v = min_u[0], h = (max_u[0] - min_u[0]) / 6.0;

    if (!(mesh && mesh->getTypeFE() != FEType::undefined) || rc.height() < 9 * fontH or colorTable.size() == 0)
        return;

    painter.setFont(font);
    for (int k = 0; k < (min_u[0] == max_u[0] ? 1 : 7); k++)
    {
        if (k == 6)
            v = max_u[0];
        color = colorTable[k * step + start];
        if (params->isGrayscale)
        {
            int gray = qRound(0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue());
            color = QColor(gray, gray, gray);
        }
        painter.setPen(color);
        painter.drawText(rc.width() - fontW1 - fontW2 - 10, int(cy), "█");
        painter.setPen(QColor(255 - float(params->bkgColor.red()), 255 - float(params->bkgColor.green()), 255 - float(params->bkgColor.blue())));
        painter.drawText(rc.width() - fontW2 - 10, int(cy), val.asprintf("%+5.3E", double(v)));
        cy += fontH;
        v += h;
    }
    painter.end();
}


void TFunctionView::initObject()
{
    matrix<unsigned> &elm = (mesh->is3D()) ? mesh->getBE() : mesh->getFE();
    QVector<QVector4D> data(elm.size2());
    QVector<float> vertices;

    initColorTable();

    numObjectPoints = 0;

    vertices.reserve(elm.size1()*27);
    for (auto i = 0u; i < elm.size1(); i++)
    {
        for (auto j = 0u; j < elm.size2(); j++)
        {
            data[j].setX(x(elm(i, j)) - x0);
            data[j].setY(y(elm(i, j)) - y0);
            data[j].setZ(z(elm(i, j)) - z0);
            data[j].setW(getValue(elm(i, j)));
        }
        if (mesh->is1D())
            createSegment(data, vertices);
        else
            createPolygon(data, vertices);
    }



    if (!vao[0].isCreated())
        vao[0].create();
    vao[0].bind();

    if (!vbo[0].isCreated())
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

}

void TFunctionView::getColor(int index, QVector<float> &color)
{
    colorTable[index].getRgbF(&color[0], &color[1], &color[2]);
    if (params->isGrayscale)
    {
        float gray = 0.299 * color[0] + 0.587 * color[1] + 0.114 * color[2];
        color = {gray, gray, gray};
    }
}


void TFunctionView::createSegment(const QVector<QVector4D> &segment, QVector<GLfloat> &vertices)
{
    int index1 = getColorIndex(segment[0].w()), index2 = getColorIndex(segment[1].w()),
        index, numSteps = abs(index1 - index2);
    float x, h = (numSteps > 0) ? (segment[1].x() - segment[0].x())/float(numSteps) : 0;
    QVector<float> color(3);

    getColor(index1, color);
    vertices.append({segment[0].x(), 0, 0, color[0], color[1], color[2], 1, 0, 0});

    x = segment[0].x();
    index = index1;
    for (int i = 0; i < numSteps; i++)
    {
        x += h;
        index1 < index2 ? index++ : index--;
        getColor(index, color);
        vertices.append({x, 0, 0, color[0], color[1], color[2], 1, 0, 0});
        numObjectPoints += 2;
        vertices.append({x, 0, 0, color[0], color[1], color[2], 1, 0, 0});
    }
    getColor(index2, color);
    vertices.append({segment[1].x(), 0, 0, color[0], color[1], color[2], 1, 0, 0});
    numObjectPoints += 2;
}

void TFunctionView::createPolygon(const QVector<QVector4D> &polygon, QVector<GLfloat> &vertices)
{
    auto sort = [](const QVector<QVector4D>& vtx) {
        QVector<QVector4D> result(vtx);
        QVector4D tmp;

        if (result[0].w() > result[1].w())
        {
            tmp = result[0];
            result[0] = result[1];
            result[1] = tmp;
        }
        if (result[0].w() > result[2].w())
        {
            tmp = result[0];
            result[0] = result[2];
            result[2] = tmp;
        }
        if (result[1].w() > result[2].w())
        {
            tmp = result[1];
            result[1] = result[2];
            result[2] = tmp;
        }
        return result;
    };


    switch (polygon.size())
    {
    case 3: // Рисование линейного треугольника
        createTriangle3D(sort(polygon), vertices);
        break;
    case 4: // Рисование четырехугольника
        createTriangle3D(sort({polygon[1], polygon[0], polygon[2]}), vertices);
        createTriangle3D(sort({polygon[0], polygon[3], polygon[2]}), vertices);
        break;
    case 6: // Рисование квадратичного треугольника
        createTriangle3D(sort({polygon[0], polygon[3], polygon[5]}), vertices);
        createTriangle3D(sort({polygon[3], polygon[1], polygon[4]}), vertices);
        createTriangle3D(sort({polygon[3], polygon[4], polygon[5]}), vertices);
        createTriangle3D(sort({polygon[5], polygon[4], polygon[2]}), vertices);
    }
}

int TFunctionView::getColorIndex(float u)
{
    int ret = int(floor((u - min_u[1]) / (max_u[1] - min_u[1]) * float(params->numColor)));
    return (ret < 0) ? 0 : ((ret > colorTable.size() - 1) ? colorTable.size() - 1 : ret);
}

void TFunctionView::createTriangle3D(const QVector<QVector4D> &tri, QVector<GLfloat> &vertices)
{
    QVector<QVector4D> p02, p012;
    QVector4D x, h;
    float step;
    int colorIndex[3];
    auto normal{[](const QVector<QVector4D>& tri){
        array<float, 3> n;
        n[0] = (tri[1].y() - tri[0].y())*(tri[2].z() - tri[0].z()) - (tri[2].y() - tri[0].y())*(tri[1].z() - tri[0].z());
        n[1] = (tri[2].x() - tri[0].x())*(tri[1].z() - tri[0].z()) - (tri[1].x() - tri[0].x())*(tri[2].z() - tri[0].z());
        n[2] = (tri[1].x() - tri[0].x())*(tri[2].y() - tri[0].y()) - (tri[2].x() - tri[0].x())*(tri[1].y() - tri[0].y());
        float len = pow(n[0] * n[0] + n[1] * n[1] + n[2] * n[2], 0.5);
        return array<float, 3>{n[0]/len, n[1]/len, n[2]/len};
    }(tri)};

    for (int i = 0; i < 3; i++)
        colorIndex[i] = getColorIndex(tri[i].w());


    if (colorIndex[0] == colorIndex[1] && colorIndex[1] == colorIndex[2])
        addTriangle(tri, vertices, normal, colorIndex[0]);   // Треугольник одного цвета
    else
    {
        // Изолинии проходят по треугольнику
        step = colorIndex[2] - colorIndex[0] + 1;
        x = QVector4D(tri[0].x(), tri[0].y(), tri[0].z(), colorIndex[0]);
        h = QVector4D((tri[2].x() - tri[0].x())/step, (tri[2].y() - tri[0].y())/step, (tri[2].z() - tri[0].z())/step, (colorIndex[2] - colorIndex[0])/step);

        for (int i = 0; i < step; i++)
            p02.push_back(QVector4D(x.x() + i*h.x(), x.y() + i*h.y(), x.z() + i*h.z(), colorIndex[0] + i*h.w()));
        p02.push_back(QVector4D(tri[2].x(), tri[2].y(), tri[2].z(), colorIndex[2]));

        step = colorIndex[1] - colorIndex[0] + 1;
        x = QVector4D(tri[0].x(), tri[0].y(), tri[0].z(), colorIndex[0]);
        h = QVector4D((tri[1].x() - tri[0].x())/step, (tri[1].y() - tri[0].y())/step, (tri[1].z() - tri[0].z())/step, (colorIndex[1] - colorIndex[0])/step);
        for (int i = 1; i < step; i++)
            p012.push_back(QVector4D(x.x() + i*h.x(), x.y() + i*h.y(), x.z() + i*h.z(), colorIndex[0] + i*h.w()));
        p012.push_back(QVector4D(tri[1].x(), tri[1].y(), tri[1].z(), colorIndex[1]));

        step = colorIndex[2] - colorIndex[1] + 1;
        x = QVector4D(tri[1].x(), tri[1].y(), tri[1].z(), colorIndex[1]);
        h = QVector4D((tri[2].x() - tri[1].x())/step, (tri[2].y() - tri[1].y())/step, (tri[2].z() - tri[1].z())/step, (colorIndex[2] - colorIndex[1])/step);
        for (int i = 1; i < step; i++)
            p012.push_back(QVector4D(x[0] + i*h[0], x[1] + i*h[1], x[2] + i*h[2], colorIndex[1] + i*h[3]));

        for (int i = 0; i < p02.length() - 1; i++)
            if (i < p012.length())
            {
                addTriangle({p02[i + 1], p02[i], p012[i]}, vertices, normal, int(round(min(min(p02[i].w(), p02[i + 1].w()), p012[i].w()))));
                if (i + 1 < p012.length())
                    addTriangle({p02[i + 1], p012[i], p012[i + 1]}, vertices, normal, int(round(min(min(p02[i + 1].w(), p012[i].w()), p012[i + 1].w()))));
            }
    }
}

void TFunctionView::addTriangle(const QVector<QVector4D> &tri, QVector<GLfloat> &vertices, const array<float, 3> &normal, int index)
{
    QVector<float> color(3);

    getColor(index, color);
    for (auto i = 0; i < 3; i++)
    {
        vertices.append({tri[i][0], tri[i][1], tri[i][2]});
        vertices.append(color);
        vertices.append({normal[0], normal[1], normal[2]});
    }
    numObjectPoints += 3;
}

double TFunctionView::x(unsigned i)
{
    return TMeshView::x(i) + (delta.size() == 0 || delta[0] == nullptr ? 0 : (mesh->isPlate() ? 0 : params->ratio*radius*((*delta[0])[i]/maxTransformRatio)));
}

double TFunctionView::y(unsigned i)
{
    return TMeshView::y(i) + (delta.size() < 2 || delta[1] == nullptr ? 0 : (mesh->isPlate() ? 0 : params->ratio*radius*((*delta[1])[i]/maxTransformRatio)));
}

double TFunctionView::z(unsigned i)
{
    return TMeshView::z(i) + (delta.size() < 3  || delta[2] == nullptr? 0 : (mesh->isPlate() ? params->ratio*radius*((*delta[0])[i]/maxTransformRatio) : params->ratio*radius*((*delta[2])[i]/maxTransformRatio)));
}


// Si=((Sxx - Syy) ** 2 + (Sxx - Szz) ** 2 + (Syy - Szz) ** 2 + 6 * (Sxy ** 2 + Sxz ** 2 + Syz ** 2)) ** 0.5 / 1.41
