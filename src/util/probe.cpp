#include "probe.h"

#include <QThread>

#include <iostream>

#include <limits>
#include <cstdint>

// readFully helper reads only the remaining bytes (min(avail, remaining))
// and loops until the expected byte count is gathered or the timeout elapses.

static bool readFully(QSerialPort* serial, char* buf, qint64 size, int timeoutMs)
{
    qint64 rx_bytes = 0;
    QDateTime start = QDateTime::currentDateTime();

    while (rx_bytes < size && start.msecsTo(QDateTime::currentDateTime()) < timeoutMs) {
        if (!serial->waitForReadyRead(100))
            continue;

        qint64 avail = serial->bytesAvailable();
        qint64 remaining = size - rx_bytes;
        qint64 toRead = qMin(avail, remaining);
        if (toRead <= 0)
            continue;

        qint64 n = serial->read(buf + rx_bytes, toRead);
        if (n < 0) {
            return false;
        }
        rx_bytes += n;
        // small sleep to allow hardware buffer to refill if needed
        QThread::msleep(1);
    }
    return rx_bytes == size;
}

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
    float reply = std::numeric_limits<float>::quiet_NaN();

    serial->clear();
    serial->write(&cmd, 1);

    serial->waitForBytesWritten(1000);
    // wait 40ms for a response (previous code used usleep(40e+3))
    QThread::msleep(40);

    // Read exactly sizeof(float) bytes
    char buf[sizeof(float)] = {0};
    if (!readFully(serial, buf, sizeof(buf), 1000)) {
        // read timeout or error; return NaN so caller can detect it
        return reply;
    }

    // Copy raw bytes into float. Keep endianness as original code expects host layout.
    static_assert(sizeof(float) == 4, "Unexpected float size");
    memcpy(&reply, buf, sizeof(float));

    return reply;
}
