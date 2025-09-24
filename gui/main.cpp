#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TMainWindow w; // Экземпляр основного окна

    // app.setAttribute(Qt::AA_UseDesktopOpenGL);
    w.show();
    return app.exec();
}

