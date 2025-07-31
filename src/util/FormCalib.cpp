#include "FormCalib.h"
#include "ui_FormCalib.h"

#include <QTimer>
#include <QDebug>

#include <Eigen/Dense>

FormCalib::FormCalib(QWidget *parent, CalibChart *calibChartRef, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormCalib),
    calibChart(calibChartRef),
    dev(d),
    formCTemp(new FormCTemp(this, dev))
{
    ui->setupUi(this);

    // formCalib is tight
    //ui->left->index->setSpacing(0);
    //ui->left->load->setSpacing(0);
    //ui->left->manual->setSpacing(0);

    connect(ui->calibManualToggle, &QCheckBox::stateChanged, this, &FormCalib::tempManualToggle);


    // Update calibChart from user given coefficients
//    connect(formCTemp, &FormCTemp::CTempChange, this, &FormCalib::calibPolyFill);
}

FormCalib::~FormCalib()
{
    delete ui;
}

void FormCalib::updateScreenData()
{
    // Enable/disable calibration
//    if ( formFan->getFanControl()->isChecked() ){
//        calibSwitch->setDisabled(false);
//    }
//    else {
//        calibSwitch->setDisabled(true);
//        calibSwitch->setChecked(false);
//    }

    if ( getCalibRunning() ){
        // Compute coefficients on the fly
        calibFitPoints();
    }

    formCTemp->updateScreenData();
}

void FormCalib::devDataIn(const State &state)
{
    formCTemp->devDataIn(state);
}

void FormCalib::reset()
{
    formCTemp->reset();
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


void FormCalib::calibFitPoints()
{
    qDebug() << "Generate coefficients from calibration";

    std::vector<QPointF> points;

    // Add the initial fixed point (20, 20)
    points.emplace_back(20.0, 20.0);

    // Get the points from the series (y values from series[0] as x, y values from series[1] as y)
    for (int i = 0; i < calibChart->size; ++i) {
        float x = calibChart->series[0]->at(i).y();
        float y = calibChart->series[1]->at(i).y();
        points.emplace_back(x, y);
    }

    // Number of points
    int n = points.size();

    // Construct the A matrix and b vector
    Eigen::MatrixXd A(n, 4);  // n rows, 4 columns (for x^0, x^1, x^2, x^3)
    Eigen::VectorXd b(n);     // n rows (for y values)

    for (int i = 0; i < n; ++i) {
        float x = points[i].x();
        float y = points[i].y();

        qDebug() << x << y ;

        A(i, 0) = 1;
        A(i, 1) = x;
        A(i, 2) = x * x;
        A(i, 3) = x * x * x;
        b(i) = y;
    }

    // Solve for the coefficients using least squares (normal equation: A^T A c = A^T b)
    Eigen::VectorXd coeffs = A.colPivHouseholderQr().solve(b);

    // Update cTemp values
    cTempCoeffs.clear();
    for (int i = 0; i < coeffs.size(); ++i) {
        cTempCoeffs.append(static_cast<float>(coeffs(i)));
    }
    formCTemp->setCTempAll(cTempCoeffs);
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

    // Create the action buttons for calibration
    calibSwitch = new QPushButton(tr("Run calibration"));
    calibSwitch->setCheckable(true);
    connect(calibSwitch, &QPushButton::toggled, this, &FormCalib::calibSwitchSlot);

//    calibUpCoefs = new QPushButton(tr("Send to device"));
//    connect(calibUpCoefs, &QPushButton::clicked, this, &FormCalib::calibUpCoefsSlot);

    QHBoxLayout *calibButtons = new QHBoxLayout;
    calibButtons->addWidget(calibSwitch);
//    calibButtons->addWidget(calibUpCoefs);
    ui->left->addLayout(calibButtons);

    ui->right->addWidget(calibChart->chartView);

    ui->right->addWidget(formCTemp);

    // Generate coefficients from initial points
    calibFitPoints();
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
void FormCalib::calibSwitchSlot(bool pressed)
{
    if ( pressed ){
        qDebug() << "Switch calibration on";
        setCalibRunning(true);
        qDebug() << "setHeatLoad" << currentLoad();
        QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, (float)(currentLoad())));
    }
    else {
        qDebug() << "Switch calibration off";
        setCalibRunning(false);
        qDebug() << "setHeatLoad" << 0;
        QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, 0));
    }
}

//void FormCalib::calibUpCoefsSlot()
//{
//    qDebug() << "Upload new coefficients to device";

//    formCTemp->upload();
//}

void FormCalib::calibPolyFill()
{
    // Compute calibration probe points from polynomial coefficients

    if ( getCalibRunning() )
        return;

//    QList<float> c = formCTemp->getCTempAll();

//    for (int i = 0; i < calibChart->size; ++i) {
//        float x = calibChart->series[0]->at(i).y();
//        float y = c.at(0) + c.at(1) * x + c.at(2) * x*x + c.at(3) * x*x*x;
//        calibChart->series[1]->replace(i, calibChart->series[1]->at(i).x(), y);
//        calibChart->scatter[1]->replace(i, calibChart->scatter[1]->at(i).x(), y);
//    }

}
