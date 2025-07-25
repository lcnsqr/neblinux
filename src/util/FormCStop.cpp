#include "FormCStop.h"
#include "ui_FormCStop.h"

FormCStop::FormCStop(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormCStop),
    dev(d)
{
    ui->setupUi(this);

    ui->cStop0->setProperty("changed", false);
    connect(ui->cStop0, &QDoubleSpinBox::valueChanged, this, &FormCStop::cStop0Change);
    ui->cStop1->setProperty("changed", false);
    connect(ui->cStop1, &QDoubleSpinBox::valueChanged, this, &FormCStop::cStop1Change);

    // Buttons
    connect(ui->cStopRestore, &QPushButton::clicked, this, &FormCStop::restore);
    connect(ui->cStopApply, &QPushButton::clicked, this, &FormCStop::apply);

}

FormCStop::~FormCStop()
{
    delete ui;
}

void FormCStop::updateScreenData()
{
    // Enable/disable CStop action buttons
    if ( ui->cStop0->property("changed").toBool()
        || ui->cStop1->property("changed").toBool() )
    {
        ui->cStopRestore->setEnabled(true);
        ui->cStopApply->setEnabled(true);
    }
    else
    {
        ui->cStopRestore->setDisabled(true);
        ui->cStopApply->setDisabled(true);
    }
}

void FormCStop::devDataIn(const State &state)
{
    // auto stop coefficients
    if ( ui->cStop0->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.cStop[0]) == static_cast<float>(ui->cStop0->value()) )
            ui->cStop0->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->cStop0->setValue( static_cast<float>(state.cStop[0]) );
        ui->cStop0->setProperty("changed", false);
    }
    if ( ui->cStop1->property("changed").toBool() ){
        if ( static_cast<float>(state.cStop[1]) == static_cast<float>(ui->cStop1->value()) )
            ui->cStop1->setProperty("changed", false);
    }
    else {
        ui->cStop1->setValue( static_cast<float>(state.cStop[1]) );
        ui->cStop1->setProperty("changed", false);
    }
}

void FormCStop::reset()
{
    ui->cStop0->setProperty("changed", false);
    ui->cStop1->setProperty("changed", false);
}

void FormCStop::apply()
{
    if ( ui->cStop0->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setCStop0", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cStop0->value())));

    if ( ui->cStop1->property("changed").toBool() )
        QMetaObject::invokeMethod(dev, "setCStop1", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cStop1->value())));

}

void FormCStop::restore()
{
    ui->cStop0->setProperty("changed", false);
    ui->cStop1->setProperty("changed", false);
}


void FormCStop::cStop0Change()
{
    ui->cStop0->setProperty("changed", true);
}

void FormCStop::cStop1Change()
{
    ui->cStop1->setProperty("changed", true);
}
