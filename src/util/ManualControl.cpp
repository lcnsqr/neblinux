#include "ManualControl.h"
#include "ui_ManualControl.h"

ManualControl::ManualControl(QWidget *parent, devNano *d)
    : QWidget(parent)
    , ui(new Ui::ManualControl)
    , dev(d)
{
    ui->setupUi(this);

    ui->fan->setProperty("changed", false);
    connect(ui->fan, &QCheckBox::stateChanged, this, &ManualControl::fanChange);

    ui->PID->setProperty("changed", false);
    connect(ui->PID, &QCheckBox::stateChanged, this, &ManualControl::PIDChange);

    ui->manualLoad->setProperty("changed", false);
    connect(ui->manualLoad, &QSpinBox::valueChanged, this, &ManualControl::loadChange);


    // Buttons
    connect(ui->manualRestore, &QPushButton::clicked, this, &ManualControl::restore);
    connect(ui->manualApply, &QPushButton::clicked, this, &ManualControl::apply);
}

ManualControl::~ManualControl()
{
    delete ui;
}

void ManualControl::updateScreenData()
{
    // Enable/disable FormPrefs action buttons
    if ( ui->fan->property("changed").toBool()
        || ui->PID->property("changed").toBool()
        || ui->manualLoad->property("changed").toBool() )
    {
        ui->manualApply->setEnabled(true);
        ui->manualRestore->setEnabled(true);
    }
    else
    {
        ui->manualApply->setDisabled(true);
        ui->manualRestore->setDisabled(true);
    }
}

void ManualControl::devDataIn(const State &state)
{
    if ( ui->fan->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<bool>(state.on) == ui->fan->isChecked() )
            ui->fan->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->fan->setChecked(static_cast<bool>(state.on));
        ui->fan->setProperty("changed", false);
    }

    if ( ui->PID->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<bool>(state.PID_enabled) == ui->PID->isChecked() )
            ui->PID->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->PID->setChecked(static_cast<bool>(state.PID_enabled));
        ui->PID->setProperty("changed", false);
    }

    if ( ui->manualLoad->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.PID[4]) == static_cast<float>(ui->manualLoad->value()) )
            ui->manualLoad->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->manualLoad->setValue( static_cast<float>(state.PID[4]) );
        ui->manualLoad->setProperty("changed", false);
    }

}

void ManualControl::reset()
{
    ui->fan->setProperty("changed", false);
    ui->PID->setProperty("changed", false);
    ui->manualLoad->setProperty("changed", false);
}

void ManualControl::apply()
{
    if ( ui->fan->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setFan", Qt::QueuedConnection, Q_ARG(bool, static_cast<bool>(ui->fan->isChecked()) ));

    if ( ui->PID->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "enablePID", Qt::QueuedConnection, Q_ARG(int, static_cast<int>(ui->PID->isChecked()) ));

    if ( ui->manualLoad->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, static_cast<float>(ui->manualLoad->value()) ));
}

void ManualControl::restore()
{
    reset();
}

void ManualControl::fanChange()
{
    ui->fan->setProperty("changed", true);
}

void ManualControl::PIDChange()
{
    ui->PID->setProperty("changed", true);
}

void ManualControl::loadChange()
{
    ui->manualLoad->setProperty("changed", true);
}
