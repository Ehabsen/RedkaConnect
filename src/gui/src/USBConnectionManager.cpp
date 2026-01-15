/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "USBConnectionManager.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>

// Protocol messages
const QByteArray USBConnectionManager::HANDSHAKE_REQUEST = "REDKA-USB-HANDSHAKE-REQUEST\n";
const QByteArray USBConnectionManager::HANDSHAKE_RESPONSE = "REDKA-USB-HANDSHAKE-RESPONSE\n";
const QByteArray USBConnectionManager::HEARTBEAT = "REDKA-USB-HEARTBEAT\n";

USBConnectionManager::USBConnectionManager(QObject* parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_discoveryTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_state(Disconnected)
{
    // Connect serial port signals
    connect(m_serialPort, &QSerialPort::readyRead, this, &USBConnectionManager::onSerialReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &USBConnectionManager::onSerialError);

    // Connect timers
    connect(m_discoveryTimer, &QTimer::timeout, this, &USBConnectionManager::onDiscoveryTimer);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &USBConnectionManager::onHeartbeatTimer);

    // Initial device scan
    refreshDevices();
}

USBConnectionManager::~USBConnectionManager()
{
    disconnect();
}

QList<USBConnectionManager::USBDevice> USBConnectionManager::availableDevices() const
{
    return m_devices;
}

void USBConnectionManager::refreshDevices()
{
    updateDeviceList();
}

bool USBConnectionManager::connectToDevice(const QString& portName)
{
    if (m_state == Connected) {
        disconnect();
    }

    setState(Connecting);
    m_errorString.clear();

    if (tryConnectToPort(portName)) {
        m_currentPort = portName;
        sendHandshake();

        if (waitForHandshakeResponse()) {
            setState(Connected);
            emit connected(portName);
            m_heartbeatTimer->start(HEARTBEAT_INTERVAL_MS);
            return true;
        } else {
            disconnect();
            m_errorString = "Handshake failed - not a RedkaConnect device";
            setState(Error);
            emit errorOccurred(m_errorString);
            return false;
        }
    } else {
        m_errorString = "Failed to open USB port: " + m_serialPort->errorString();
        setState(Error);
        emit errorOccurred(m_errorString);
        return false;
    }
}

void USBConnectionManager::disconnect()
{
    m_heartbeatTimer->stop();

    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    if (m_state != Disconnected) {
        emit disconnected();
        setState(Disconnected);
        m_currentPort.clear();
    }
}

bool USBConnectionManager::sendData(const QByteArray& data)
{
    if (m_state != Connected || !m_serialPort->isOpen()) {
        return false;
    }

    qint64 written = m_serialPort->write(data);
    return written == data.size();
}

QByteArray USBConnectionManager::readData()
{
    if (m_state != Connected || !m_serialPort->isOpen()) {
        return QByteArray();
    }

    return m_serialPort->readAll();
}

void USBConnectionManager::startAutoDiscovery()
{
    m_discoveryTimer->start(DISCOVERY_INTERVAL_MS);
}

void USBConnectionManager::stopAutoDiscovery()
{
    m_discoveryTimer->stop();
}

void USBConnectionManager::onSerialReadyRead()
{
    QByteArray data = m_serialPort->readAll();

    // Check for protocol messages
    if (data == HANDSHAKE_REQUEST) {
        // Respond to handshake
        m_serialPort->write(HANDSHAKE_RESPONSE);
        return;
    }

    if (data == HEARTBEAT) {
        // Heartbeat received - connection is alive
        return;
    }

    // Regular data
    emit dataReceived(data);
}

void USBConnectionManager::onSerialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        m_errorString = QString("USB serial error: %1").arg(m_serialPort->errorString());
        emit errorOccurred(m_errorString);

        if (m_state == Connected) {
            disconnect();
        }
    }
}

void USBConnectionManager::onDiscoveryTimer()
{
    updateDeviceList();
}

void USBConnectionManager::onHeartbeatTimer()
{
    if (m_state == Connected && m_serialPort->isOpen()) {
        m_serialPort->write(HEARTBEAT);
    } else {
        // Connection lost
        disconnect();
    }
}

void USBConnectionManager::updateDeviceList()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QMap<QString, USBDevice> currentDevices;

    for (const QSerialPortInfo& port : ports) {
        // Only show potential RedkaConnect devices
        // Look for CDC-ACM devices or devices with "Redka" in description
        bool isPotentialDevice = port.description().contains("CDC", Qt::CaseInsensitive) ||
                                port.description().contains("ACM", Qt::CaseInsensitive) ||
                                port.description().contains("Redka", Qt::CaseInsensitive) ||
                                port.manufacturer().contains("Redka", Qt::CaseInsensitive);

        if (isPotentialDevice) {
            USBDevice device;
            device.portName = port.portName();
            device.description = port.description();
            device.manufacturer = port.manufacturer();
            device.serialNumber = port.serialNumber();
            device.isConnected = (device.portName == m_currentPort && m_state == Connected);
            device.lastSeen = QDateTime::currentMSecsSinceEpoch();

            currentDevices[device.portName] = device;
        }
    }

    // Check for new devices
    for (const USBDevice& device : currentDevices) {
        bool found = false;
        for (const USBDevice& existing : m_devices) {
            if (existing.portName == device.portName) {
                found = true;
                break;
            }
        }

        if (!found) {
            m_devices.append(device);
            emit deviceDiscovered(device);
        }
    }

    // Check for removed devices
    QList<QString> toRemove;
    for (const USBDevice& existing : m_devices) {
        if (!currentDevices.contains(existing.portName)) {
            toRemove.append(existing.portName);
        }
    }

    for (const QString& portName : toRemove) {
        for (int i = 0; i < m_devices.size(); ++i) {
            if (m_devices[i].portName == portName) {
                m_devices.removeAt(i);
                emit deviceRemoved(portName);
                break;
            }
        }
    }
}

bool USBConnectionManager::tryConnectToPort(const QString& portName)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    return m_serialPort->open(QIODevice::ReadWrite);
}

void USBConnectionManager::sendHandshake()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->write(HANDSHAKE_REQUEST);
    }
}

bool USBConnectionManager::waitForHandshakeResponse()
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < HANDSHAKE_TIMEOUT_MS) {
        if (m_serialPort->waitForReadyRead(100)) {
            QByteArray response = m_serialPort->readAll();
            if (response == HANDSHAKE_RESPONSE) {
                return true;
            }
        }
    }

    return false;
}

void USBConnectionManager::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}

QString USBConnectionManager::detectDeviceType(const QString& portName)
{
    // This could be enhanced to detect specific USB device types
    // For now, just return generic USB serial
    return "USB Serial Device";
}