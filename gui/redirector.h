#ifndef REDIRECTOR_H
#define REDIRECTOR_H

#pragma once
#include <QTextEdit>
#include <QString>
#include <QMetaObject>
#include <QTextCharFormat>
#include <streambuf>
#include <iostream>

class TStreamRedirector : public std::streambuf
{
public:
    TStreamRedirector(QTextEdit *textEdit, std::ostream &stream, const QColor &color = Qt::black)
        : m_textEdit(textEdit), m_stream(stream), m_color(color)
    {
        m_oldBuf = stream.rdbuf(this);
    }

    ~TStreamRedirector()
    {
        m_stream.rdbuf(m_oldBuf);
    }

protected:
    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        QString text = QString::fromLocal8Bit(s, static_cast<int>(n));
        appendToTextEdit(text);
        return n;
    }

    int overflow(int c) override
    {
        if (c != EOF)
        {
            char ch = static_cast<char>(c);
            appendToTextEdit(QString(ch));
        }
        return c;
    }

private:
    void appendToTextEdit(const QString& text)
    {
        if (!m_textEdit) return;

        QMetaObject::invokeMethod(m_textEdit, [=]() {
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);

            QTextCharFormat format;
            format.setForeground(m_color);
            cursor.setCharFormat(format);

            if (text.contains('\r'))
            {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.select(QTextCursor::BlockUnderCursor);
                //cursor.insertText(text);
            }
            cursor.insertText(text);
            m_textEdit->setTextCursor(cursor); // Прокрутка вниз
        }, Qt::QueuedConnection);

        // m_textEdit->moveCursor(QTextCursor::End);
        // m_textEdit->ensureCursorVisible();
    }

    QTextEdit *m_textEdit;
    std::ostream &m_stream;
    std::streambuf *m_oldBuf;
    QColor m_color;
};

#endif // REDIRECTOR_H
