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
    mymessagebox.cpp \
    masterkeystable.cpp \
    addkeywidget.cpp

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
    mymessagebox.h \
    masterkeystable.h \
    addkeywidget.h

FORMS    +=

RESOURCES += \
    storage.qrc

unix{
    LIBS += -lcryptopp
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cryptopp563/release/ -lcryptopp563
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../cryptopp563/debug/ -lcryptopp563

INCLUDEPATH += $$PWD/../cryptopp563
DEPENDPATH += $$PWD/../cryptopp563
