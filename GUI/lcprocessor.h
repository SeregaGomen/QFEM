#ifndef LCPROCESSOR_H
#define LCPROCESSOR_H

#include <QObject>
#include "lclist.h"

class TFEMObject;
class LimitList;

using namespace std;

/***********************************************************************/
/* Обертка для класса, реализующего построение списка краевых условий  */
/***********************************************************************/
class TLCProcessor : public QObject
{
    Q_OBJECT

signals:
    // сигнал о завершении расчета
    void finished(void);

private:
    // Объект рассчета (содержит геометрию)
    TFEMObject* object;
    // Список краевых условий
    LimitList lcVertex;
    // Признак того, что процесс прерван
    bool isStoped;
    // Формирование списка краевых условий
    void processBoundaryVertex(void);
    void calc(unsigned, unsigned, int, int, string, string, int&);
public slots:
    // запуск расчета
    void start(void)
    {
        processBoundaryVertex();
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
    LimitList& getLCVertex(void)
    {
        return lcVertex;
    }
    void clear(void)
    {
        lcVertex.clear();
    }
};

#endif // LCPROCESSOR_H
