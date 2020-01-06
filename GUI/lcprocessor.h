#ifndef LCPROCESSOR_H
#define LCPROCESSOR_H

#include <QObject>

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
    LimitList* lcVertex;
    // Признак того, что процесс прерван
    bool isStoped;
    // Формирование списка краевых условий
    void processBoundaryVertex(void);
    void calc(unsigned, unsigned, int, int, string, string, int&);
public slots:
    // запуск расчета
    void start(void);
    // остановка расчета
    void stop(void);

public:
    TLCProcessor(TFEMObject*);
    ~TLCProcessor(void);
    LimitList* getLCVertex(void)
    {
        return lcVertex;
    }
    void clear(void);
};

#endif // LCPROCESSOR_H
