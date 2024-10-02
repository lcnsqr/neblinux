#include "FormHeat.h"
#include "ui_FormHeat.h"

FormHeat::FormHeat(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormHeat),
    dev(d)
{
    ui->setupUi(this);

    ui->pidEnabled->setProperty("changedAt", QDateTime::currentDateTime().toString());
    connect(ui->pidEnabled, &QCheckBox::stateChanged, this, &FormHeat::pidEnabledChange);

    ui->heatLoad->setProperty("changed", false);
    connect(ui->heatLoad, &QSpinBox::valueChanged, this, &FormHeat::heatLoadChange);
    connect(ui->heatLoad, &QSpinBox::editingFinished, this, &FormHeat::heatLoadSave);

}

FormHeat::~FormHeat()
{
    delete ui;
}

QCheckBox *FormHeat::getPidEnabled()
{
    return ui->pidEnabled;
}

QSpinBox *FormHeat::getHeatLoad()
{
    return ui->heatLoad;
}

void FormHeat::pidEnabledChange(int state)
{
    ui->pidEnabled->setProperty("changedAt", QDateTime::currentDateTime().toString());
    if (state == Qt::Checked) {
        QMetaObject::invokeMethod(dev, "enablePID", Qt::QueuedConnection, Q_ARG(int, 1));
    }
    else {
        QMetaObject::invokeMethod(dev, "enablePID", Qt::QueuedConnection, Q_ARG(int, 0));
    }
}

void FormHeat::heatLoadChange()
{
    ui->heatLoad->setProperty("changed", true);
}

void FormHeat::heatLoadSave()
{
    if ( ! ui->heatLoad->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, (float)(ui->heatLoad->value())));
}
