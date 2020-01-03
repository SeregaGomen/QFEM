#ifndef IMAGEPARAMS_H
#define IMAGEPARAMS_H

#include <QColor>
#include <QDataStream>
#include <fstream>


using namespace std;

class TImageParams
{
public:
    bool isMesh;       // Отображение сетки
    bool isVertex;     // ... узлов
    bool isFace;       // ... поверхности
    bool isCoord;      // ... осей координат
    bool isLight;      // ... освещения
    bool isShowLegend; // ... шкалы значений
    bool isLimit;      // ... граничных условий
    bool isLoad;      // ... нагрузок
    bool isNormal;     // ... нормалей к поверхности
    bool isDoubleSided;// Двусторонее освещение полигонов (может тормозить)
    bool isColor;      // Цветная (RGB) визуализация
    bool isSpectral;   // Цветная (спектральная) визуализация
    bool isNegative;   // Негативное отображение
    bool isBW;         // Черно-белое отображение
    int numColor;      // Кол-во градаций цвета (цветов) при отображении
    int angleX;        // Угол поворота вокруг оси Х
    int angleY;        // ... Y
    int angleZ;        // ... Z
    float translateX;  // Коэффициент смещения вдоль оси X
    float translateY;  // ... Y
    float translateZ;  // ... Z
    float koff;        // Коэффициент усиления трансформации
    float alpha;       // Прозрачность материала
    float scale;       // Коэффициент масштабирования
    QColor bkgColor;   // Цвет фона изображения
    TImageParams(void)
    {
        init();
    }
    ~TImageParams(void) {}
    TImageParams& operator = (TImageParams& right)
    {
        koff = right.koff;
        alpha = right.alpha;
        numColor = right.numColor;
        isColor = right.isColor;
        isCoord = right.isCoord;
        isLight = right.isLight;
        isDoubleSided = right.isDoubleSided;
        isMesh = right.isMesh;
        isFace = right.isFace;
        isVertex = right.isVertex;
        isSpectral = right.isSpectral;
        isNegative = right.isNegative;
        isBW = right.isBW;
        isShowLegend = right.isShowLegend;
        isLimit = right.isLimit;
        isLoad = right.isLoad;
        isNormal = right.isNormal;
        bkgColor = right.bkgColor;
        scale = right.scale;
        angleX = right.angleX;
        angleY = right.angleY;
        angleZ = right.angleZ;
        translateX = right.translateX;
        translateY = right.translateY;
        translateZ = right.translateZ;
        return *this;
    }
    void write(QDataStream& file)
    {
        file << koff;
        file << alpha;
        file << numColor;
        file << isColor;
        file << isCoord;
        file << isLight;
        file << isDoubleSided;
        file << isMesh;
        file << isFace;
        file << isVertex;
        file << isSpectral;
        file << isNegative;
        file << isBW;
        file << isShowLegend;
        file << isLimit;
        file << isLoad;
        file << bkgColor;
        file << scale;
        file << angleX;
        file << angleY;
        file << angleZ;
        file << translateX;
        file << translateY;
        file << translateZ;
        file << isNormal;
    }
    void read(QDataStream& file)
    {
        file >> koff;
        file >> alpha;
        file >> numColor;
        file >> isColor;
        file >> isCoord;
        file >> isLight;
        file >> isDoubleSided;
        file >> isMesh;
        file >> isFace;
        file >> isVertex;
        file >> isSpectral;
        file >> isNegative;
        file >> isBW;
        file >> isShowLegend;
        file >> isLimit;
        file >> isLoad;
        file >> bkgColor;
        file >> scale;
        file >> angleX;
        file >> angleY;
        file >> angleZ;
        file >> translateX;
        file >> translateY;
        file >> translateZ;
        file >> isNormal;
    }
    void init(void)
    {
        angleX =
        angleY =
        angleZ = 0;
        koff =
        translateX =
        translateY =
        translateZ = 0.0;
        alpha = scale = 1.0;
        numColor = 16;
        isColor =
        isCoord =
        isLight =
        isFace =
        isDoubleSided =
        isShowLegend = true;
        isMesh =
        isVertex =
        isSpectral =
        isNegative =
        isLimit =
        isLoad =
        isNormal =
        isBW = false;
        bkgColor = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0).darker();
    }
};

#endif // IMAGEPARAMS_H
