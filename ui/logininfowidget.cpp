#include "logininfowidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include "encryption/userinformation.h"


QPointer<LoginInfoWidget> LoginInfoWidget::internalPtr = QPointer<LoginInfoWidget>();

class LoginInfoWidget::Private
{
public:
    Private()
    {

    }

    QLabel* displayInfo;
    QPushButton* bottomInfo;
};
LoginInfoWidget::LoginInfoWidget()
    :d(new Private())
{

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    d->displayInfo = new QLabel(this);
    d->bottomInfo = new QPushButton(this);
    mainLayout->addWidget(d->displayInfo);
    mainLayout->addWidget(d->bottomInfo);

    logOutUpdate();

    connect(d->bottomInfo, SIGNAL(clicked(bool)),
            UserInformation::instance(), SLOT(toggleLogIn()));
}

LoginInfoWidget* LoginInfoWidget::instance()
{
    if(LoginInfoWidget::internalPtr.isNull())
        LoginInfoWidget::internalPtr = new LoginInfoWidget();

    return LoginInfoWidget::internalPtr;
}

void LoginInfoWidget::logInUpdate(QString userName)
{
    d->displayInfo->setText("Logged in as:" + userName);
    d->bottomInfo->setText("Log out");
}

void LoginInfoWidget::logOutUpdate()
{
    d->displayInfo->setText("Not logged in");
    d->bottomInfo->setText(tr("Log in"));
}
