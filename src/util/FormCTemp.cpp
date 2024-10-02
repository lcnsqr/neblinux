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
    connect(ui->cTemp0, &QDoubleSpinBox::editingFinished, this, &FormCTemp::CTemp0Save);

    ui->cTemp1->setProperty("changed", false);
    connect(ui->cTemp1, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp1Change);
    connect(ui->cTemp1, &QDoubleSpinBox::editingFinished, this, &FormCTemp::CTemp1Save);

    ui->cTemp2->setProperty("changed", false);
    connect(ui->cTemp2, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp2Change);
    connect(ui->cTemp2, &QDoubleSpinBox::editingFinished, this, &FormCTemp::CTemp2Save);

    ui->cTemp3->setProperty("changed", false);
    connect(ui->cTemp3, &QDoubleSpinBox::valueChanged, this, &FormCTemp::CTemp3Change);
    connect(ui->cTemp3, &QDoubleSpinBox::editingFinished, this, &FormCTemp::CTemp3Save);
}

FormCTemp::~FormCTemp()
{
    delete ui;
}

QDoubleSpinBox *FormCTemp::getCTemp0()
{
    return ui->cTemp0;
}

QDoubleSpinBox *FormCTemp::getCTemp1()
{
    return ui->cTemp1;
}

QDoubleSpinBox *FormCTemp::getCTemp2()
{
    return ui->cTemp2;
}

QDoubleSpinBox *FormCTemp::getCTemp3()
{
    return ui->cTemp3;
}

QDoubleSpinBox *FormCTemp::getCTemp(int i)
{
    QDoubleSpinBox *cTemp = nullptr;
    if ( i == 0 )
        cTemp = ui->cTemp0;
    else if ( i == 1 )
        cTemp = ui->cTemp1;
    else if ( i == 2 )
        cTemp = ui->cTemp2;
    else if ( i == 3 )
        cTemp = ui->cTemp3;
    return cTemp;
}

void FormCTemp::CTemp0Change()
{
    ui->cTemp0->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp0Save()
{
    if ( ! ui->cTemp0->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCTemp0", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cTemp0->value())));
}

void FormCTemp::CTemp1Change()
{
    ui->cTemp1->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp1Save()
{
    if ( ! ui->cTemp1->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCTemp1", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cTemp1->value())));
}

void FormCTemp::CTemp2Change()
{
    ui->cTemp2->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp2Save()
{
    if ( ! ui->cTemp2->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCTemp2", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cTemp2->value())));
}

void FormCTemp::CTemp3Change()
{
    ui->cTemp3->setProperty("changed", true);
    emit CTempChange();
}

void FormCTemp::CTemp3Save()
{
    if ( ! ui->cTemp3->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCTemp3", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cTemp3->value())));
}
