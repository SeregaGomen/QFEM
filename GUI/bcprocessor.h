#ifndef BCPROCESSOR_H
#define BCPROCESSOR_H

#include <QObject>
#include <QVector3D>

class TFEMObject;
class TParameter;

using namespace std;

/************************************************************************/
/* Реализация построения вектора узловых нагрузок, краевых условий, etc */
/************************************************************************/
class TLCProcessor : public QObject
{
    Q_OBJECT

signals:
    // сигнал о завершении расчета
    void finished(void);

private:
    // Объект рассчета (содержит геометрию)
    TFEMObject* object;
    // Вектор значений нагрузок, краевых условий, etc
    QVector<QVector3D> vertex;
    // Признак того, что процесс прерван
    bool isStoped;
    // Формирование списка краевых условий
    void processVertex(void);
    void calc(int, unsigned, unsigned, int&);
    void calcPressureLoad(unsigned, TParameter&, int&);
    void calcSurfaceLoad(unsigned, TParameter&, int&);
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
    TLCProcessor(TFEMObject* p)
    {
        object = p;
    }
    ~TLCProcessor(void) {}
    QVector<QVector3D>& getVertex(void)
    {
        return vertex;
    }
    void clear(void)
    {
        vertex.clear();
    }
};

#endif // BCPROCESSOR_H
