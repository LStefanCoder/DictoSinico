#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QCoreApplication>
#include <QFileInfo>
#include <QString>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    a.setApplicationName("CC-CEDICT GUI");

    //template code based on https://qss-stock.devsecstudio.com/documentation.php
    QFile file("C:/Users/User/Documents/Projects/2026/CeDict/themes/Geoo.qss");
    file.open(QFile::ReadOnly);

    //commented stylesheet out
    //QString styleSheet { QLatin1String(file.readAll()) };

    //setup stylesheet
    //a.setStyleSheet(styleSheet);

    //loading a basic font for the program
    const int addedBasicFont = QFontDatabase::addApplicationFont(":/fonts/Arimo-Regular.ttf");
    const QStringList familiesBasic = QFontDatabase::applicationFontFamilies(addedBasicFont);
    const QString familyBasic = familiesBasic.first();
    QFont fontBasic(familyBasic);
    fontBasic.setPointSize(12);

    a.setFont(fontBasic);

    MainWindow w;
    w.show();

    //loading the dictionary at the start of the program
    w.loadDictionaryFile(":/cedict_ts.txt", ":/hanziDB_twocolumns.csv");
    //w.loadDictionaryFile("C:/Users/User/Documents/Projects/2026/CeDict/cedict_ts.txt");

    return QCoreApplication::exec();
}
