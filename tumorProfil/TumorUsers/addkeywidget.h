#ifndef ADDKEYWIDGET_H
#define ADDKEYWIDGET_H


#include <QDialog>

class KeyInfo
{
public:
    KeyInfo()
    {

    }

    KeyInfo(QString name)
    {
        this->name = name;
    }
    KeyInfo(const KeyInfo& copy)
    {
        this->name = copy.name;
    }

    // Add extra fields if necessary
    QString name;
};

class AddKeyWidget : public QDialog
{
public:
    AddKeyWidget();

    static KeyInfo addKey();

    QString currentItem();
private:

    void setupUi();
    void setHardcodedNames();
    class Private;
    Private* d;
};

#endif // ADDKEYWIDGET_H
