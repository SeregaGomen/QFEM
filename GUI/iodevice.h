#ifndef IODEVICE_H
#define IODEVICE_H

#include <QtGui>
#include <QMainWindow>

class TextEditIoDevice : public QIODevice
{
    Q_OBJECT

public:
    TextEditIoDevice(QColor& color, QTextEdit* const textEdit, QObject* const parent)  : QIODevice(parent), textEdit(textEdit), curColor(color)
    {
        open(QIODevice::WriteOnly|QIODevice::Text);
    }

protected:
    qint64 readData(char* /*data*/, qint64 /*maxSize*/)
    {
        return 0;
    }
    qint64 writeData(const char* data, qint64 maxSize)
    {
        if (textEdit)
        {
            textEdit->setTextColor(curColor);
            textEdit->insertPlainText(data);
        }
        return maxSize;
    }

private:
    QPointer<QTextEdit> textEdit;
    QColor curColor;
};

#endif // IODEVICE_H
