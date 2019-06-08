#-------------------------------------------------
#
# Project created by QtCreator 2019-05-11T20:45:37
#
#-------------------------------------------------

QT += core gui
QT += network
QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VNC_Server
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qvncserver.cpp \
    qvnc_connection.cpp \
    vnc_map.cpp \
    qvnc_screen.cpp

HEADERS += \
        mainwindow.h \
    qvncserver.h \
    qvnc_connection.h \
    vnc_map.h \
    qvnc_screen.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
