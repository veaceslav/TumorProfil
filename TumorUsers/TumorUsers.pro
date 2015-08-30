#-------------------------------------------------
#
# Project created by QtCreator 2015-08-30T11:41:25
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TumorUsers
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    databaseconfigelement.cpp

HEADERS  += mainwindow.h \
    databaseconfigelement.h

FORMS    +=

RESOURCES += \
    storage.qrc
