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
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), d(new Private())
{
    setupUi();
    DatabaseConfigElement* conf = new DatabaseConfigElement();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
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

    d->databaseType->addItem(tr("SQLite"));//, DatabaseParameters::SQLiteDatabaseType());
    d->databaseType->addItem(tr("MYSQL"));//, DatabaseParameters::MySQLDatabaseType());


    d->databaseType->setToolTip(tr("<p>Select here the type of database backend.</p>"
                                  "<p><b>SQlite</b> backend is for local database storage with a small and medium collection sizes. "
                                  "It is the default and recommended backend.</p>"
                                  "<p><b>MySQL</b> backend is a more robust solution especially for remote and shared database storage. "
                                  "It is also more efficient to manage huge collection sizes. "
                                  "Be careful: this one it is still in experimental stage.</p>"
                                 ));


    // --------------------------------------------------------

//    connect(d->databaseType, SIGNAL(currentIndexChanged(int)),
//            this, SLOT(slotHandleDBTypeIndexChanged(int)));

//    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
//            this, SLOT(slotCheckDatabaseConnection()));

    QWidget* wg = new QWidget(this);
    wg->setLayout(layout);

    this->setCentralWidget(wg);
}

void MainWindow::setupParameters()
{

}
