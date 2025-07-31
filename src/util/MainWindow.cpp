#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QShortcut>
#include <QKeySequence>

#include <QGridLayout>
#include <QHBoxLayout>

#include <QLegendMarker>
#include <QColor>

#include <iostream>
#include <QCheckBox>
#include <QPushButton>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "devNano.h"

#include <Eigen/Dense>

#include <QtCore/QTimer>

#include <QLabel>

#include <QtMath>

#include <QDebug>

#include <QToolTip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , devThread(new QThread(this))
    , dev(new devNano())
    , probeThread(new QThread(this))
    , probe(new Probe())
    , formPID(new FormPID(this, dev))
    , formFan(new FormFan(this, dev))
    , formCStop(new FormCStop(this, dev))
    , formCalib(new FormCalib(this, dev))
    , formPrefs(new FormPrefs(this, dev))
{
    ui->setupUi(this);

    // Create a shortcut for Ctrl+W
    QShortcut *shortcutCtrlW = new QShortcut(QKeySequence("Ctrl+W"), this);
    connect(shortcutCtrlW, &QShortcut::activated, this, &QWidget::close);

    // Create a shortcut for Ctrl+Q
    QShortcut *shortcutCtrlQ = new QShortcut(QKeySequence("Ctrl+Q"), this);
    connect(shortcutCtrlQ, &QShortcut::activated, this, &QWidget::close);


    // Preferences autostop, tempstep, screensaver
    QLabel *prefHeader = new QLabel(tr("Preferences"));
    prefHeader->setAlignment(Qt::AlignCenter);
    prefHeader->setStyleSheet("margin-top: 24px;");
    ui->right->addWidget(prefHeader);
    ui->right->addWidget(formPrefs);

    ui->right->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));


    // Create the buttons for eeprom actions
    eepromReset = new QPushButton(tr("Reset EEPROM"));
    connect(eepromReset, &QPushButton::clicked, this, &MainWindow::eepromResetSlot);
    eepromStore = new QPushButton(tr("Store on EEPROM"));
    connect(eepromStore, &QPushButton::clicked, this, &MainWindow::eepromStoreSlot);
    QHBoxLayout *eepromButtons = new QHBoxLayout;
    eepromButtons->addWidget(eepromReset);
    eepromButtons->addWidget(eepromStore);
    ui->right->addLayout(eepromButtons);


    // Setup window menu and child windows (views)
    setupViews();


    // Setup slots and signals

    // Device signals
    connect(dev, &devNano::dataIn, this, &MainWindow::devDataIn);
    connect(dev, &devNano::error, this, &MainWindow::devError);
    connect(devThread, &QThread::finished, dev, &QObject::deleteLater);
    connect(dev, &devNano::finished, devThread, &QThread::quit);
    connect(ui->devConnect, &QCheckBox::stateChanged, this, &MainWindow::devConnect);

    // Probe signals
    connect(probe, &Probe::dataIn, this, &MainWindow::probeDataIn);
    connect(probe, &Probe::error, this, &MainWindow::probeError);
    connect(probeThread, &QThread::finished, probe, &QObject::deleteLater);
    connect(probe, &Probe::finished, probeThread, &QThread::quit);
    connect(ui->probeConnect, &QCheckBox::stateChanged, this, &MainWindow::probeConnect);

    // Probe type
    ui->probeTA612c->setProperty("probeType", 0x04);
    connect(ui->probeTA612c, &QRadioButton::toggled, this, &MainWindow::setProbeType);
    ui->probeArduino->setProperty("probeType", 0x01);
    connect(ui->probeArduino, &QRadioButton::toggled, this, &MainWindow::setProbeType);


    // Timer for general screen updates
    QTimer *screenUpdates = new QTimer();
    // Regression charts
    QObject::connect(screenUpdates, &QTimer::timeout, this, &MainWindow::updateScreenData);

    screenUpdates->start(100);


    // Communication threads

    // Dispatch device object to its thread
    dev->moveToThread(devThread);
    // Setup device pulling interval
    int interval = refreshInterval;
    QMetaObject::invokeMethod(dev, "setInterval", Qt::QueuedConnection, Q_ARG(int, interval));
    devThread->start();

    // Dispatch probe object to its thread
    probe->moveToThread(probeThread);
    // Setup probe pulling interval
    QMetaObject::invokeMethod(probe, "setInterval", Qt::QueuedConnection, Q_ARG(int, interval));
    probeThread->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::restore()
{
    settings.beginGroup("Geometry");
    if (settings.contains("MainWindow"))
        restoreGeometry(settings.value("MainWindow").toByteArray());
    settings.endGroup();

    for (const auto& [name, view] : views) {
        view->restore();
    }
}


