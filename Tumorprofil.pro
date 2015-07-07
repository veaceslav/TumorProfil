#-------------------------------------------------
#
# Project created by QtCreator 2012-01-18T11:01:56
#
#-------------------------------------------------

QT       += core gui sql xml widgets

TARGET = Tumorprofil
TEMPLATE = app

CONFIG += c++11

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
    storage/patientmodel.cpp \
    storage/patientpropertyfiltermodel.cpp \
    storage/patientpropertymodel.cpp \
    datamodel/pathologypropertyinfo.cpp \
    ui/reportwindow.cpp \
    ui/reporttableview.cpp \
    util/csvfile.cpp \
    util/csvconverter.cpp \
    medical/modeldatagenerator.cpp \
    medical/resultcompletenesschecker.cpp \
    ui/pathologymetadatawidget.cpp \
    medical/actionableresultchecker.cpp \
    medical/dataaggregator.cpp \
    storage/dataaggregationmodel.cpp \
    ui/analysistableview.cpp \
    ui/aggregatetableview.cpp \
    storage/dataaggregationfiltermodel.cpp \
    medical/confidenceinterval.cpp \
    medical/ihcscore.cpp \
    ui/columnselectiondialog.cpp \
    medical/combinedvalue.cpp \
    datamodel/historyelements.cpp \
    datamodel/diseasehistory.cpp \
    storage/databaseconstants.cpp \
    ui/filtermainwindow.cpp \
    ui/patientpropertymodelviewadapter.cpp \
    storage/diseasehistorymodel.cpp \
    ui/history/historyelementeditwidget.cpp \
    ui/history/therapyelementeditwidget.cpp \
    ui/history/historywindow.cpp \
    ui/history/historypatientlistview.cpp \
    ui/history/datevalidator.cpp \
    medical/history/historyiterator.cpp \
    ui/history/visualhistorywidget.cpp \
    util/analysisgenerator.cpp \
    util/historyvalidator.cpp \
    settings/mainsettings.cpp \
    menubar.cpp

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
    storage/patientmodel.h \
    storage/patientpropertyfiltermodel.h \
    storage/patientpropertymodel.h \
    datamodel/pathologypropertyinfo.h \
    ui/reportwindow.h \
    ui/reporttableview.h \
    util/csvfile.h \
    util/csvconverter.h \
    medical/modeldatagenerator.h \
    medical/resultcompletenesschecker.h \
    ui/pathologymetadatawidget.h \
    medical/actionableresultchecker.h \
    medical/dataaggregator.h \
    storage/dataaggregationmodel.h \
    ui/analysistableview.h \
    ui/aggregatetableview.h \
    storage/dataaggregationfiltermodel.h \
    medical/confidenceinterval.h \
    medical/ihcscore.h \
    ui/columnselectiondialog.h \
    medical/combinedvalue.h \
    datamodel/historyelements.h \
    datamodel/diseasehistory.h \
    util/xmltextintmapper.h \
    util/xmlstreamutils.h \
    storage/databaseconstants.h \
    ui/filtermainwindow.h \
    ui/patientpropertymodelviewadapter.h \
    storage/diseasehistorymodel.h \
    ui/history/historyelementeditwidget.h \
    ui/history/therapyelementeditwidget.h \
    ui/history/historywindow.h \
    ui/history/historypatientlistview.h \
    ui/history/datevalidator.h \
    medical/history/historyiterator.h \
    ui/history/visualhistorywidget.h \
    util/analysisgenerator.h \
    util/historyvalidator.h \
    settings/mainsettings.h \
    menubar.h

INCLUDEPATH += datamodel/ \
    ui/ \
    ui/history \
    storage/ \
    util/ \
    medical/ \
    C:/Users/wiesweg/Software/boost_1_50_0/

OTHER_FILES += \
    gpl-header-template.txt \
    icons/silk/index.theme \
    storage/dbconfig.xml

RESOURCES += \
    icons/icontheme-silk.qrc \
    storage/dbconfig.qrc
