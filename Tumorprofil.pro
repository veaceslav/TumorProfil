#-------------------------------------------------
#
# Project created by QtCreator 2012-01-18T11:01:56
#
#-------------------------------------------------

QT       += core gui sql xml

TARGET = Tumorprofil
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    datamodel/patient.cpp \
    datamodel/disease.cpp \
    datamodel/pathology.cpp \
    datamodel/property.cpp \
    datamodel/tnm.cpp \
    ui/patientlistview.cpp \
    ui/patientdisplay.cpp \
    ui/patiententerform.cpp \
    ui/diseasetabwidget.cpp \
    ui/tnmwidget.cpp \
    ui/pathologypropertywidget.cpp \
    pathologywidgetgenerator.cpp \
    ui/entityselectionwidget.cpp \
    ui/smokerwidget.cpp \
    storage/patientmanager.cpp \
    storage/databasecorebackend.cpp \
    storage/databaseparameters.cpp \
    storage/sqlquery.cpp \
    storage/dbactiontype.cpp \
    storage/databaseconfigelement.cpp \
    storage/schemaupdater.cpp \
    storage/databasetransaction.cpp \
    storage/databaseoperationgroup.cpp \
    storage/databaseaccess.cpp \
    storage/patientdb.cpp \
    storage/patientmodel.cpp

HEADERS  += mainwindow.h \
    datamodel/patient.h \
    datamodel/disease.h \
    datamodel/pathology.h \
    datamodel/property.h \
    datamodel/tnm.h \
    ui/patientlistview.h \
    ui/patientdisplay.h \
    ui/patiententerform.h \
    ui/diseasetabwidget.h \
    ui/tnmwidget.h \
    ui/pathologypropertywidget.h \
    pathologywidgetgenerator.h \
    ui/entityselectionwidget.h \
    ui/smokerwidget.h \
    storage/patientmanager.h \
    storage/databasecorebackend.h \
    storage/databasecorebackend_p.h \
    storage/databaseerrorhandler.h \
    storage/databaseparameters.h \
    storage/sqlquery.h \
    storage/dbactiontype.h \
    storage/databaseconfigelement.h \
    storage/schemaupdater.h \
    storage/databasetransaction.h \
    storage/databaseoperationgroup.h \
    storage/databaseaccess.h \
    storage/patientdb.h \
    storage/databaseinitializationobserver.h \
    storage/patientmodel.h

INCLUDEPATH += datamodel/ \
    ui/ \
    storage/

OTHER_FILES += \
    gpl-header-template.txt \
    icons/silk/index.theme \
    storage/dbconfig.xml

RESOURCES += \
    icons/icontheme-silk.qrc \
    storage/dbconfig.qrc
