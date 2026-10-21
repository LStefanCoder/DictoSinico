QT += widgets
QT += concurrent #for the loading window
QT += widgets svg

CONFIG += c++17

SOURCES += \
    loadingdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    preferencesdialog.cpp \
    svgwidget.cpp

HEADERS += \
    loadingdialog.h \
    mainwindow.h \
    preferencesdialog.h \
    svgwidget.h

FORMS += \
    mainwindow.ui \
    preferencesdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    cedict_ts.txt
    hanziDB_twocolumns.csv

RESOURCES += \
    resources.qrc
