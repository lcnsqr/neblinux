#ifndef PROBE_H
#define PROBE_H

#include <QObject>
#include <QTimer>

#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>

#include <array>

class Probe: public QObject {
    Q_OBJECT

public:
    explicit Probe(QObject* parent = nullptr);
    ~Probe();

public slots:
    void setPortName(QString path);
    void setBaudRate(qint32 baud);
    void setInterval(int msecs);

    void startReading();
    void stopReading();

    void pull();

signals:
    void dataIn(const float reading);
    void error(const QString& error);
    void finished();

private:
    // When the connection begun
    QDateTime connectTime;

    // Serial communication
    QSerialPort* serial;
    QString portName;
    qint32 baudRate = QSerialPort::Baud9600;

    // Timer to call pull() on timeout
    QTimer* timer;
    int updateInterval = 250;

    // Data load
    float reading;
};

#endif // PROBE_H
