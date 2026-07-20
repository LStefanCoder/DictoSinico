#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    a.setApplicationName("CC-CEDICT GUI");

    //template code based on https://qss-stock.devsecstudio.com/documentation.php
    QFile file("C:/Users/User/Documents/Projects/2026/CeDict/themes/Geoo.qss");
    file.open(QFile::ReadOnly);

    QString styleSheet { QLatin1String(file.readAll()) };

    //setup stylesheet
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();

    //loading the dictionary at the start of the program
    w.loadDictionaryFile("C:/Users/User/Documents/Projects/2026/CeDict/cedict_ts.txt");

    return QCoreApplication::exec();
}
