#include "FormCalib.h"
#include "ui_FormCalib.h"

#include <QLegendMarker>

#include <QTimer>
#include <QDebug>

#include <Eigen/Dense>

FormCalib::FormCalib(QWidget *parent, devNano *d) :
    QWidget(parent),
    ui(new Ui::FormCalib),
    dev(d),
    formCTemp(new FormCTemp(this, dev))
{
    ui->setupUi(this);

    // Setup calibration chart

    // Calibration points
    calibChart.size = 8;
    calibChart.min = 10;
    calibChart.max = 105;
    // Domain for the density function (tan)
    calibChart.from = -1.0;
    calibChart.to = 0.7;
    calibChart.len = calibChart.to - calibChart.from;
    calibChart.iFrom = qTan(calibChart.from);
    calibChart.iTo = qTan(calibChart.to);
    calibChart.iLen = calibChart.iTo - calibChart.iFrom;
    calibChart.s = 0;
    calibChart.sd = calibChart.len / (calibChart.size - 1);

    while (calibChart.s <= calibChart.len + 1e-5)
    {
        float x = qRound(calibChart.min + (calibChart.max - calibChart.min) * (qTan(calibChart.from + calibChart.s) - calibChart.iFrom) / calibChart.iLen);
        // Use placeholder values for y
        calibChart.prePoints.append(QPointF(x, 25 + 51 * calibChart.s));
        calibChart.probePoints.append(QPointF(x, 50 + 143 * calibChart.s));
        calibChart.s += calibChart.sd;
    }

    // Setup calibration chart
    calibChart.chart = new QChart();
    // Reduce top and bottom margins
    QMargins calibChartMargins = calibChart.chart->margins();
    calibChartMargins.setTop(0);
    calibChartMargins.setBottom(0);
    calibChart.chart->setMargins(calibChartMargins);

    calibChart.series[0] = new QLineSeries();
    calibChart.series[0]->setName(tr("Pre"));
    calibChart.series[0]->append(calibChart.prePoints);
    calibChart.pen[0] = new QPen(QColor(78,154,6,128));
    calibChart.pen[0]->setWidth(2);
    calibChart.series[0]->setPen(*calibChart.pen[0]);

    calibChart.scatter[0] = new QScatterSeries();
    calibChart.scatter[0]->append(calibChart.prePoints);
    calibChart.scatter[0]->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    calibChart.scatter[0]->setMarkerSize(8.0);
    calibChart.scatter[0]->setColor(QColor(78,154,6,128));
    calibChart.scatter[0]->setPen(Qt::NoPen);

    calibChart.series[1] = new QLineSeries();
    calibChart.series[1]->setName(tr("Probe"));
    calibChart.series[1]->append(calibChart.probePoints);
    calibChart.pen[1] = new QPen(QColor(204,0,0,128));
    calibChart.pen[1]->setWidth(2);
    calibChart.series[1]->setPen(*calibChart.pen[1]);

    calibChart.scatter[1] = new QScatterSeries();
    calibChart.scatter[1]->append(calibChart.probePoints);
    calibChart.scatter[1]->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    calibChart.scatter[1]->setMarkerSize(8.0);
    calibChart.scatter[1]->setBrush(QColor(204,0,0,128));
    calibChart.scatter[1]->setPen(Qt::NoPen);

    calibChart.chart->setBackgroundVisible(false);

    calibChart.chart->addSeries(calibChart.series[0]);
    calibChart.chart->addSeries(calibChart.scatter[0]);
    calibChart.chart->addSeries(calibChart.series[1]);
    calibChart.chart->addSeries(calibChart.scatter[1]);

    calibChart.chart->legend()->markers(calibChart.series[0]).first()->setPen(*calibChart.pen[0]);
    calibChart.chart->legend()->markers(calibChart.scatter[0]).first()->setVisible(false);
    calibChart.chart->legend()->markers(calibChart.series[1]).first()->setPen(*calibChart.pen[1]);
    calibChart.chart->legend()->markers(calibChart.scatter[1]).first()->setVisible(false);

    calibChart.axisX = new QValueAxis();
    calibChart.axisX->setRange(calibChart.min, calibChart.max);
    calibChart.axisX->setTickCount(2);
    calibChart.axisX->setLabelFormat("%d");
    calibChart.chart->addAxis(calibChart.axisX, Qt::AlignBottom);
    calibChart.series[0]->attachAxis(calibChart.axisX);
    calibChart.scatter[0]->attachAxis(calibChart.axisX);
    calibChart.series[1]->attachAxis(calibChart.axisX);
    calibChart.scatter[1]->attachAxis(calibChart.axisX);

    calibChart.axisY = new QValueAxis();
    calibChart.axisY->setRange(0, 400);
    calibChart.axisY->setLabelFormat("%d");
    calibChart.chart->addAxis(calibChart.axisY, Qt::AlignRight);
    calibChart.series[0]->attachAxis(calibChart.axisY);
    calibChart.scatter[0]->attachAxis(calibChart.axisY);
    calibChart.series[1]->attachAxis(calibChart.axisY);
    calibChart.scatter[1]->attachAxis(calibChart.axisY);

    calibChart.chartView = new QChartView(calibChart.chart);
    calibChart.chartView->setRenderHint(QPainter::Antialiasing);
    calibChart.chartView->setMinimumHeight(220);

    ui->right->addWidget(calibChart.chartView);

    // Update calibChart from user given coefficients
    connect(formCTemp, &FormCTemp::CTempChange, this, &FormCalib::calibPolyFill);
    ui->right->addWidget(formCTemp);


    // Create the action buttons for calibration
    calibSwitch = new QPushButton(tr("Start calibration"));
    calibSwitch->setCheckable(true);
    connect(calibSwitch, &QPushButton::toggled, this, &FormCalib::calibSwitchSlot);

    QHBoxLayout *calibButtons = new QHBoxLayout;
//    calibButtons->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    calibButtons->addWidget(calibSwitch);
//    calibButtons->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    ui->left->insertLayout(0, calibButtons);

    // Form with every calib point in calibChart
    for (int i = 0; i < calibChart.prePoints.size(); ++i){
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
        load->setValue(static_cast<int>(calibChart.prePoints[i].x()));
        connect(load, &QSpinBox::editingFinished, this, &FormCalib::pointNewLoad);
        ui->load->insertWidget(0, load, 0, Qt::AlignHCenter);

        QSpinBox* manual = new QSpinBox();
        manual->setObjectName("manualCalib" + QString::number(i));
        manual->setProperty("index", i);
        manual->setMaximumSize(110, 29);
        manual->setMaximum(400);
        manual->setValue(calibChart.series[1]->at(i).y());
        connect(manual, &QSpinBox::editingFinished, this, &FormCalib::tempManualInput);
        ui->manual->insertWidget(0, manual, 0, Qt::AlignHCenter);
    }

    // Disable manual temperature input fields
    setManualDisabled(true);
    connect(ui->calibManualToggle, &QCheckBox::stateChanged, this, &FormCalib::tempManualToggle);

    // Generate coefficients for points in chart
    calibFitPoints();

}

