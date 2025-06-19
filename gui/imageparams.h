#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <QColor>
#include <QDataStream>


using namespace std;

class TImageParams
{
public:
    bool isAutoRotate;          // Автоповорот
    bool isMesh;                // Отображение сетки
    bool isVertex;              // ... узлов
    bool isSurface;             // ... поверхности
    bool isAxis;                // ... осей координат
    bool isLight;               // ... освещения
    bool isShowLegend;          // ... шкалы значений
    bool isNormal;              // ... нормалей к поверхности
    bool isDoubleSided;         // Двусторонее освещение полигонов (может тормозить)
    bool isSpectral;            // Цветная (спектральная) визуализация
    bool isNegative;            // Негативное отображение
    bool isGrayscale;           // Черно-белое отображение
    int numColor;               // Кол-во градаций цвета (цветов) при отображении
    array<float, 3> angle;      // Угол поворота вокруг осей Х, Y и Z
    array<float, 3> translate;  // Сдвиг вдоль осей Х, Y и Z
    float ratio;                // Коэффициент усиления трансформации
    float alpha;                // Прозрачность материала
    float scale;                // Коэффициент масштабирования
    QColor bkgColor;            // Цвет фона изображения
    TImageParams(void)
    {
        init();
    }
    ~TImageParams(void) {}
    TImageParams &operator = (TImageParams &right)
    {
        isAutoRotate = right.isAutoRotate;
        ratio = right.ratio;
        alpha = right.alpha;
        numColor = right.numColor;
        isAxis = right.isAxis;
        isLight = right.isLight;
        isDoubleSided = right.isDoubleSided;
        isMesh = right.isMesh;
        isSurface = right.isSurface;
        isVertex = right.isVertex;
        isSpectral = right.isSpectral;
        isNegative = right.isNegative;
        isGrayscale = right.isGrayscale;
        isShowLegend = right.isShowLegend;
        isNormal = right.isNormal;
        bkgColor = right.bkgColor;
        scale = right.scale;
        angle = right.angle;
        translate = right.translate;
        return *this;
    }
    void write(QDataStream& file)
    {
        file << ratio;
        file << alpha;
        file << numColor;
        file << isAxis;
        file << isLight;
        file << isDoubleSided;
        file << isMesh;
        file << isSurface;
        file << isVertex;
        file << isSpectral;
        file << isNegative;
        file << isGrayscale;
        file << isShowLegend;
        file << bkgColor;
        file << scale;
        file << angle[0];
        file << angle[1];
        file << angle[2];
        file << translate[0];
        file << translate[1];
        file << translate[2];
        file << isNormal;
        file << isAutoRotate;
    }
    void read(QDataStream& file)
    {
        file >> ratio;
        file >> alpha;
        file >> numColor;
        file >> isAxis;
        file >> isLight;
        file >> isDoubleSided;
        file >> isMesh;
        file >> isSurface;
        file >> isVertex;
        file >> isSpectral;
        file >> isNegative;
        file >> isGrayscale;
        file >> isShowLegend;
        file >> bkgColor;
        file >> scale;
        file >> angle[0];
        file >> angle[1];
        file >> angle[2];
        file >> translate[0];
        file >> translate[1];
        file >> translate[2];
        file >> isNormal;
        file >> isAutoRotate;
    }
    void init(void)
    {
        angle = {0, 0, 0};
        translate = {0, 0, 0};
        ratio = 0;
        alpha = scale = 1.0;
        numColor = 32;
        isAxis =
        isLight =
        isSurface =
        isDoubleSided =
        isAutoRotate =
        isShowLegend = true;
        isMesh =
        isVertex =
        isSpectral =
        isNegative =
        isNormal =
        isGrayscale = false;
        //bkgColor = QColor::fromCmykF(0.39f, 0.39f, 0.0f, 0.0f).darker();
        bkgColor = QColor::fromRgb(255, 255, 255, 255);
    }
};

#endif // IMAGEPARAMS_H
