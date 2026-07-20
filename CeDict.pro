QT += widgets
QT += concurrent #for the loading window

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    loadingdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    preferencesdialog.cpp

HEADERS += \
    loadingdialog.h \
    mainwindow.h \
    preferencesdialog.h

FORMS += \
    mainwindow.ui \
    preferencesdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
