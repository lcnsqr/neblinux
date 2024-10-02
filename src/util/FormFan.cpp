#include "FormFan.h"
#include "ui_FormFan.h"

FormFan::FormFan(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormFan),
    dev(d)
{
    ui->setupUi(this);

    ui->fanControl->setProperty("changedAt", QDateTime::currentDateTime().toString());
    connect(ui->fanControl, &QCheckBox::stateChanged, this, &FormFan::fanControlChange);

    ui->fanLoad->setProperty("changed", false);
    connect(ui->fanLoad, &QSpinBox::valueChanged, this, &FormFan::fanLoadChange);
    connect(ui->fanLoad, &QSpinBox::editingFinished, this, &FormFan::fanLoadSave);
}

FormFan::~FormFan()
{
    delete ui;
}

QCheckBox *FormFan::getFanControl()
{
    return ui->fanControl;
}

QSpinBox *FormFan::getFanLoad()
{
    return ui->fanLoad;
}

QLabel *FormFan::getElapsed()
{
    return ui->elapsed;
}

void FormFan::fanControlChange(int state)
{
    ui->fanControl->setProperty("changedAt", QDateTime::currentDateTime().toString());
    if (state == Qt::Checked) {
        QMetaObject::invokeMethod(dev, "setFan", Qt::QueuedConnection, Q_ARG(bool, true));
    }
    else {
        QMetaObject::invokeMethod(dev, "setFan", Qt::QueuedConnection, Q_ARG(bool, false));
    }
}

void FormFan::fanLoadChange()
{
    ui->fanLoad->setProperty("changed", true);
}

void FormFan::fanLoadSave()
{
    if ( ! ui->fanLoad->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setFanLoad", Qt::QueuedConnection, Q_ARG(float, (float)(ui->fanLoad->value())));

}
