#include "FormCTemp.h"
#include "ui_FormCTemp.h"

FormCTemp::FormCTemp(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormCTemp),
    dev(d)
{
    ui->setupUi(this);

    ui->cTemp0->setProperty("changed", false);
    connect(ui->cTemp0, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp0Change);
    ui->cTemp1->setProperty("changed", false);
    connect(ui->cTemp1, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp1Change);
    ui->cTemp2->setProperty("changed", false);
    connect(ui->cTemp2, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp2Change);
    ui->cTemp3->setProperty("changed", false);
    connect(ui->cTemp3, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp3Change);

    // Buttons
    connect(ui->cTempRestore, &QPushButton::clicked, this, &FormCTemp::restore);
    connect(ui->cTempApply, &QPushButton::clicked, this, &FormCTemp::apply);
}

FormCTemp::~FormCTemp()
{
    delete ui;
}

void FormCTemp::updateScreenData()
{
    // Enable/disable cTemp action buttons
    if ( ui->cTemp0->property("changed").toBool()
        || ui->cTemp1->property("changed").toBool() 
        || ui->cTemp2->property("changed").toBool() 
        || ui->cTemp3->property("changed").toBool() )
    {
        ui->cTempRestore->setEnabled(true);
        ui->cTempApply->setEnabled(true);
    }
    else
    {
        ui->cTempRestore->setDisabled(true);
        ui->cTempApply->setDisabled(true);
    }
}

QList<float> FormCTemp::getCTempAll()
{
    QList<float> c;
    c.append(static_cast<float>(ui->cTemp0->value()));
    c.append(static_cast<float>(ui->cTemp1->value()));
    c.append(static_cast<float>(ui->cTemp2->value()));
    c.append(static_cast<float>(ui->cTemp3->value()));

    return c;
}

void FormCTemp::setCTempAll(const QList<float> &values)
{
    ui->cTemp0->setValue( values.at(0) );
    ui->cTemp1->setValue( values.at(1) );
    ui->cTemp2->setValue( values.at(2) );
    ui->cTemp3->setValue( values.at(3) );
}

void FormCTemp::devDataIn(const State &state)
{
    // Temperature profile coefficients
    if ( ui->cTemp0->property("changed").toBool() ){
        // Set unchanged if device data and field value do not differ
        if ( static_cast<float>(state.cTemp[0]) == static_cast<float>(ui->cTemp0->value()) )
            ui->cTemp0->setProperty("changed", false);
    }
    else {
        // No new value from user, update with device data
        ui->cTemp0->setValue( static_cast<float>(state.cTemp[0]) );
        ui->cTemp0->setProperty("changed", false);
    }

    if ( ui->cTemp1->property("changed").toBool() ){
        if ( static_cast<float>(state.cTemp[1]) == static_cast<float>(ui->cTemp1->value()) )
            ui->cTemp1->setProperty("changed", false);
    }
    else {
        ui->cTemp1->setValue( static_cast<float>(state.cTemp[1]) );
        ui->cTemp1->setProperty("changed", false);
    }

    if ( ui->cTemp2->property("changed").toBool() ){
        if ( static_cast<float>(state.cTemp[2]) == static_cast<float>(ui->cTemp2->value()) )
            ui->cTemp2->setProperty("changed", false);
    }
    else {
        ui->cTemp2->setValue( static_cast<float>(state.cTemp[2]) );
        ui->cTemp2->setProperty("changed", false);
    }

    if ( ui->cTemp3->property("changed").toBool() ){
        if ( static_cast<float>(state.cTemp[3]) == static_cast<float>(ui->cTemp3->value()) )
            ui->cTemp3->setProperty("changed", false);
    }
    else {
        ui->cTemp3->setValue( static_cast<float>(state.cTemp[3]) );
        ui->cTemp3->setProperty("changed", false);
    }
}

void FormCTemp::reset()
{
    ui->cTemp0->setProperty("changed", false);
    ui->cTemp1->setProperty("changed", false);
    ui->cTemp2->setProperty("changed", false);
    ui->cTemp3->setProperty("changed", false);
}

void FormCTemp::upload(){
    if ( ui->cTemp0->property("changed").toBool() ||
         ui->cTemp1->property("changed").toBool() ||
         ui->cTemp2->property("changed").toBool() ||
         ui->cTemp3->property("changed").toBool() ){

        QList<float> c = getCTempAll();

        QMetaObject::invokeMethod(dev, "setCTempAll", Qt::QueuedConnection, Q_ARG(QList<float>, c));

        reset();
    }
}

void FormCTemp::apply()
{
    upload();
}

void FormCTemp::restore()
{
    reset();
}

void FormCTemp::CTemp0Change()
{
    ui->cTemp0->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp1Change()
{
    ui->cTemp1->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp2Change()
{
    ui->cTemp2->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp3Change()
{
    ui->cTemp3->setProperty("changed", true);
    emit CTempChange();
}
