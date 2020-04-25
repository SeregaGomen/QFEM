#ifndef BCPROCESSOR_H
#define BCPROCESSOR_H

#include <QObject>
#include <QVector>
#include <QVector4D>

class TFEMObject;
enum class ParamType;
enum class ErrorCode;

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
    int numThread = 1;
    // Тип визуализируемого параметра
    ParamType paramType;
    // Объект рассчета (содержит геометрию)
    TFEMObject* object;
    // Вектор значений нагрузок, краевых условий, etc
    QVector<QVector4D> vertex;
    // Признак того, что процесс прерван
    bool isStoped;
    // Формирование списка краевых условий
    void processVertex(void);
    void calcParam(unsigned, unsigned, ErrorCode&);
    void calcPressureLoad(unsigned, unsigned, ErrorCode&);
    void calcVolumeLoad(unsigned, unsigned, ErrorCode&);
    void calcConcentratedLoad(unsigned, unsigned, ErrorCode&);
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
    void setType(ParamType type)
    {
        paramType = type;
    }
    void setNumTheread(int n)
    {
        numThread = n;
    }
};

#endif // BCPROCESSOR_H
