#ifndef QMSG_H
#define QMSG_H

#include <QApplication>
#include <QProgressBar>
#include "msg/msg.h"

//--------------------------------------------------------------------------
//      Реализация анимации прогресса вычислений с добавленными слотами
//--------------------------------------------------------------------------
class SProgressBar : public QProgressBar
{
    Q_OBJECT

public slots:
    void slotSetTextVisible(bool isVisible)
    {
        QProgressBar::setTextVisible(isVisible);
    }
    void slotSetFormat(const QString& format)
    {
        QProgressBar::setFormat(format);
    }

public:
    SProgressBar(QWidget *parent = nullptr) : QProgressBar(parent) {}
    ~SProgressBar(void) {}
};

//--------------------------------------------------------------------------
// Реализация системы сообщений для QT GUI с использованием сигналов/слотов
//--------------------------------------------------------------------------
class SProcessMessenger : public QObject, public TMessenger
{
    Q_OBJECT

signals:
    void signalSetValue(int);
    void signalSetMinimum(int);
    void signalSetMaximum(int);
    void signalSetTextVisible(bool);
    void signalSetFormat(const QString&);

public:
    SProcessMessenger(void) : QObject(), TMessenger() {}
    ~SProcessMessenger(void) {}
//    void setProcess(void)
//    {
//        TMessenger::setProcess();
//        emit signalSetValue(0);
//        QCoreApplication::processEvents();
//    }
    void setProcess(ProcessCode code)
    {
        TMessenger::setProcess(code);
        emit signalSetTextVisible(false);
        emit signalSetMinimum(0);
        emit signalSetMaximum(0);
        emit signalSetValue(0);
        QCoreApplication::processEvents();
    }
    void setProcess(ProcessCode code,int start,int stop, int step = 0)
    {
        TMessenger::setProcess(code, start, stop, step);
        emit signalSetTextVisible(true);
        emit signalSetMinimum(1);
        emit signalSetMaximum(100);
        emit signalSetValue(1);
        QCoreApplication::processEvents();
    }
    void addProgress(void)
    {
        TMessenger::addProgress();
//        if (TMessenger::oldPersent % /*TMessenger::processStep*/5 == 0)
        {
            emit signalSetFormat(QString("%1: %2%").arg(sayProcess(processCode).c_str()).arg(TMessenger::oldPersent));
            if (processCurrent == processStop)
                emit signalSetValue(TMessenger::processStart);
            else
                emit signalSetValue(TMessenger::oldPersent);
            QCoreApplication::processEvents();
        }
    }
};

#endif // QMSG_H
