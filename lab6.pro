QT       += core gui sql widgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dataworker.cpp \
    dbmanager.cpp \
    goodsmodel.cpp \
    main.cpp \
    mainwindow.cpp \
    masterview.cpp \
    recordsdialog.cpp

HEADERS += \
    dataworker.h \
    dbmanager.h \
    goodsmodel.h \
    mainwindow.h \
    masterview.h \
    recordsdialog.h

FORMS += \
    mainwindow.ui \
    masterview.ui \
    recordsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
