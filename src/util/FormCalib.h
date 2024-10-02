#ifndef FORMCALIB_H
#define FORMCALIB_H

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QRadioButton>

#include "devNano.h"

#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QChartView>

// calibChart from mainWindow
struct CalibChart {
    // Points distribution stuff
    int size;
    float min;
    float max;
    float from;
    float to;
    float len;
    float iFrom;
    float iTo;
    float iLen;
    float s;
    float sd;
    QList<QPointF> prePoints;
    QList<QPointF> probePoints;
    QLineSeries *series[2];
    QScatterSeries *scatter[2];
    QPen *pen[2];
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QChartView *chartView;
};

namespace Ui {
class FormCalib;
}

class FormCalib : public QWidget
{
    Q_OBJECT

public:
    explicit FormCalib(QWidget *parent = nullptr, CalibChart *calibChartRef = nullptr, devNano *d = nullptr);
    ~FormCalib();

    void updatePoints();

    void setManualDisabled(bool value);
    void setManualValue(int targetIndex, int value);

    bool getCalibRunning() const;
    void setCalibRunning(bool newCalibRunning);

    bool getCalibManual() const;
    void setCalibManual(bool newCalibManual);

    int currentLoad();
    int selectedIndex();
    int loadByIndex(int targetIndex);

public slots:
    void calibPointSelect(bool checked);

    void pointNewLoad();

    void tempManualInput();

    void tempManualToggle(int state);

    void getFormReady();

private:
    Ui::FormCalib *ui;

    CalibChart *calibChart;
    devNano *dev;

    bool calibRunning = false;
    bool calibManual = false;
};

#endif // FORMCALIB_H
