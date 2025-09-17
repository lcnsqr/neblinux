#ifndef DEVNANO_H
#define DEVNANO_H

#include <QObject>
#include <QTimer>

#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>

#include "state.h"
#include <array>

class devNano: public QObject {
    Q_OBJECT

public:
    explicit devNano(QObject* parent = nullptr);
    ~devNano();

public slots:
    void setPortName(QString path);
    void setBaudRate(qint32 baud);
    void setInterval(int msecs);

    void startReading();
    void stopReading();

    void pull();
    void push();

    void prepareCalib();
    void finishCalib();

    void setTempTarget(float value);
    void setCPID0(float value);
    void setCPID1(float value);
    void setCPID2(float value);

    void setFan(bool value);
    void setFanLoad(float value);

    void setCTemp0(float value);
    void setCTemp1(float value);
    void setCTemp2(float value);
    void setCTemp3(float value);
    void setCTempAll(const QList<float>& values);

    void enablePID(int value);
    void setHeatLoad(float value);

    void setCStop0(float value);
    void setCStop1(float value);

    void autostop(int value);
    void tempstep(int value);
    void screensaver(int value);

    void eepromStore();
    void eepromReset();

signals:
    void dataIn(const struct State& data);
    void error(const QString& error);
    void finished();

private:
    // When the connection begun
    QDateTime connectTime;

    // Serial communication
    QSerialPort* serial;
    QString portName;
    qint32 baudRate = QSerialPort::Baud115200;

    // Timer to call pull() on timeout
    QTimer* timer;
    int updateInterval = 250;

    // Send one byte command to device
    void deviceCmd(const char cmd);

    // Data load
    struct State state;
    struct StateIO stateOut;
};

#endif // DEVNANO_H