void MainWindow::setupViews()
{
    // Configure charts and set initial values
    setupCharts();

    QAction *menu = ui->Output_and_target_temperatures;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(tempChartA.chartView);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    menu = ui->Pre_and_probe_temperatures;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(tempChartB.chartView);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    menu = ui->PID;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(formPID);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    menu = ui->Stability;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(derivChart.chartView);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    menu = ui->Blower;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(formFan);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    // Calibration
    menu = ui->Calibration;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(formCalib);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    // Heating
    menu = ui->Heating;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(heatChart.chartView);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

    // Auto Stop
    menu = ui->Auto_stop;
    views[menu->objectName()] = new View(nullptr, menu);
    views[menu->objectName()]->layout->addWidget(autostopChart.chartView);
    // Form for autostop coefficients
    views[menu->objectName()]->layout->addWidget(formCStop);
    connect(menu, &QAction::triggered, this, &MainWindow::triggerView);

}

void MainWindow::setupCharts()
{

    // Size of the time series
    chartPastSize = chartPastTime / refreshInterval + 1;

    tempChartA.chart = new QChart();

    tempChartA.series[0] = new QLineSeries();
    tempChartA.series[0]->setName(tr("Output"));
    tempChartA.pen[0] = new QPen(QColor(52,101,164,128));
    tempChartA.pen[0]->setWidth(2);
    tempChartA.series[0]->setPen(*tempChartA.pen[0]);

    tempChartA.series[1] = new QLineSeries();
    tempChartA.series[1]->setName(tr("Target"));
    tempChartA.pen[1] = new QPen(QColor(206,92,0,128));
    tempChartA.pen[1]->setWidth(2);
    tempChartA.series[1]->setPen(*tempChartA.pen[1]);

    tempChartA.chart->setBackgroundVisible(false);

    tempChartA.chart->addSeries(tempChartA.series[0]);
    tempChartA.chart->addSeries(tempChartA.series[1]);

    tempChartA.chart->legend()->markers(tempChartA.series[0]).first()->setPen(*tempChartA.pen[0]);
    tempChartA.chart->legend()->markers(tempChartA.series[1]).first()->setPen(*tempChartA.pen[1]);

    tempChartA.axisX = new QValueAxis();
    tempChartA.axisX->setRange((float)(-(chartPastSize-1))/(1000.0/(float)refreshInterval), 0);
    tempChartA.axisX->setLabelFormat("%d");
    tempChartA.chart->addAxis(tempChartA.axisX, Qt::AlignBottom);
    tempChartA.series[0]->attachAxis(tempChartA.axisX);
    tempChartA.series[1]->attachAxis(tempChartA.axisX);

    tempChartA.axisY = new QValueAxis();
    tempChartA.axisY->setRange(0, 400);
    tempChartA.axisY->setLabelFormat("%d");
    tempChartA.chart->addAxis(tempChartA.axisY, Qt::AlignRight);
    tempChartA.series[0]->attachAxis(tempChartA.axisY);
    tempChartA.series[1]->attachAxis(tempChartA.axisY);

    tempChartA.chartView = new QChartView(tempChartA.chart);
    tempChartA.chartView->setRenderHint(QPainter::Antialiasing);
    tempChartA.chartView->setMinimumHeight(220);


    // Carregar valores fixos no eixo horizontal
    for (int j = 0; j < chartPastSize; ++j){
        tempChartA.series[0]->append((float)(-(chartPastSize-1)+j)/(1000.0/(float)refreshInterval), 0.0);
        tempChartA.series[1]->append((float)(-(chartPastSize-1)+j)/(1000.0/(float)refreshInterval), 0.0);
    }


    tempChartB.chart = new QChart();

    tempChartB.series[0] = new QLineSeries();
    tempChartB.series[0]->setName(tr("Pre"));
    tempChartB.pen[0] = new QPen(QColor(78,154,6,128));
    tempChartB.pen[0]->setWidth(2);
    tempChartB.series[0]->setPen(*tempChartB.pen[0]);

    tempChartB.series[1] = new QLineSeries();
    tempChartB.series[1]->setName(tr("Probe"));
    tempChartB.pen[1] = new QPen(QColor(204,0,0,128));
    tempChartB.pen[1]->setWidth(2);
    tempChartB.series[1]->setPen(*tempChartB.pen[1]);

    tempChartB.chart->setBackgroundVisible(false);

    tempChartB.chart->addSeries(tempChartB.series[0]);
    tempChartB.chart->addSeries(tempChartB.series[1]);

    tempChartB.chart->legend()->markers(tempChartB.series[0]).first()->setPen(*tempChartB.pen[0]);
    tempChartB.chart->legend()->markers(tempChartB.series[1]).first()->setPen(*tempChartB.pen[1]);

    tempChartB.axisX = new QValueAxis();
    tempChartB.axisX->setRange((float)(-(chartPastSize-1))/(1000.0/(float)refreshInterval), 0);
    tempChartB.axisX->setLabelFormat("%d");
    tempChartB.chart->addAxis(tempChartB.axisX, Qt::AlignBottom);
    tempChartB.series[0]->attachAxis(tempChartB.axisX);
    tempChartB.series[1]->attachAxis(tempChartB.axisX);

    tempChartB.axisY = new QValueAxis();
    tempChartB.axisY->setRange(0, 400);
    tempChartB.axisY->setLabelFormat("%d");
    tempChartB.chart->addAxis(tempChartB.axisY, Qt::AlignRight);
    tempChartB.series[0]->attachAxis(tempChartB.axisY);
    tempChartB.series[1]->attachAxis(tempChartB.axisY);

    tempChartB.chartView = new QChartView(tempChartB.chart);
    tempChartB.chartView->setRenderHint(QPainter::Antialiasing);
    tempChartB.chartView->setMinimumHeight(220);

    // Carregar valores fixos no eixo horizontal
    for (int j = 0; j < chartPastSize; ++j){
        tempChartB.series[0]->append((float)(-(chartPastSize-1)+j)/(1000.0/(float)refreshInterval), 0.0);
        tempChartB.series[1]->append((float)(-(chartPastSize-1)+j)/(1000.0/(float)refreshInterval), 0.0);
    }

    // Bar set for derivChart
    derivChart.barset = new QBarSet(tr("Stable if close to zero"));
    *(derivChart.barset) << 0 << 0 << 0;

    // Create a bar series and add the single bar set
    derivChart.series = new QBarSeries();
    derivChart.series->append(derivChart.barset);

    // Create the chart and add the bar series
    derivChart.chart = new QChart();
    derivChart.chart->addSeries(derivChart.series);
    derivChart.chart->setAnimationOptions(QChart::NoAnimation);
    derivChart.chart->setBackgroundVisible(false);

    // Create categories for the X-axis (only one category)
    derivChart.categories << "Pre" << "Probe" << "Load";
    derivChart.axisX = new QBarCategoryAxis();
    derivChart.axisX->append(derivChart.categories);
    derivChart.chart->addAxis(derivChart.axisX, Qt::AlignBottom);
    derivChart.series->attachAxis(derivChart.axisX);

    // Set Y-axis for the chart
    derivChart.axisY = new QValueAxis();
    derivChart.axisY->setRange(-5, 5);
    derivChart.axisY->setLabelFormat("%d");
    derivChart.chart->addAxis(derivChart.axisY, Qt::AlignLeft);
    derivChart.series->attachAxis(derivChart.axisY);

    // Create chart view to display the chart
    derivChart.chartView = new QChartView(derivChart.chart);
    derivChart.chartView->setRenderHint(QPainter::Antialiasing);
    derivChart.chartView->setMinimumHeight(220);



    // Heating element load chart
    heatChart.chart = new QChart();

    heatChart.series = new QLineSeries();
    heatChart.series->setName(tr("Load"));
    heatChart.pen = new QPen(QColor(85,87,83,128));
    heatChart.pen->setWidth(2);
    heatChart.series->setPen(*heatChart.pen);

    heatChart.chart->setBackgroundVisible(false);

    heatChart.chart->addSeries(heatChart.series);

    heatChart.chart->legend()->markers(heatChart.series).first()->setPen(*heatChart.pen);

    heatChart.axisX = new QValueAxis();
    heatChart.axisX->setRange((float)(-(chartPastSize-1))/(1000.0/(float)refreshInterval), 0);
    heatChart.axisX->setLabelFormat("%d");
    heatChart.chart->addAxis(heatChart.axisX, Qt::AlignBottom);
    heatChart.series->attachAxis(heatChart.axisX);

    heatChart.axisY = new QValueAxis();
    heatChart.axisY->setRange(0, 255);
    heatChart.axisY->setLabelFormat("%d");
    heatChart.chart->addAxis(heatChart.axisY, Qt::AlignRight);
    heatChart.series->attachAxis(heatChart.axisY);

    heatChart.chartView = new QChartView(heatChart.chart);
    heatChart.chartView->setRenderHint(QPainter::Antialiasing);
    heatChart.chartView->setMinimumHeight(220);

    // Carregar valores fixos no eixo horizontal
    for (int j = 0; j < chartPastSize; ++j){
        heatChart.series->append((float)(-(chartPastSize-1)+j)/(1000.0/(float)refreshInterval), 0.0);
    }


    // Auto Stop

    // Bar set for autostopChart
    autostopChart.barset[0] = new QBarSet(tr("Linear coefficients over time"));
    *(autostopChart.barset[0]) << 0 << 0;

    autostopChart.barset[1] = new QBarSet(tr("Maximum"));
    *(autostopChart.barset[1]) << 0 << 0;

    // Create a bar series and add the single bar set
    autostopChart.series = new QBarSeries();
    autostopChart.series->append(autostopChart.barset[0]);
    autostopChart.series->append(autostopChart.barset[1]);

    // Show tooltip when pointer hovers
    QObject::connect(autostopChart.series, &QBarSeries::hovered, [&](bool status, int index, QBarSet *barSet) {
        if (status) {  // When the pointer hovers over the bar
            // Get the value of the bar
            qreal value = barSet->at(index);

            // Get the global position of the mouse cursor and display the tooltip
            QPoint globalPos = QCursor::pos();
            QToolTip::showText(globalPos, QString::number(value));
        } else {
            // Hide the tooltip when the pointer leaves the bar
            QToolTip::hideText();
        }
    });

    // Create the chart and add the bar series
    autostopChart.chart = new QChart();
    autostopChart.chart->addSeries(autostopChart.series);
    autostopChart.chart->setTitle(tr("Auto stop threshold"));
    autostopChart.chart->setAnimationOptions(QChart::NoAnimation);
    autostopChart.chart->setBackgroundVisible(false);

    // Create categories for the X-axis
    autostopChart.categories << "Temperature" << "Heating";
    autostopChart.axisX = new QBarCategoryAxis();
    autostopChart.axisX->append(autostopChart.categories);
    autostopChart.chart->addAxis(autostopChart.axisX, Qt::AlignBottom);
    autostopChart.series->attachAxis(autostopChart.axisX);

    // Set Y-axis for the chart
    autostopChart.axisY = new QValueAxis();
    autostopChart.axisY->setRange(-0.1, 0.1);
    autostopChart.axisY->setLabelFormat("%.2f");
    autostopChart.chart->addAxis(autostopChart.axisY, Qt::AlignLeft);
    autostopChart.series->attachAxis(autostopChart.axisY);

    // Create chart view to display the chart
    autostopChart.chartView = new QChartView(autostopChart.chart);
    autostopChart.chartView->setRenderHint(QPainter::Antialiasing);
    autostopChart.chartView->setMinimumHeight(220);

}

