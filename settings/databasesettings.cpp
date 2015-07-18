#include "databasesettings.h"

#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QApplication>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>

#include "databaseparameters.h"


class DatabaseSettings::Private
{
public:
    Private()
    {

    }
    QLineEdit*      sqlitePath;
    QPushButton*    sqliteBrowse;

    QGroupBox*      expertSettings;
    QLabel*         databasePathLabel;

    QString        originalDbPath;
    QString        originalDbType;

    QLineEdit*     databaseName;
    QLineEdit*     databaseNameThumbnails;
    QLineEdit*     hostName;
    QLineEdit*     connectionOptions;
    QLineEdit*     userName;
    QLineEdit*     password;

    QComboBox*     databaseType;
    QSpinBox*      hostPort;
    QCheckBox*     internalServer;

//    RFileSelector* databasePathEdit;
};

DatabaseSettings::DatabaseSettings(QWidget* parent)
    : QWidget(parent), d(new Private())
{
    setupMainArea();
}

void DatabaseSettings::slotHandleDBTypeIndexChanged(int index)
{
    const QString& dbType = d->databaseType->itemData(index).toString();
    setDatabaseInputFields(dbType);
}

void DatabaseSettings::setupMainArea()
{
    QVBoxLayout* const layout  = new QVBoxLayout();


    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(tr("Database File Path"), this);
    QVBoxLayout* const vlay    = new QVBoxLayout(dbPathBox);
    d->databasePathLabel       = new QLabel(tr("<p>The location where the database file will be stored on your system. "
                                                 "There is one common database file for all root albums.<br/>"
                                                 "Write access is required to be able to edit image properties.</p>"
                                                 "<p>Note: a remote file system, such as NFS, cannot be used here.</p><p></p>"),
                                            dbPathBox);
    d->databasePathLabel->setWordWrap(true);
    d->sqlitePath = new QLineEdit(this);
    d->sqliteBrowse = new QPushButton(tr("Browse"),this);
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(d->sqlitePath,8);
    pathLayout->addWidget(d->sqliteBrowse,2);


    QLabel* const databaseTypeLabel                  = new QLabel(tr("Type"));
    d->databaseType                                     = new QComboBox();

    QLabel* const databaseNameLabel                  = new QLabel(tr("Schema Name"));
    d->databaseName                                     = new QLineEdit();
    QLabel* const databaseNameThumbnailsLabel        = new QLabel(tr("Username<br>Schema Name"));
    d->databaseNameThumbnails                           = new QLineEdit();
    QLabel* const hostNameLabel                      = new QLabel(tr("Host Name"));
    d->hostName                                         = new QLineEdit();
    QLabel* const hostPortLabel                      = new QLabel(tr("Port"));
    d->hostPort                                         = new QSpinBox();
    d->hostPort->setMaximum(65535);

    QLabel* const connectionOptionsLabel             = new QLabel(tr("Database<br>Connection<br>Options"));
    d->connectionOptions                                = new QLineEdit();

    QLabel* const userNameLabel                      = new QLabel(tr("User"));
    d->userName                                         = new QLineEdit();

    QLabel* const passwordLabel                      = new QLabel(tr("Password"));
    d->password                                         = new QLineEdit();
    d->password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(tr("Check DB Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);


    expertSettinglayout->addRow(hostNameLabel, d->hostName);
    expertSettinglayout->addRow(hostPortLabel, d->hostPort);
    expertSettinglayout->addRow(databaseNameLabel, d->databaseName);
    expertSettinglayout->addRow(databaseNameThumbnailsLabel, d->databaseNameThumbnails);
    expertSettinglayout->addRow(userNameLabel, d->userName);
    expertSettinglayout->addRow(passwordLabel, d->password);
    expertSettinglayout->addRow(connectionOptionsLabel, d->connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(d->databaseType);
    vlay->addWidget(d->databasePathLabel);
//    vlay->addWidget(d->databasePathEdit);
    vlay->addLayout(pathLayout);
    vlay->addWidget(d->expertSettings);
    vlay->setSpacing(0);
    vlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    dbPathBox->setLayout(vlay);

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    d->databaseType->addItem(tr("SQLite"),               DatabaseParameters::SQLiteDatabaseType());
    d->databaseType->addItem(tr("MYSQL"),               DatabaseParameters::MySQLDatabaseType());


    d->databaseType->setToolTip(tr("<p>Select here the type of database backend.</p>"
                                  "<p><b>SQlite</b> backend is for local database storage with a small and medium collection sizes. "
                                  "It is the default and recommended backend.</p>"
                                  "<p><b>MySQL</b> backend is a more robust solution especially for remote and shared database storage. "
                                  "It is also more efficient to manage huge collection sizes. "
                                  "Be careful: this one it is still in experimental stage.</p>"
                                 ));

    setDatabaseInputFields(DatabaseParameters::SQLiteDatabaseType());

    // --------------------------------------------------------

    connect(d->databaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(checkDatabaseConnection()));

    setLayout(layout);
}

void DatabaseSettings::setupSQLiteOptions()
{
//    whcFile = QFileDialog::getOpenFileName(this,
//                                           tr("Open WHC project"),
//                                           "",
//                                           tr("WHC Project File (*.whc)"));
}

void DatabaseSettings::setupMySQLOPtions()
{

}

void DatabaseSettings::setDatabaseInputFields(const QString &currentIndexStr)
{
    if (currentIndexStr == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        d->databasePathLabel->setVisible(true);
        d->sqlitePath->setVisible(true);
        d->sqliteBrowse->setVisible(true);
//        d->databasePathEdit->setVisible(true);
        d->expertSettings->setVisible(false);

        connect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
//        connect(databasePathEdit->fileDialog(), SIGNAL(urlSelected(QUrl)),
//                this, SLOT(slotChangeDatabasePath(QUrl)));

//        connect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
//                this, SLOT(slotDatabasePathEditedDelayed()));
    }
    else
    {
        d->databasePathLabel->setVisible(false);
//        databasePathEdit->setVisible(false);
        d->sqlitePath->setVisible(false);
        d->sqliteBrowse->setVisible(false);
        d->expertSettings->setVisible(true);

        disconnect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
//        disconnect(databasePathEdit->fileDialog(), SIGNAL(urlSelected(QUrl)),
//                   this, SLOT(slotChangeDatabasePath(QUrl)));

//        disconnect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
//                   this, SLOT(slotDatabasePathEditedDelayed()));
    }

}

void DatabaseSettings::slotSetDatabasePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    d->sqlitePath->setText(dir);
}

