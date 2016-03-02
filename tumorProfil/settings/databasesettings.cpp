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
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QDebug>

#include "databaseparameters.h"
#include "constants.h"


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
    QLineEdit*     databaseNameUsers;
    QLineEdit*     hostName;
    QLineEdit*     connectionOptions;
    QLineEdit*     userName;
    QLineEdit*     password;

    QComboBox*     databaseType;
    QSpinBox*      hostPort;
};

DatabaseSettings::DatabaseSettings(QWidget* parent)
    : QWidget(parent), d(new Private())
{
    setupMainArea();
    loadSettings();
}

QString DatabaseSettings::currentDatabaseType()
{
    return d->databaseType->itemData(d->databaseType->currentIndex()).toString();
}

void DatabaseSettings::loadSettings()
{
    DatabaseParameters dp;
    dp.readFromConfig();

    if(dp.isMySQL())
        d->databaseType->setCurrentIndex(1);

    d->databaseName->setText(dp.databaseName);
    d->databaseNameUsers->setText(dp.databaseNameThumbnails);
    d->hostName->setText(dp.hostName);
    d->connectionOptions->setText(dp.connectOptions);
    d->userName->setText(dp.userName);
    d->password->setText(dp.password);
    d->sqlitePath->setText(dp.sqliteDatabasePath);
    d->hostPort->setValue(dp.port);
}

void DatabaseSettings::applySettings()
{
    DatabaseParameters dp = getDatabaseParameters();
    dp.writeToConfig();
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


    QLabel* const databaseTypeLabel     = new QLabel(tr("Type"));
    d->databaseType                     = new QComboBox();

    QLabel* const databaseNameLabel     = new QLabel(tr("Schema Name"));
    d->databaseName                     = new QLineEdit();
    QLabel* const databaseNameUsersLabel= new QLabel(tr("Username<br>Schema Name"));
    d->databaseNameUsers                = new QLineEdit();
    QLabel* const hostNameLabel         = new QLabel(tr("Host Name"));
    d->hostName                         = new QLineEdit();
    QLabel* const hostPortLabel         = new QLabel(tr("Port"));
    d->hostPort                         = new QSpinBox();
    d->hostPort->setMaximum(65535);

    QLabel* const connectionOptionsLabel= new QLabel(tr("Database<br>Connection<br>Options"));
    d->connectionOptions                = new QLineEdit();

    QLabel* const userNameLabel         = new QLabel(tr("User"));
    d->userName                         = new QLineEdit();

    QLabel* const passwordLabel         = new QLabel(tr("Password"));
    d->password                         = new QLineEdit();
    d->password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(tr("Check DB Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);


    expertSettinglayout->addRow(hostNameLabel, d->hostName);
    expertSettinglayout->addRow(hostPortLabel, d->hostPort);
    expertSettinglayout->addRow(databaseNameLabel, d->databaseName);
    expertSettinglayout->addRow(databaseNameUsersLabel, d->databaseNameUsers);
    expertSettinglayout->addRow(userNameLabel, d->userName);
    expertSettinglayout->addRow(passwordLabel, d->password);
    expertSettinglayout->addRow(connectionOptionsLabel, d->connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(d->databaseType);
    vlay->addWidget(d->databasePathLabel);
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

    d->databaseType->addItem(tr("SQLite"), DatabaseParameters::SQLiteDatabaseType());
    d->databaseType->addItem(tr("MYSQL"), DatabaseParameters::MySQLDatabaseType());


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
            this, SLOT(slotCheckDatabaseConnection()));

    setLayout(layout);
}


void DatabaseSettings::setDatabaseInputFields(const QString &currentIndexStr)
{
    if (currentIndexStr == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        d->databasePathLabel->setVisible(true);
        d->sqlitePath->setVisible(true);
        d->sqliteBrowse->setVisible(true);
        d->expertSettings->setVisible(false);

        connect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
//        connect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
//                this, SLOT(slotDatabasePathEditedDelayed()));
    }
    else
    {
        d->databasePathLabel->setVisible(false);
        d->sqlitePath->setVisible(false);
        d->sqliteBrowse->setVisible(false);
        d->expertSettings->setVisible(true);

        disconnect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
//      disconnect(databasePathEdit->lineEdit(), SIGNAL(textChanged(QString)),
//                   this, SLOT(slotDatabasePathEditedDelayed()));
    }

}

DatabaseParameters DatabaseSettings::getDatabaseParameters()
{
    DatabaseParameters parameters;

    parameters.connectOptions = d->connectionOptions->text();
    parameters.databaseType   = currentDatabaseType();
    parameters.hostName       = d->hostName->text();
    parameters.password       = d->password->text();
    parameters.port           = d->hostPort->text().toInt();
    parameters.userName       = d->userName->text();

    if (parameters.databaseType == QString(DatabaseParameters::SQLiteDatabaseType()))
    {
        // TODO: fix this
        parameters.databaseName = QDir::cleanPath(d->sqlitePath->text() + QLatin1Char('/') + QLatin1String("digikam4.db"));
        parameters.databaseNameThumbnails = parameters.databaseName;
    }
    else
    {
        parameters.databaseName = d->databaseName->text();
        parameters.databaseNameThumbnails = d->databaseNameUsers->text();
    }


    return parameters;
}

void DatabaseSettings::slotSetDatabasePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    d->sqlitePath->setText(dir);
}

void DatabaseSettings::slotCheckDatabaseConnection()
{
    // TODO : if check DB connection operations can be threaded, use DBusyDlg dialog there...

    qApp->setOverrideCursor(Qt::WaitCursor);
    QString databaseID(QLatin1String("TumorDbConnectionTest"));

    {

        QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(currentDatabaseType(), databaseID);
        DatabaseParameters parameters = getDatabaseParameters();

        if(!testDatabase.isValid())
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testDatabase.lastError().text().toLatin1() +  "</p>") );

            return;
        }

        qDebug() << parameters;
        if(!parameters.isValid())
        {
            qApp->restoreOverrideCursor();
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                      QString("Database parameters are invalid").toLatin1() +  "</p>") );

            return;
        }
        testDatabase.setHostName(parameters.hostName);
        testDatabase.setPort(parameters.port);
        testDatabase.setUserName(parameters.userName);
        testDatabase.setPassword(parameters.password);
        testDatabase.setConnectOptions(parameters.connectOptions);

        qApp->restoreOverrideCursor();

        testDatabase.setDatabaseName(parameters.databaseName);

        bool result = testDatabase.open();

        if (!result)
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testDatabase.lastError().text().toLatin1() +  "</p>") );
        }

        QSqlQuery* testQuery = new QSqlQuery(testDatabase);
        testQuery->prepare(QLatin1String("show tables"));

        result = testQuery->exec();

        if (result)
        {
            QMessageBox::information(qApp->activeWindow(), tr("Database connection test"),
                                     tr("Database connection test successful."));
        }
        else
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testQuery->lastError().text().toLatin1() +  "</p>") );
        }

        testDatabase.close();
    }
    QSqlDatabase::removeDatabase(databaseID);

}
