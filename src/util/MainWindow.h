#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "devNano.h"
#include "probe.h"

#include <QString>

#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QPointF>

#include "FormPID.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>

#include "FormHeat.h"

#include "FormFan.h"

#include <QtCharts/QScatterSeries>
#include <QPen>

#include "FormCalib.h"

#include "FormCTemp.h"

#include "FormCStop.h"

#include "FormPrefs.h"

#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    // calibChart should be public
    struct CalibChart calibChart;

private:
    Ui::MainWindow *ui;

    // Neblinux device connected to the serial port
    QThread* devThread;
    devNano* dev;
    QString devPortName;

    // Temperature probe connected to the serial port
    QThread* probeThread;
    Probe* probe;
    QString probePortName;

    static const int refreshInterval = 250;
    static const int chartPastSize = 201;

    // tempChartA
    struct {
        QLineSeries *series[2];
        QPen *pen[2];
        QChart *chart;
        QValueAxis *axisX;
        QValueAxis *axisY;
        QChartView *chartView;
    } tempChartA;

    // tempChartB
    struct {
        QLineSeries *series[2];
        QPen *pen[2];
        QChart *chart;
        QValueAxis *axisX;
        QValueAxis *axisY;
        QChartView *chartView;
    } tempChartB;

    // PID fields
    FormPID* formPID;

    // Heat fields (PID enabled
    // checkbox and manual value)
    FormHeat* formHeat;

    // Fan fields (Fan control
    // fan load and last duration)
    FormFan* formFan;

    // cStop fields
    FormCStop* formCStop;

    // cTemp fields
    FormCTemp* formCTemp;

    // Calibration fields
    FormCalib* formCalib;

    // derivChart
    struct {
        QBarSet *barset;
        QBarSeries *series;
        QChart *chart;
        QStringList categories;
        QBarCategoryAxis *axisX;
        QValueAxis *axisY;
        QChartView *chartView;
    } derivChart;

    // heatChart
    struct {
        QLineSeries *series;
        QPen *pen;
        QChart *chart;
        QValueAxis *axisX;
        QValueAxis *axisY;
        QChartView *chartView;
    } heatChart;

    // autostopChart
    struct {
        QBarSet *barset[2];
        QBarSeries *series;
        QChart *chart;
        QStringList categories;
        QBarCategoryAxis *axisX;
        QValueAxis *axisY;
        QChartView *chartView;
    } autostopChart;

    // Preferences (autostop, tempstep and screensaver)
    FormPrefs* formPrefs;

    // Calibration managing buttons
    QPushButton *calibSwitch = nullptr;
    QPushButton *calibUpCoefs = nullptr;

    // Third degree polynomial to fit calibration points
    QList<float> cTempCoeffs;
    void calibFitPoints();

    // EEPROM action buttons
    QPushButton *eepromReset = nullptr;
    QPushButton *eepromStore = nullptr;

    // Update derivative charts
    void regressions();

public slots:
    void devConnect(int);
    void devDataIn(const struct State& state);
    void devError(const QString& error);

    void probeConnect(int);
    void probeDataIn(const float reading);
    void probeError(const QString& error);

    void setProbeType(bool checked);

    void calibSwitchSlot(bool pressed);
    void calibUpCoefsSlot();
    void calibPolyFill();

    void eepromResetSlot();
    void eepromStoreSlot();

    void updateScreenData();

};
#endif // MAINWINDOW_H