FormCalib::~FormCalib()
{
    delete ui;
}

void FormCalib::updateScreenData()
{
    if ( calibRunning ){
        // Compute coefficients on the fly
        calibFitPoints();
    }

    formCTemp->updateScreenData();
}

void FormCalib::devDataIn(const State &state)
{

    // Calibration chart
    if ( calibRunning ){
        int calibIndex = selectedIndex();
        calibChart.series[0]->replace(calibIndex, calibChart.series[0]->at(calibIndex).x(), state.tempCore);
        calibChart.scatter[0]->replace(calibIndex, calibChart.scatter[0]->at(calibIndex).x(), state.tempCore);
    }

    // Update temperature profile coefficients
    formCTemp->devDataIn(state);
}

void FormCalib::reset()
{
    formCTemp->reset();
}

void FormCalib::probeDataIn(const float reading)
{
    // Calibration chart
    if ( calibRunning && ! calibManual ){
        int calibIndex = selectedIndex();
        calibChart.series[1]->replace(calibIndex, calibChart.series[1]->at(calibIndex).x(), reading);
        calibChart.scatter[1]->replace(calibIndex, calibChart.scatter[1]->at(calibIndex).x(), reading);
        setManualValue(calibIndex, reading);
    }

}


void FormCalib::calibFitPoints()
{
    //qDebug() << "Generate coefficients from calibration";

    std::vector<QPointF> points;

    // Add the initial fixed point (20, 20)
    points.emplace_back(20.0, 20.0);

    // Get the points from the series (y values from series[0] as x, y values from series[1] as y)
    for (int i = 0; i < calibChart.size; ++i) {
        float x = calibChart.series[0]->at(i).y();
        float y = calibChart.series[1]->at(i).y();
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

        //qDebug() << x << y ;

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
        if (button && calibRunning) {
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
        calibChart.series[0]->replace(index, (float)load, calibChart.series[0]->at(index).y());
        calibChart.scatter[0]->replace(index, (float)load, calibChart.scatter[0]->at(index).y());

        // Probe
        calibChart.series[1]->replace(index, (float)load, calibChart.series[1]->at(index).y());
        calibChart.scatter[1]->replace(index, (float)load, calibChart.scatter[1]->at(index).y());

        if ( index == 0 ){
            calibChart.min = load;
            calibChart.axisX->setMin((float)load);
        }

        if ( index == calibChart.prePoints.size() - 1 ){
            calibChart.max = load;
            calibChart.axisX->setMax((float)load);
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
        calibChart.series[1]->replace(index, calibChart.series[1]->at(index).x(), (float)temp);
        calibChart.scatter[1]->replace(index, calibChart.scatter[1]->at(index).x(), (float)temp);
    }
}

void FormCalib::tempManualToggle(int state)
{
    if (state == Qt::Checked) {
        calibManual = true;
        setManualDisabled(false);

    }
    else {
        calibManual = false;
        setManualDisabled(true);
    }
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


int FormCalib::currentLoad()
{
    return loadByIndex(selectedIndex());
}

void FormCalib::calibSwitchSlot(bool pressed)
{

    if ( pressed ){
        qDebug() << "Switch calibration on";
        QMetaObject::invokeMethod(dev, "prepareCalib", Qt::QueuedConnection);
        calibRunning = true;
        qDebug() << "setHeatLoad" << currentLoad();
        QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, (float)(currentLoad())));
        calibSwitch->setText(tr("Stop calibration"));
    }
    else {
        qDebug() << "Switch calibration off";
        QMetaObject::invokeMethod(dev, "finishCalib", Qt::QueuedConnection);
        calibRunning = false;
        qDebug() << "setHeatLoad" << 0;
        QMetaObject::invokeMethod(dev, "setHeatLoad", Qt::QueuedConnection, Q_ARG(float, 0));
        calibSwitch->setText(tr("Start calibration"));
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

    if ( calibRunning )
        return;

    QList<float> c = formCTemp->getCTempAll();

    for (int i = 0; i < calibChart.size; ++i) {
        float x = calibChart.series[0]->at(i).y();
        float y = c.at(0) + c.at(1) * x + c.at(2) * x*x + c.at(3) * x*x*x;
        calibChart.series[1]->replace(i, calibChart.series[1]->at(i).x(), y);
        calibChart.scatter[1]->replace(i, calibChart.scatter[1]->at(i).x(), y);
    }

}
