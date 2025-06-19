#ifndef TTERMINAL_H
#define TTERMINAL_H

#include <QTextEdit>
#include <QThread>

class TTerminal : public QTextEdit
{
private:
    QThread* thread;
protected:
    void keyPressEvent(QKeyEvent *event)
    {
        if (thread->isRunning())
            return;
        QTextEdit::keyPressEvent(event);
    }
    void mousePressEvent(QMouseEvent *event)
    {
        if (thread->isRunning())
            return;
        QTextEdit::mousePressEvent(event);
    }
    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        if (thread->isRunning())
            return;
        QTextEdit::mouseDoubleClickEvent(event);
    }
    void contextMenuEvent(QContextMenuEvent *event)
    {
        if (thread->isRunning())
            return;
        QTextEdit::contextMenuEvent(event);
    }
public:
    TTerminal(QThread* t, QWidget* parent = nullptr) : QTextEdit(parent), thread(t) {}
   ~TTerminal(void) {}
};

#endif // TTERMINAL_H
