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
//    connect(ui->cStop0, &QDoubleSpinBox::editingFinished, this, &FormCStop::cStop0Save);
    ui->cStop1->setProperty("changed", false);
    connect(ui->cStop1, &QDoubleSpinBox::valueChanged, this, &FormCStop::cStop1Change);
//    connect(ui->cStop1, &QDoubleSpinBox::editingFinished, this, &FormCStop::cStop1Save);
}

FormCStop::~FormCStop()
{
    delete ui;
}

QDoubleSpinBox *FormCStop::getCStop0()
{
    return ui->cStop0;
}

QDoubleSpinBox *FormCStop::getCStop1()
{
    return ui->cStop1;
}

void FormCStop::cStop0Change()
{
    ui->cStop0->setProperty("changed", true);
}

void FormCStop::cStop0Save()
{
    if ( ! ui->cStop0->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCStop0", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cStop0->value())));
}

void FormCStop::cStop1Change()
{
    ui->cStop1->setProperty("changed", true);
}

void FormCStop::cStop1Save()
{
    if ( ! ui->cStop1->property("changed").toBool() )
        return;

    QMetaObject::invokeMethod(dev, "setCStop1", Qt::QueuedConnection, Q_ARG(float, (float)(ui->cStop1->value())));
}
