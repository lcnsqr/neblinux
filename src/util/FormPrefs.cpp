#include "FormPrefs.h"
#include "ui_FormPrefs.h"

FormPrefs::FormPrefs(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormPrefs),
    dev(d)
{
    ui->setupUi(this);

    ui->autostop->setProperty("changedAt", QDateTime::currentDateTime().toString());
    connect(ui->autostop, &QCheckBox::stateChanged, this, &FormPrefs::autostopChange);

    ui->tempstep->setProperty("changed", false);
    connect(ui->tempstep, &QSpinBox::valueChanged, this, &FormPrefs::tempstepChange);
//    connect(ui->tempstep, &QSpinBox::editingFinished, this, &FormPrefs::tempstepSave);

    ui->screensaver->setProperty("changedAt", QDateTime::currentDateTime().toString());
    connect(ui->screensaver, &QCheckBox::stateChanged, this, &FormPrefs::screensaverChange);

}

FormPrefs::~FormPrefs()
{
    delete ui;
}

QCheckBox *FormPrefs::getAutostop()
{
    return ui->autostop;
}

QSpinBox *FormPrefs::getTempstep()
{
    return ui->tempstep;
}

QCheckBox *FormPrefs::getScreensaver()
{
    return ui->screensaver;
}

void FormPrefs::autostopChange(int state)
{
    ui->autostop->setProperty("changedAt", QDateTime::currentDateTime().toString());
    if (state == Qt::Checked) {
        QMetaObject::invokeMethod(dev, "autostop", Qt::QueuedConnection, Q_ARG(int, 1));
    }
    else {
        QMetaObject::invokeMethod(dev, "autostop", Qt::QueuedConnection, Q_ARG(int, 0));
    }
}

void FormPrefs::tempstepChange()
{
    ui->tempstep->setProperty("changed", true);
}

void FormPrefs::tempstepSave()
{
    if ( ! ui->tempstep->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "tempstep", Qt::QueuedConnection, Q_ARG(int, (int)(ui->tempstep->value())));
}

void FormPrefs::screensaverChange(int state)
{
    ui->screensaver->setProperty("changedAt", QDateTime::currentDateTime().toString());
    if (state == Qt::Checked) {
        QMetaObject::invokeMethod(dev, "screensaver", Qt::QueuedConnection, Q_ARG(int, 1));
    }
    else {
        QMetaObject::invokeMethod(dev, "screensaver", Qt::QueuedConnection, Q_ARG(int, 0));
    }
}
