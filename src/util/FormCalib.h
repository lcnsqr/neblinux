#ifndef FORMCALIB_H
#define FORMCALIB_H

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QRadioButton>

#include <QPushButton>

#include "devNano.h"

#include "FormCTemp.h"

#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QChartView>

namespace Ui {
class FormCalib;
}

class FormCalib : public QWidget
{
    Q_OBJECT

public:
    explicit FormCalib(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormCalib();

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

    void probeDataIn(const float reading);

    int currentLoad();
    int selectedIndex();
    int loadByIndex(int targetIndex);

public slots:
    void calibPointSelect(bool checked);

    void pointNewLoad();

    void tempManualInput();

    void tempManualToggle(int state);

    void calibSwitchSlot(bool pressed);

    void calibPolyFill();

private:
    Ui::FormCalib *ui;

    devNano *dev;

    // Chart attributes
    struct {
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
    } calibChart;

    // cTemp fields
    FormCTemp* formCTemp;

    void setManualDisabled(bool value);
    void setManualValue(int targetIndex, int value);

    bool calibRunning = false;
    bool calibManual = false;

    // Calibration managing buttons
    QPushButton *calibSwitch = nullptr;

    // Third degree polynomial to fit calibration points
    QList<float> cTempCoeffs;
    void calibFitPoints();
};

#endif // FORMCALIB_H
