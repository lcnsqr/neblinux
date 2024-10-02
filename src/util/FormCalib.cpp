#include "FormCalib.h"
#include "ui_FormCalib.h"

#include <QTimer>
#include <QDebug>

FormCalib::FormCalib(QWidget *parent, CalibChart *calibChartRef, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormCalib),
    calibChart(calibChartRef),
    dev(d)
{
    ui->setupUi(this);

    // formCalib is tight
    ui->index->setSpacing(0);
    ui->load->setSpacing(0);
    ui->manual->setSpacing(0);

    connect(ui->calibManualToggle, &QCheckBox::stateChanged, this, &FormCalib::tempManualToggle);

}

FormCalib::~FormCalib()
{
    delete ui;
}

void FormCalib::updatePoints()
{
    for (int i = 0; i < calibChart->prePoints.size(); ++i){
        QRadioButton* selector = new QRadioButton();
        selector->setObjectName("radioCalib" + QString::number(i));
        selector->setProperty("index", i);
        selector->setMaximumSize(23, 29);
        if ( i == 0 )
            selector->setChecked(true);
        connect(selector, &QRadioButton::toggled, this, &FormCalib::calibPointSelect);
        ui->index->insertWidget(0, selector, 0, Qt::AlignHCenter);

        QSpinBox* load = new QSpinBox();
        load->setObjectName("loadCalib" + QString::number(i));
        load->setProperty("index", i);
        load->setMaximumSize(110, 29);
        load->setMaximum(255);
        load->setValue(static_cast<int>(calibChart->prePoints[i].x()));
        connect(load, &QSpinBox::editingFinished, this, &FormCalib::pointNewLoad);
        ui->load->insertWidget(0, load, 0, Qt::AlignHCenter);

        QSpinBox* manual = new QSpinBox();
        manual->setObjectName("manualCalib" + QString::number(i));
        manual->setProperty("index", i);
        manual->setMaximumSize(110, 29);
        manual->setMaximum(400);
        manual->setValue(calibChart->series[1]->at(i).y());
        connect(manual, &QSpinBox::editingFinished, this, &FormCalib::tempManualInput);
        ui->manual->insertWidget(0, manual, 0, Qt::AlignHCenter);
    }
    QTimer::singleShot(250, this, &FormCalib::getFormReady);
}

void FormCalib::setManualDisabled(bool value)
{
    for (int i = 0; i < ui->manual->count(); ++i) {
        QWidget *widget = ui->manual->itemAt(i)->widget();
        if (widget) {
            QSpinBox *manual = qobject_cast<QSpinBox*>(widget);
            if (manual) {
                manual->setDisabled(value);
            }
        }
    }
}

void FormCalib::setManualValue(int targetIndex, int value)
{
    for (int i = 0; i < ui->manual->count(); ++i) {
        QWidget *widget = ui->manual->itemAt(i)->widget();
        if (widget) {
            QSpinBox *manual = qobject_cast<QSpinBox*>(widget);
            if (manual) {
                int index = manual->property("index").toInt();
                if (index == targetIndex)
                    manual->setValue(value);
            }
        }
    }
}

void FormCalib::calibPointSelect(bool checked)
{
    if (checked) {
        QRadioButton *button = qobject_cast<QRadioButton*>(sender());
        if (button && getCalibRunning()) {
            int index = button->property("index").toInt();
            qDebug() << "setHeatLoad" << loadByIndex(index);
            QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, (float)(loadByIndex(index))));
        }
    }
}

void FormCalib::pointNewLoad()
{
    QSpinBox *loadBox = qobject_cast<QSpinBox*>(sender());
    if (loadBox) {
        int index = loadBox->property("index").toInt();
        int load = loadBox->value();

        // Pre
        calibChart->series[0]->replace(index, (float)load, calibChart->series[0]->at(index).y());
        calibChart->scatter[0]->replace(index, (float)load, calibChart->scatter[0]->at(index).y());

        // Probe
        calibChart->series[1]->replace(index, (float)load, calibChart->series[1]->at(index).y());
        calibChart->scatter[1]->replace(index, (float)load, calibChart->scatter[1]->at(index).y());

        if ( index == 0 ){
            calibChart->min = load;
            calibChart->axisX->setMin((float)load);
        }

        if ( index == calibChart->prePoints.size() - 1 ){
            calibChart->max = load;
            calibChart->axisX->setMax((float)load);
        }
    }
}

void FormCalib::tempManualInput()
{
    QSpinBox *tempBox = qobject_cast<QSpinBox*>(sender());
    if (tempBox) {
        int index = tempBox->property("index").toInt();
        int temp = tempBox->value();

        // Put it in the probe series
        calibChart->series[1]->replace(index, calibChart->series[1]->at(index).x(), (float)temp);
        calibChart->scatter[1]->replace(index, calibChart->scatter[1]->at(index).x(), (float)temp);
    }
}

void FormCalib::tempManualToggle(int state)
{
    if (state == Qt::Checked) {
        setCalibManual(true);
        setManualDisabled(false);

    }
    else {
        setCalibManual(false);
        setManualDisabled(true);
    }
}

void FormCalib::getFormReady()
{
    setManualDisabled(true);
}

int FormCalib::selectedIndex()
{
    for (int i = 0; i < ui->index->count(); ++i) {
        QWidget *widget = ui->index->itemAt(i)->widget();
        if (widget) {
            QRadioButton *radio = qobject_cast<QRadioButton*>(widget);
            if (radio && radio->isChecked() ) {
                QVariant indexValue = radio->property("index");
                return indexValue.toInt();
            }
        }
    }
    return 0;
}

int FormCalib::loadByIndex(int targetIndex)
{
    for (int i = 0; i < ui->load->count(); ++i) {
        QWidget *widget = ui->load->itemAt(i)->widget();
        if (widget) {
            QVariant indexValue = widget->property("index");
            if (indexValue.isValid() && indexValue.toInt() == targetIndex) {
                return qobject_cast<QSpinBox*>(widget)->value();
            }
        }
    }
    return 0;
}

bool FormCalib::getCalibManual() const
{
    return calibManual;
}

void FormCalib::setCalibManual(bool newCalibManual)
{
    calibManual = newCalibManual;
}

int FormCalib::currentLoad()
{
    return loadByIndex(selectedIndex());
}

bool FormCalib::getCalibRunning() const
{
    return calibRunning;
}

void FormCalib::setCalibRunning(bool newCalibRunning)
{
    calibRunning = newCalibRunning;
}
