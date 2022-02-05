#include "femprocessor.h"
#include "object/object.h"

TFEMProcessor::TFEMProcessor(void) : QObject()
{
    femObject = new TFEMObject();
}

TFEMProcessor::~TFEMProcessor(void)
{
    if (femObject)
        delete femObject;
}

// Запуск расчета
void TFEMProcessor::start(void)
{
    femObject->start();
    emit finished();
}

// Остановка расчета
void TFEMProcessor::stop(void)
{
    if (femObject)
        femObject->stop();
}

// Проверка, выполнен ли расчет
bool TFEMProcessor::isCalculated(void)
{
    return femObject->isCalculated();
}

// Проверка, запущен ли расчет
bool TFEMProcessor::isStarted(void)
{
    return femObject->isStarted();
}

// Инициализация объекта расчета
void TFEMProcessor::clear(void)
{
    femObject->clear();
}
