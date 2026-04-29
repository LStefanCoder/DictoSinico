#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    a.setApplicationName("CC-CEDICT GUI");

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
