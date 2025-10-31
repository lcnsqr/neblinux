#include "devNano.h"

#include <QThread>

#include <iostream>

#include <limits>
#include <cstdint>

// Helper shared with probe.cpp; kept local copy here to avoid cross-file dependency.
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
        QThread::msleep(1);
    }
    return rx_bytes == size;
}

devNano::devNano(QObject* parent)
    : QObject(parent), serial(new QSerialPort(this)), timer(new QTimer(this))
{
    memset(&state, 0, sizeof(struct State));
    memset(&stateOut, 0, sizeof(struct StateIO));

    // Transmission error check
    stateOut.serialCheck = SERIAL_TAG;

    // Call pull() on timer timeout
    connect(timer, &QTimer::timeout, this, &devNano::pull);
}

devNano::~devNano()
{
    stopReading();
}

void devNano::setPortName(QString path)
{
    portName = path;
}

void devNano::setBaudRate(qint32 baud)
{
    baudRate = baud;
}

void devNano::setInterval(int msecs)
{
    updateInterval = msecs;
    timer->setInterval(updateInterval);
}

void devNano::startReading()
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

            // Fetch data
            timer->start(updateInterval);
        }

    } catch (std::exception &e) {
        emit error("Error: " + QString(e.what()));
    }

}

void devNano::stopReading()
{
    timer->stop();
    if (serial->isOpen()) {
        serial->close();
    }
}

void devNano::pull()
{
    if (!serial->isOpen()){
        return;
    }

    // Wait grace time period before transfering
    if ( connectTime.msecsTo(QDateTime::currentDateTime()) < 2000 ){
        return;
    }

    try {

        // Local state to receive serial data
        struct State stateLocal;

        int rx_bytes = 0;

        const char cmd = SERIAL_READ;

        // Prepare check field to avoid transmission errors
        stateLocal.serialCheck = ~ SERIAL_TAG;

        serial->clear();
        serial->write(&cmd, 1);

        serial->waitForBytesWritten(1000);
        QThread::msleep(40);

        // Read exactly sizeof(struct State) bytes with timeout
        char *buf = reinterpret_cast<char*>(&stateLocal);
        if (!readFully(serial, buf, static_cast<qint64>(sizeof(struct State)), 1000)) {
          emit error(tr("Timeout reading device state"));
          return;
        }

        // Validate serialCheck marker (the device is expected to flip the field)
        if (stateLocal.serialCheck != SERIAL_TAG) {
          emit error(tr("Serial check mismatch (corrupt/shifted packet)"));
          return;
        }

        if ( stateLocal.serialCheck == SERIAL_TAG ){
            // Tags match
            state = stateLocal;

            // Atualizar a estrutura de envio com o estado recebido
            stateOut.tempTarget = state.tempTarget;
            stateOut.tempStep = state.tempStep;
            stateOut.fan = state.fan;
            stateOut.PID_enabled = state.PID_enabled;
            stateOut.heat = (state.PID_enabled) ? 0 : state.PID[4];
            stateOut.cTemp[0] = state.cTemp[0];
            stateOut.cTemp[1] = state.cTemp[1];
            stateOut.cTemp[2] = state.cTemp[2];
            stateOut.cTemp[3] = state.cTemp[3];
            stateOut.cPID[0] = state.cPID[0];
            stateOut.cPID[1] = state.cPID[1];
            stateOut.cPID[2] = state.cPID[2];
            stateOut.autostop = state.autostop;
            stateOut.screensaver = state.screensaver;
            stateOut.splash = state.splash;
            stateOut.cStop[0] = state.cStop[0];
            stateOut.cStop[1] = state.cStop[1];

            emit dataIn(state);
        }


    } catch (std::exception &e) {
        emit error("Erro: " + QString(e.what()));
    }

}

void devNano::push()
{
    if (!serial->isOpen()){
        return;
    }

    // Wait grace time period before transfering
    if ( connectTime.msecsTo(QDateTime::currentDateTime()) < 2000 ){
        return;
    }

    try {

        const char cmd = SERIAL_WRITE;

        serial->clear();
        serial->write(&cmd, 1);

        serial->waitForBytesWritten(1000);
        QThread::usleep(static_cast<qint64>(40e+3));

        serial->write((char *)&stateOut, sizeof(struct StateIO));

        serial->waitForBytesWritten(1000);

    } catch (std::exception &e) {
        emit error("Erro: " + QString(e.what()));
    }

}

void devNano::prepareCalib()
{
    stateOut.PID_enabled = 0;
    stateOut.heat = 0;
    push();
    QThread::usleep(static_cast<qint64>(300e+3));
    deviceCmd(SERIAL_START);
}

void devNano::finishCalib()
{
    deviceCmd(SERIAL_STOP);
    QThread::usleep(static_cast<qint64>(300e+3));
    stateOut.heat = 0;
    stateOut.PID_enabled = 1;
    push();
}

void devNano::setTempTarget(float value) {
    stateOut.tempTarget = value;
    push();
}

void devNano::setCPID0(float value)
{
    stateOut.cPID[0] = value;
    push();
}

void devNano::setCPID1(float value)
{
    stateOut.cPID[1] = value;
    push();
}

void devNano::setCPID2(float value)
{
    stateOut.cPID[2] = value;
    push();
}

void devNano::setFan(bool flag)
{
    if ( flag )
        deviceCmd(SERIAL_START);
    else
        deviceCmd(SERIAL_STOP);
}

void devNano::setFanLoad(float value)
{
    stateOut.fan = value;
    push();
}

void devNano::setCTemp0(float value)
{
    stateOut.cTemp[0] = value;
    push();
}

void devNano::setCTemp1(float value)
{
    stateOut.cTemp[1] = value;
    push();
}

void devNano::setCTemp2(float value)
{
    stateOut.cTemp[2] = value;
    push();
}

void devNano::setCTemp3(float value)
{
    stateOut.cTemp[3] = value;
    push();
}

void devNano::setCTempAll(const QList<float> &values)
{
    stateOut.cTemp[0] = values.at(0);
    stateOut.cTemp[1] = values.at(1);
    stateOut.cTemp[2] = values.at(2);
    stateOut.cTemp[3] = values.at(3);
    push();
}

void devNano::enablePID(int value)
{
    stateOut.PID_enabled = value;
    push();
}

void devNano::setHeatLoad(float value)
{
    stateOut.heat = value;
    push();
}

void devNano::setCStop0(float value)
{
    stateOut.cStop[0] = value;
    push();
}

void devNano::setCStop1(float value)
{
    stateOut.cStop[1] = value;
    push();
}

void devNano::autostop(int value)
{
    stateOut.autostop = value;
    push();
}

void devNano::tempstep(int value)
{
    stateOut.tempStep = value;
    push();
}

void devNano::screensaver(int value)
{
    stateOut.screensaver = value;
    push();
}

void devNano::eepromStore()
{
    qDebug() << "Device store";
    deviceCmd(SERIAL_STORE);
}

void devNano::eepromReset()
{
    qDebug() << "Device reset";
    deviceCmd(SERIAL_RESET);
}

void devNano::deviceCmd(const char cmd)
{
    if (!serial->isOpen()){
        return;
    }

    // Wait grace time period before transfering
    if ( connectTime.msecsTo(QDateTime::currentDateTime()) < 2000 ){
        return;
    }

    try {

        serial->clear();
        serial->write(&cmd, 1);

        serial->waitForBytesWritten(1000);
        QThread::usleep(static_cast<qint64>(40e+3));

    } catch (std::exception &e) {
        //std::cerr << error("Erro: " + QString(e.what())) << std::endl;
    }
}
