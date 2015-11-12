#include "databaseguioptions.h"
#include <QLineEdit>
#include <QString>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include "databaseconfigelement.h"
#include "databaseparameters.h"
#include "databaseaccess.h"
#include "schemaupdater.h"
#include "adminuser.h"
#include "userwidget.h"


class DatabaseGuiOptions::Private
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

    QLineEdit*     adminPassword;
    QLineEdit*     databaseNameUsers;
    QLineEdit*     hostName;
    QLineEdit*     connectionOptions;
    QLineEdit*     userName;
    QLineEdit*     password;

    QComboBox*     databaseType;
    QSpinBox*      hostPort;
    DatabaseParameters conf;
};
DatabaseGuiOptions::DatabaseGuiOptions( QWidget *parent) :
    QWidget(parent), d(new Private())
{
    setupUi();
//    d->conf.defaultParameters(QLatin1String("QMYSQL"));
    setupParameters();
    d->databaseType->setCurrentIndex(1); // Make MqSql Default
    slotHandleDBTypeIndexChanged();
}

DatabaseGuiOptions::~DatabaseGuiOptions()
{
}

void DatabaseGuiOptions::slotHandleDBTypeIndexChanged()
{
    if (d->databaseType->currentData() == QLatin1String("QSQLITE"))
    {
        d->sqlitePath->setVisible(true);
        d->sqliteBrowse->setVisible(true);
        d->expertSettings->setVisible(false);

        connect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
    }
    else
    {
        d->sqlitePath->setVisible(false);
        d->sqliteBrowse->setVisible(false);
        d->expertSettings->setVisible(true);

        disconnect(d->sqliteBrowse, SIGNAL(clicked(bool)), this, SLOT(slotSetDatabasePath()));
    }
}

void DatabaseGuiOptions::slotSetDatabasePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    d->sqlitePath->setText(dir);
}

void DatabaseGuiOptions::slotCheckDatabaseConnection()
{

    DatabaseParameters params;

    params.databaseType = d->databaseType->currentData().toString();
    params.databaseName = d->databaseNameUsers->text();
    params.hostName = d->hostName->text();
    params.port     = d->hostPort->value();
    params.userName = d->userName->text();
    params.password = d->password->text();
    params.connectOptions = d->connectionOptions->text();

    if(!params.isValid())
    {
        QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                              tr("Database connection test was not successful. <p>Error was: Parameters are not valid </p>") );

        return;
    }

    bool connected = DatabaseAccess::instance()->openDb(params);

    if(connected)
    {
        SchemaUpdater schemaUpdate(DatabaseAccess::instance());

        bool result = schemaUpdate.update();
        if(result)
        {
            emit signalconnectedToDb();
        }
    }
}


void DatabaseGuiOptions::setupUi()
{
    QVBoxLayout* const layout  = new QVBoxLayout();

    // --------------------------------------------------------

    QGroupBox* dbPathBox = new QGroupBox(tr("Database File Path"), this);
    QVBoxLayout* const vlay    = new QVBoxLayout(dbPathBox);
    d->sqlitePath = new QLineEdit(this);
    d->sqliteBrowse = new QPushButton(tr("Browse"),this);
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(d->sqlitePath,8);
    pathLayout->addWidget(d->sqliteBrowse,2);


    QLabel* const databaseTypeLabel     = new QLabel(tr("Type"));
    d->databaseType                     = new QComboBox();

    QLabel* const adminPasswordLabel     = new QLabel(tr("Admin Password"));
    d->adminPassword                     = new QLineEdit();
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
    expertSettinglayout->addRow(adminPasswordLabel, d->adminPassword);
    expertSettinglayout->addRow(databaseNameUsersLabel, d->databaseNameUsers);
    expertSettinglayout->addRow(userNameLabel, d->userName);
    expertSettinglayout->addRow(passwordLabel, d->password);
    expertSettinglayout->addRow(connectionOptionsLabel, d->connectionOptions);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    vlay->addWidget(databaseTypeLabel);
    vlay->addWidget(d->databaseType);
    vlay->addLayout(pathLayout);
    vlay->addWidget(d->expertSettings);
//    vlay->setSpacing(0);
//    vlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    dbPathBox->setLayout(vlay);

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->addWidget(dbPathBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    d->databaseType->addItem(tr("SQLite"), "QSQLITE");
    d->databaseType->addItem(tr("MYSQL"), "QMYSQL");


    d->databaseType->setToolTip(tr("<p>Select here the type of database backend.</p>"
                                  "<p><b>SQlite</b> backend is for local database storage with a small and medium collection sizes. "
                                  "It is the default and recommended backend.</p>"
                                  "<p><b>MySQL</b> backend is a more robust solution especially for remote and shared database storage. "
                                  "It is also more efficient to manage huge collection sizes. "
                                  "Be careful: this one it is still in experimental stage.</p>"
                                 ));


    // --------------------------------------------------------

    connect(d->databaseType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged()));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(slotCheckDatabaseConnection()));


    QWidget* wg = new QWidget(this);
    wg->setLayout(layout);

    this->setLayout(layout);
}

void DatabaseGuiOptions::setupParameters()
{
    d->conf = d->conf.defaultParameters(QLatin1String("QMYSQL"));

    d->databaseNameUsers->setText(d->conf.databaseName);
    d->hostName->setText(d->conf.hostName);
    d->hostPort->setValue(d->conf.port);
    d->userName->setText(d->conf.userName);
    d->password->setText(d->conf.password);
}

