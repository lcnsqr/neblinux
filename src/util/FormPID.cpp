#include "FormPID.h"
#include "ui_FormPID.h"

#include <iostream>

FormPID::FormPID(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormPID),
    dev(d)
{
    ui->setupUi(this);

    ui->target->setProperty("changed", false);
    connect(ui->target, &QSpinBox::valueChanged, this, &FormPID::targetChange);
    connect(ui->target, &QSpinBox::editingFinished, this, &FormPID::targetSave);

    ui->cPID0->setProperty("changed", false);
    connect(ui->cPID0, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID0Change);
    connect(ui->cPID0, &QDoubleSpinBox::editingFinished, this, &FormPID::cPID0Save);
    ui->cPID1->setProperty("changed", false);
    connect(ui->cPID1, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID1Change);
    connect(ui->cPID1, &QDoubleSpinBox::editingFinished, this, &FormPID::cPID1Save);
    ui->cPID2->setProperty("changed", false);
    connect(ui->cPID2, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID2Change);
    connect(ui->cPID2, &QDoubleSpinBox::editingFinished, this, &FormPID::cPID2Save);

}

FormPID::~FormPID()
{
    delete ui;
}

QSpinBox *FormPID::getTarget()
{
    return ui->target;
}

QDoubleSpinBox *FormPID::getCPID(int i)
{

    QDoubleSpinBox *cPID = nullptr;
    if ( i == 0 )
        cPID = ui->cPID0;
    else if ( i == 1 )
        cPID = ui->cPID1;
    else if ( i == 2 )
        cPID = ui->cPID2;
    return cPID;

}

void FormPID::targetChange()
{
    ui->target->setProperty("changed", true);
}

void FormPID::targetSave()
{
    if ( ! ui->target->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setTempTarget", Qt::QueuedConnection, Q_ARG(float, (float)(ui->target->value())));
}

void FormPID::cPID0Change()
{
    ui->cPID0->setProperty("changed", true);
}

void FormPID::cPID0Save()
{
    if ( ! ui->cPID0->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCPID0", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID0->value())));
}

void FormPID::cPID1Change()
{
    ui->cPID1->setProperty("changed", true);
}

void FormPID::cPID1Save()
{
    if ( ! ui->cPID1->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCPID1", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID1->value())));
}

void FormPID::cPID2Change()
{
    ui->cPID2->setProperty("changed", true);
}

void FormPID::cPID2Save()
{
    if ( ! ui->cPID2->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCPID2", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID2->value())));
}