void MainWindow::triggerView()
{
    if(QAction *action = qobject_cast<QAction*>(sender())) {

        auto it = views.find(action->objectName());
        if (it != views.end()) {
            View* view = it->second;
            if ( action->isChecked() ){
                view->show();
            }
            else {
                view->close();
            }
        }

    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.beginGroup("Geometry");
    settings.setValue("MainWindow", saveGeometry());
    settings.endGroup();

    for (const auto& [name, view] : views) {
        view->store();
    }

    QMetaObject::invokeMethod(dev, "stopReading", Qt::QueuedConnection);

    QMetaObject::invokeMethod(probe, "stopReading", Qt::QueuedConnection);

    devThread->quit();
    devThread->wait();

    probeThread->quit();
    probeThread->wait();

    QMainWindow::closeEvent(event);
    qApp->quit();
}

void MainWindow::devConnect(int state)
{
    if (ui->devLocation->count() == 0)
        return;

    if (state == Qt::Checked) {

        // Reset changed state of all input fields
        formPID->reset();

        formFan->getFanLoad()->setProperty("changed", false);

        formCalib->reset();

        formCStop->reset();

        formPrefs->getTempstep()->setProperty("changed", false);

        // Get the selected item from the combo box
        devPortName = ui->devLocation->currentText();

        ui->statusbar->showMessage(tr("Connected to ") + devPortName);

        QMetaObject::invokeMethod(dev, "setPortName", Qt::QueuedConnection, Q_ARG(QString, devPortName));

        ui->devLocation->setDisabled(true);

        QMetaObject::invokeMethod(dev, "startReading", Qt::QueuedConnection);
    }
    else {
        ui->statusbar->showMessage(tr("Disconnected"));
        QMetaObject::invokeMethod(dev, "stopReading", Qt::QueuedConnection);
        ui->devLocation->setDisabled(false);
    }
}

void MainWindow::probeConnect(int state)
{
    if (ui->probeLocation->count() == 0)
        return;

    if (state == Qt::Checked) {
        // Get the selected item from the combo box
        probePortName = ui->probeLocation->currentText();

        ui->statusbar->showMessage(tr("Connected to ") + probePortName);

        QMetaObject::invokeMethod(probe, "setPortName", Qt::QueuedConnection, Q_ARG(QString, probePortName));

        ui->probeLocation->setDisabled(true);
        ui->probeTA612c->setDisabled(true);
        ui->probeArduino->setDisabled(true);

        QMetaObject::invokeMethod(probe, "startReading", Qt::QueuedConnection);

    }
    else {
        QMetaObject::invokeMethod(probe, "stopReading", Qt::QueuedConnection);
        ui->probeLocation->setDisabled(false);
        ui->probeTA612c->setDisabled(false);
        ui->probeArduino->setDisabled(false);
    }
}

void MainWindow::devDataIn(const struct State& state)
{
    // Update charts and forms with data from device

    for (int j = 0; j < chartPastSize - 1; ++j){
        tempChartA.series[0]->replace(j, tempChartA.series[0]->at(j).x(), tempChartA.series[0]->at(j+1).y());
        tempChartA.series[1]->replace(j, tempChartA.series[1]->at(j).x(), tempChartA.series[1]->at(j+1).y());

        tempChartB.series[0]->replace(j, tempChartB.series[0]->at(j).x(), tempChartB.series[0]->at(j+1).y());

        heatChart.series->replace(j, heatChart.series->at(j).x(), heatChart.series->at(j+1).y());
    }

    tempChartA.series[1]->setName(tr("Target: ")+QString::number(static_cast<int>(state.tempTarget))+"°C");
    tempChartA.series[1]->replace(chartPastSize - 1, tempChartA.series[1]->at(chartPastSize - 1).x(), state.tempTarget);

    if ( static_cast<bool>(state.on) && static_cast<int>(state.PID[4]) > 0 ){
        tempChartA.series[0]->setName(tr("Output: ")+QString::number(static_cast<int>(state.tempEx))+"°C");
        tempChartA.series[0]->replace(chartPastSize - 1, tempChartA.series[0]->at(chartPastSize - 1).x(), state.tempEx);
    }
    else {
        tempChartA.series[0]->setName(tr("Output: ")+QString::number(static_cast<int>(state.tempCore))+"°C");
        tempChartA.series[0]->replace(chartPastSize - 1, tempChartA.series[0]->at(chartPastSize - 1).x(), state.tempCore);
    }
    tempChartB.series[0]->replace(chartPastSize - 1, tempChartB.series[0]->at(chartPastSize - 1).x(), state.tempCore);
    tempChartB.series[0]->setName(tr("Pre: ")+QString::number(static_cast<int>(state.tempCore))+"°C");

    heatChart.series->replace(chartPastSize - 1, heatChart.series->at(chartPastSize - 1).x(), state.PID[4]);
    heatChart.series->setName(tr("Load: ")+QString::number(static_cast<int>(state.PID[4])));


    // PID form fields
    formPID->devDataIn(state);


    // Fan form fields
    if ( QDateTime::fromString(formFan->getFanControl()->property("changedAt").toString()).msecsTo(QDateTime::currentDateTime()) > 1000 )
        formFan->getFanControl()->setChecked(static_cast<bool>(state.on));

    if ( ! formFan->getFanLoad()->property("changed").toBool() ){
        formFan->getFanLoad()->setValue( static_cast<int>(state.fan) );
        formFan->getFanLoad()->setProperty("changed", false);
    }

    // Elapsed running time
    int minutes = state.elapsed / 60;
    int seconds = state.elapsed % 60;
    formFan->getElapsed()->setText(QString("%1m%2s").arg(minutes, 1, 10, QChar('0')).arg(seconds, 1, 10, QChar('0')));


    // Coefficents of temperature profiles
    formCalib->devDataIn(state);


    // autostopChart
    autostopChart.barset[0]->replace(0, state.sStop[0]);
    autostopChart.barset[0]->replace(1, state.sStop[1]);
    // Max values
    if ( static_cast<bool>(state.on) && state.elapsed > 60 ){
        if ( autostopChart.barset[1]->at(0) < state.sStop[0] )
            autostopChart.barset[1]->replace(0, state.sStop[0]);
        if ( autostopChart.barset[1]->at(1) > state.sStop[1] )
            autostopChart.barset[1]->replace(1, state.sStop[1]);
    }
    // auto stop coefficients
    formCStop->devDataIn(state);


    // Preferences form fields
    if ( QDateTime::fromString(formPrefs->getAutostop()->property("changedAt").toString()).msecsTo(QDateTime::currentDateTime()) > 1000 )
        formPrefs->getAutostop()->setChecked(static_cast<bool>(state.autostop));

    if ( ! formPrefs->getTempstep()->property("changed").toBool() ){
        formPrefs->getTempstep()->setValue( static_cast<int>(state.tempStep) );
        formPrefs->getTempstep()->setProperty("changed", false);
    }

    if ( QDateTime::fromString(formPrefs->getScreensaver()->property("changedAt").toString()).msecsTo(QDateTime::currentDateTime()) > 1000 )
        formPrefs->getScreensaver()->setChecked(static_cast<bool>(state.screensaver));
}

void MainWindow::devError(const QString& error) {
    ui->statusbar->showMessage(error);
}

void MainWindow::probeDataIn(const float reading)
{
    for (int j = 0; j < chartPastSize - 1; ++j){
        tempChartB.series[1]->replace(j, tempChartB.series[1]->at(j).x(), tempChartB.series[1]->at(j+1).y());
    }

    tempChartB.series[1]->replace(chartPastSize - 1, tempChartB.series[1]->at(chartPastSize - 1).x(), reading);
    tempChartB.series[1]->setName(tr("Probe: ")+QString::number(static_cast<int>(reading))+"°C");

    formCalib->probeDataIn(reading);
}

void MainWindow::probeError(const QString &error)
{
    ui->statusbar->showMessage(error);
}

void MainWindow::setProbeType(bool checked)
{
    if (checked) {
        QRadioButton *button = qobject_cast<QRadioButton*>(sender());
        if (button) {
            char pt = (char)button->property("probeType").toInt();
            qDebug() << "probeType" << QString::number(pt);
            QMetaObject::invokeMethod(probe, "setProbeType", Qt::QueuedConnection, Q_ARG(char, pt));
        }
    }
}



void MainWindow::eepromResetSlot()
{
    QMetaObject::invokeMethod(dev, "eepromReset", Qt::QueuedConnection);
}

void MainWindow::eepromStoreSlot()
{
    QMetaObject::invokeMethod(dev, "eepromStore", Qt::QueuedConnection);
}

void MainWindow::updateScreenData(){

    // Get a list of all available serial ports
    const QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    if ( availablePorts.count() != ui->devLocation->count() ){
        // Replace ports list
        ui->devLocation->clear();
        ui->probeLocation->clear();
        for (const QSerialPortInfo &serialPortInfo : availablePorts) {
            ui->devLocation->addItem(serialPortInfo.portName());
            ui->probeLocation->addItem(serialPortInfo.portName());
        }
        ui->devLocation->setCurrentText(devPortName);
        ui->probeLocation->setCurrentText(probePortName);
    }

    if ( ui->devLocation->count() == 0 ){
        ui->devLocation->setDisabled(true);
        ui->devConnect->setDisabled(true);
    }
    else {
        if ( ui->devConnect->isChecked() ){
            ui->devLocation->setDisabled(true);
        }
        else {
            ui->devLocation->setDisabled(false);
            ui->devConnect->setDisabled(false);
        }
    }

    if ( ui->probeLocation->count() == 0 ){
        ui->probeLocation->setDisabled(true);
        ui->probeConnect->setDisabled(true);
        ui->probeTA612c->setDisabled(true);
        ui->probeArduino->setDisabled(true);
    }
    else {
        if ( ui->probeConnect->isChecked() ){
            ui->probeLocation->setDisabled(true);
            ui->probeTA612c->setDisabled(true);
            ui->probeArduino->setDisabled(true);
        }
        else {
            ui->probeLocation->setDisabled(false);
            ui->probeConnect->setDisabled(false);
            ui->probeTA612c->setDisabled(false);
            ui->probeArduino->setDisabled(false);
        }
    }


    // Enable/disable FormPID action buttons
    formPID->updateScreenData();

    // Enable/disable CStop action buttons
    formCStop->updateScreenData();

    formCalib->updateScreenData();

    // Update derivative charts
    regressions();
}

void MainWindow::regressions(){

    // How many recent points to compute
    const int tailPoints = 20;
    // Corresponding x and y coordinates of the recent points
    Eigen::VectorXd x(tailPoints);
    Eigen::VectorXd y(tailPoints);
    // Linear regression matrix
    Eigen::MatrixXd A(2, 2);
    // RHS
    Eigen::VectorXd b(2);
    // Line coefs resulting from the regression
    Eigen::VectorXd c(2);

    // Fill x coords used by all regressions
    for (int i = 0; i < tailPoints; ++i) {
        x(i) = static_cast<float>(i)/static_cast<float>(tailPoints);
    }
    // Regression matrix also common to all regressions
    for (int i = 0; i < 2 * 2; ++i) {
        A(i / 2, i % 2) = 0;
        for (int k = 0; k < tailPoints; ++k) {
          A(i / 2, i % 2) += pow(x(k), (i / 2) + (i % 2));
        }
    }
    // Inverse of A
    Eigen::MatrixXd A_inv = A.inverse();


    // Fill y coords for pre temp
    for (int i = 0; i < tailPoints; ++i) {
        y(i) = tempChartB.series[0]->points()[chartPastSize - tailPoints + i].y();
    }
    // Fill RHS for pre temp
    for (int j = 0; j < 2; ++j) {
      b(j) = 0;
      for (int i = 0; i < tailPoints; ++i) {
        b(j) += pow(x(i), j) * y(i);
      }
    }
    // Compute line coefs
    c = A_inv * b;
    // Update derivChart for pre temp
    derivChart.barset->replace(0, c(1));


    // Fill y coords for probe
    for (int i = 0; i < tailPoints; ++i) {
        y(i) = tempChartB.series[1]->points()[chartPastSize - tailPoints + i].y();
    }
    // Fill RHS for probe
    for (int j = 0; j < 2; ++j) {
      b(j) = 0;
      for (int i = 0; i < tailPoints; ++i) {
        b(j) += pow(x(i), j) * y(i);
      }
    }
    // Compute line coefs
    c = A_inv * b;
    // Update derivChart for probe temp
    derivChart.barset->replace(1, c(1));

    // Fill y coords for heat load
    for (int i = 0; i < tailPoints; ++i) {
        y(i) = heatChart.series->points()[chartPastSize - tailPoints + i].y();
    }
    // Fill RHS for heat load
    for (int j = 0; j < 2; ++j) {
      b(j) = 0;
      for (int i = 0; i < tailPoints; ++i) {
        b(j) += pow(x(i), j) * y(i);
      }
    }
    // Compute line coefs
    c = A_inv * b;
    // Update derivChart for heat load
    derivChart.barset->replace(2, c(1));

}
