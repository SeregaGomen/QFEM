#ifndef BCPROCESSOR_H
#define BCPROCESSOR_H

#include <QObject>
#include <QVector>
#include <QVector4D>

class TFEMObject;

using namespace std;

/************************************************************************/
/* Реализация построения вектора узловых нагрузок, краевых условий, etc */
/************************************************************************/
class TBCProcessor : public QObject
{
    Q_OBJECT

signals:
    // сигнал о завершении расчета
    void finished(void);

private:
    // Тип визуализируемого параметра
    int paramType;
    // Объект рассчета (содержит геометрию)
    TFEMObject* object;
    // Вектор значений нагрузок, краевых условий, etc
    QVector<QVector4D> vertex;
    // Признак того, что процесс прерван
    bool isStoped;
    // Формирование списка краевых условий
    void processVertex(void);
    void calcParam(unsigned, unsigned, int&);
    void calcPressureLoad(unsigned, unsigned, int&);
    void calcVolumeLoad(unsigned, unsigned, int&);
    void calcConcentratedLoad(unsigned, unsigned, int&);
public slots:
    // запуск расчета
    void start(void)
    {
        processVertex();
        emit finished();
    }
    // остановка расчета
    void stop(void)
    {
        isStoped = true;
    }
public:
    TBCProcessor(TFEMObject* p)
    {
        object = p;
    }
    ~TBCProcessor(void) {}
    QVector<QVector4D>& getVertex(void)
    {
        return vertex;
    }
    void clear(void)
    {
        vertex.clear();
    }
    void setType(int type)
    {
        paramType = type;
    }
};

#endif // BCPROCESSOR_H
