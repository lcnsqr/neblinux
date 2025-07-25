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

void FormPID::updateScreenData()
{
    // Enable/disable FormPID action buttons
    if ( ui->cPID0->property("changed").toBool()
        || ui->cPID1->property("changed").toBool()
        || ui->cPID2->property("changed").toBool() )
    {
        ui->cPIDrestore->setEnabled(true);
        ui->cPIDapply->setEnabled(true);
    }
    else
    {
        ui->cPIDrestore->setDisabled(true);
        ui->cPIDapply->setDisabled(true);
    }

}

void FormPID::devDataIn(const State &state)
{
    if ( ui->cPID0->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.cPID[0]) == static_cast<float>(ui->cPID0->value()) )
            ui->cPID0->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->cPID0->setValue( static_cast<float>(state.cPID[0]) );
        ui->cPID0->setProperty("changed", false);
    }

    if ( ui->cPID1->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.cPID[1]) == static_cast<float>(ui->cPID1->value()) )
            ui->cPID1->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->cPID1->setValue( static_cast<float>(state.cPID[1]) );
        ui->cPID1->setProperty("changed", false);
    }

    if ( ui->cPID2->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.cPID[2]) == static_cast<float>(ui->cPID2->value()) )
            ui->cPID2->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->cPID2->setValue( static_cast<float>(state.cPID[2]) );
        ui->cPID2->setProperty("changed", false);
    }
}

void FormPID::reset()
{
    ui->cPID0->setProperty("changed", false);
    ui->cPID1->setProperty("changed", false);
    ui->cPID2->setProperty("changed", false);
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
