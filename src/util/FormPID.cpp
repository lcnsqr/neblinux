#include "FormPID.h"
#include "ui_FormPID.h"

#include <iostream>

FormPID::FormPID(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormPID),
    dev(d)
{
    ui->setupUi(this);

    ui->cPID0->setProperty("changed", false);
    connect(ui->cPID0, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID0Change);
    ui->cPID1->setProperty("changed", false);
    connect(ui->cPID1, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID1Change);
    ui->cPID2->setProperty("changed", false);
    connect(ui->cPID2, &QDoubleSpinBox::valueChanged, this, &FormPID::cPID2Change);

    // Buttons
    connect(ui->cPIDrestore, &QPushButton::clicked, this, &FormPID::restore);
    connect(ui->cPIDapply, &QPushButton::clicked, this, &FormPID::apply);

}

FormPID::~FormPID()
{
    delete ui;
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

QPushButton *FormPID::getcPIDrestore()
{
    return ui->cPIDrestore;
}

QPushButton *FormPID::getcPIDapply()
{
    return ui->cPIDapply;
}

void FormPID::apply()
{
    if ( ui->cPID0->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setCPID0", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID0->value())));

    if ( ui->cPID1->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setCPID1", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID1->value())));

    if ( ui->cPID2->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setCPID2", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cPID2->value())));
}

void FormPID::restore()
{
    ui->cPID0->setProperty("changed", false);
    ui->cPID1->setProperty("changed", false);
    ui->cPID2->setProperty("changed", false);
}

void FormPID::cPID0Change()
{
    ui->cPID0->setProperty("changed", true);
}

void FormPID::cPID1Change()
{
    ui->cPID1->setProperty("changed", true);
}

void FormPID::cPID2Change()
{
    ui->cPID2->setProperty("changed", true);
}
