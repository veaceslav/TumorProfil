#include "mainwindow.h"

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
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "databaseconfigelement.h"


class MainWindow::Private
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
    DatabaseConfigElement conf;
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), d(new Private())
{
    setupUi();
    d->conf = DatabaseConfigElement::element("QMYSQL");
    setupParameters();
    d->databaseType->setCurrentIndex(1); // Make MqSql Default
    slotHandleDBTypeIndexChanged();
}

MainWindow::~MainWindow()
{
}

void MainWindow::slotHandleDBTypeIndexChanged()
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

void MainWindow::slotSetDatabasePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    d->sqlitePath->setText(dir);
}

void MainWindow::slotCheckDatabaseConnection()
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    QString databaseID(QLatin1String("TumorDbConnectionTest"));

    {

        QSqlDatabase testDatabase     = QSqlDatabase::addDatabase(d->databaseType->currentData().toString(),
                                                                  databaseID);

        if(!testDatabase.isValid())
        {
            QMessageBox::critical(qApp->activeWindow(), tr("Database connection test"),
                                  tr("Database connection test was not successful. <p>Error was:" +
                                       testDatabase.lastError().text().toLatin1() +  "</p>") );

            return;
        }

        testDatabase.setHostName(d->hostName->text());
        testDatabase.setPort(d->hostPort->value());
        testDatabase.setUserName(d->userName->text());
        testDatabase.setPassword(d->password->text());
        testDatabase.setConnectOptions(d->connectionOptions->text());

        qApp->restoreOverrideCursor();

        testDatabase.setDatabaseName(d->databaseNameUsers->text());

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

void MainWindow::setupUi()
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

    this->setCentralWidget(wg);
}

void MainWindow::setupParameters()
{
    d->databaseNameUsers->setText(d->conf.databaseName);
    d->hostName->setText(d->conf.hostName);
    d->hostPort->setValue(d->conf.port.toInt());
    d->userName->setText(d->conf.userName);
    d->password->setText(d->conf.password);
}
