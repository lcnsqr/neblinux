#include "probe.h"

#include <QThread>

#include <iostream>


Probe::Probe(QObject* parent)
    : QObject(parent), serial(new QSerialPort(this)), timer(new QTimer(this))
{

    // Defaults to TA612c
    probeType = 0x04;
    setBaudRate(QSerialPort::Baud9600);

    // Call pull() on timer timeout
    connect(timer, &QTimer::timeout, this, &Probe::pull);

}

Probe::~Probe()
{
    stopReading();
}

void Probe::setPortName(QString path)
{
    portName = path;
}

void Probe::setBaudRate(qint32 baud)
{
    baudRate = baud;
}

void Probe::setInterval(int msecs)
{
    updateInterval = msecs;
    timer->setInterval(updateInterval);
}

void Probe::setProbeType(char t)
{
    probeType = t;
    if ( probeType == 0x01 ){
        setBaudRate(QSerialPort::Baud115200);
    }
    else {
        setBaudRate(QSerialPort::Baud9600);
    }
}

void Probe::startReading()
{

    if (serial->isOpen())
        serial->close();

    try {
        // Configura a porta serial
        serial->setPortName(portName);
        serial->setBaudRate(baudRate);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (!serial->open(QIODevice::ReadWrite)) {
            throw std::runtime_error("Error opening serial port");
        }
        else {
            // Store when the connection started
            connectTime = QDateTime::currentDateTime();

            // Fetch data every 100 ms (1/10 second)
            timer->start(updateInterval);
        }

    } catch (std::exception &e) {
        emit error("Error: " + QString(e.what()));
    }

}

void Probe::stopReading()
{
    timer->stop();
    if (serial->isOpen()) {
        serial->close();
    }
}

void Probe::pull()
{
    if (!serial->isOpen()){
        return;
    }

    // Wait grace time period before transfering
    if ( connectTime.msecsTo(QDateTime::currentDateTime()) < 2000 ){
        return;
    }

    try {

        if ( probeType == 0x01 ){
            emit dataIn(pullArduino());
        }
        else {
            emit dataIn(pullTA612c());
        }


    } catch (std::exception &e) {
        emit error("Probe error: " + QString(e.what()));
    }

}

float Probe::pullTA612c()
{
    // The four readings of the TA612c
    std::array<float, 4> T;

    // Read command
    char writeOut[5] = {(char)0xAA, (char)0x55, (char)0x01, (char)0x03, (char)0x03};
    serial->write(writeOut, sizeof(writeOut));

    QThread::usleep(static_cast<qint64>(1e+3));

    // Raw load
    char readIn[13] = {0};

    if (serial->waitForReadyRead(1000)) {  // Aguarda até 1 segundo por dados
        serial->read(readIn, sizeof(readIn));

        // Bit conversions
        T[0] = (float)((readIn[5] << 8) | (readIn[4] & 0xff)) / 10.0;
        T[1] = (float)((readIn[7] << 8) | (readIn[6] & 0xff)) / 10.0;
        T[2] = (float)((readIn[9] << 8) | (readIn[8] & 0xff)) / 10.0;
        T[3] = (float)((readIn[11] << 8) | (readIn[10] & 0xff)) / 10.0;

        // Arithmetic mean
        return (T[0]+T[1]+T[2]+T[3])/4.0;
    }
    return 0;
}

float Probe::pullArduino()
{
    const char cmd = SERIAL_READ;

    float reply = 0;

    int rx_bytes = 0;

    serial->clear();
    serial->write(&cmd, 1);

    serial->waitForBytesWritten(1000);
    QThread::usleep(static_cast<qint64>(40e+3));

    // QSerialPort object reads buffer size chunks
    rx_bytes = 0;

    // Don't dwell in the loop for more than one second
    QDateTime t = QDateTime::currentDateTime();

    while ( rx_bytes < (int)sizeof(float) && t.msecsTo(QDateTime::currentDateTime()) < 1000 ){
        if (serial->waitForReadyRead(100)) {
            rx_bytes += serial->read((char*)&reply + rx_bytes, serial->bytesAvailable());
            QThread::usleep(static_cast<qint64>(40e+3));
        }
    }

    return reply;

}
