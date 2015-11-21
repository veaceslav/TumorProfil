#-------------------------------------------------
#
# Project created by QtCreator 2015-08-30T11:41:25
#
#-------------------------------------------------

QT       += core gui xml sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TumorUsers
TEMPLATE = app

INCLUDEPATH += /usr/include/cryptopp

LIBS += -lcryptopp

SOURCES += main.cpp\
        mainwindow.cpp \
    databaseconfigelement.cpp \
    userwidget.cpp \
    databaseguioptions.cpp \
    schemaupdater.cpp \
    databaseaccess.cpp \
    databaseparameters.cpp \
    queryutils.cpp \
    aesutils.cpp \
    useradddialog.cpp \
    adminuser.cpp \
    mymessagebox.cpp

HEADERS  += mainwindow.h \
    databaseconfigelement.h \
    userwidget.h \
    databaseguioptions.h \
    schemaupdater.h \
    databaseaccess.h \
    databaseparameters.h \
    queryutils.h \
    aesutils.h \
    useradddialog.h \
    adminuser.h \
    mymessagebox.h

FORMS    +=

RESOURCES += \
    storage.qrc
