#include "FormPrefs.h"
#include "ui_FormPrefs.h"

FormPrefs::FormPrefs(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormPrefs),
    dev(d)
{
    ui->setupUi(this);

    ui->autostop->setProperty("changed", false);
    connect(ui->autostop, &QCheckBox::stateChanged, this, &FormPrefs::autostopChange);

    ui->tempstep->setProperty("changed", false);
    connect(ui->tempstep, &QSpinBox::valueChanged, this, &FormPrefs::tempstepChange);

    ui->target->setProperty("changed", false);
    connect(ui->target, &QSpinBox::valueChanged, this, &FormPrefs::targetChange);

    ui->screensaver->setProperty("changed", false);
    connect(ui->screensaver, &QCheckBox::stateChanged, this, &FormPrefs::screensaverChange);

    // Buttons
    connect(ui->prefsRestore, &QPushButton::clicked, this, &FormPrefs::restore);
    connect(ui->prefsApply, &QPushButton::clicked, this, &FormPrefs::apply);
}

FormPrefs::~FormPrefs()
{
    delete ui;
}


void FormPrefs::updateScreenData()
{
    // Enable/disable FormPrefs action buttons
    if ( ui->screensaver->property("changed").toBool()
        || ui->tempstep->property("changed").toBool()
        || ui->target->property("changed").toBool()
        || ui->autostop->property("changed").toBool() )
    {
        ui->prefsApply->setEnabled(true);
        ui->prefsRestore->setEnabled(true);
    }
    else
    {
        ui->prefsApply->setDisabled(true);
        ui->prefsRestore->setDisabled(true);
    }

}

void FormPrefs::devDataIn(const State &state)
{
    if ( ui->target->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.tempTarget) == static_cast<float>(ui->target->value()) )
            ui->target->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->target->setValue( static_cast<float>(state.tempTarget) );
        ui->target->setProperty("changed", false);
    }

    if ( ui->tempstep->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<int>(state.tempStep) == static_cast<int>(ui->tempstep->value()) )
            ui->tempstep->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->tempstep->setValue( static_cast<int>(state.tempStep) );
        ui->tempstep->setProperty("changed", false);
    }

    if ( ui->screensaver->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<bool>(state.screensaver) == ui->screensaver->isChecked() )
            ui->screensaver->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->screensaver->setChecked(static_cast<bool>(state.screensaver));
        ui->screensaver->setProperty("changed", false);
    }

    if ( ui->autostop->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<bool>(state.autostop) == ui->autostop->isChecked() )
            ui->autostop->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->autostop->setChecked(static_cast<bool>(state.autostop));
        ui->autostop->setProperty("changed", false);
    }

}


void FormPrefs::apply()
{
    if ( ui->autostop->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "autostop", Qt::QueuedConnection, Q_ARG(int, static_cast<int>(ui->autostop->isChecked()) ));

    if ( ui->screensaver->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "screensaver", Qt::QueuedConnection, Q_ARG(int, static_cast<int>(ui->screensaver->isChecked()) ));

    if ( ui->tempstep->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "tempstep", Qt::QueuedConnection, Q_ARG(int, static_cast<int>(ui->tempstep->value()) ));

    if ( ui->target->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setTempTarget", Qt::QueuedConnection, Q_ARG(float, static_cast<float>(ui->target->value()) ));
}

void FormPrefs::reset()
{
    ui->tempstep->setProperty("changed", false);
    ui->target->setProperty("changed", false);
    ui->screensaver->setProperty("changed", false);
    ui->autostop->setProperty("changed", false);
}

void FormPrefs::restore()
{
    reset();
}

void FormPrefs::autostopChange()
{
    ui->autostop->setProperty("changed", true);
}

void FormPrefs::tempstepChange()
{
    ui->tempstep->setProperty("changed", true);
}

void FormPrefs::targetChange()
{
    ui->target->setProperty("changed", true);
}

void FormPrefs::screensaverChange()
{
    ui->screensaver->setProperty("changed", true);
}
