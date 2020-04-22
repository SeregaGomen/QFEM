#ifndef TFEMPROCESSOR_H
#define TFEMPROCESSOR_H


#include <QObject>

class TFEMObject;

/**************************************************************************/
/* Обертка для класса, реализующего вычисление конечно-элементной задачи  */
/**************************************************************************/
class TFEMProcessor : public QObject
{
    Q_OBJECT

signals:
    void finished(void); // сигнал о завершении расчета

private:
    TFEMObject* femObject; // объект расчет

public slots:
    void start(void);  // запуск расчета
    void stop(void); // остановка расчета

public:
    TFEMProcessor(void);
    ~TFEMProcessor(void);
    TFEMObject* getFEMObject(void) const
    {
        return femObject;
    }
    bool isCalculated(void);
    bool isStarted(void);
    void clear(void);
};

#endif // TFEMPROCESSOR_H
