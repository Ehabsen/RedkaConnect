/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMap>
#include <QByteArray>

/**
 * @brief USB Connection Manager
 *
 * Handles direct USB cable connections between computers.
 * Uses USB Serial (CDC-ACM) protocol for peer-to-peer communication.
 *
 * Features:
 * - Auto-detection of USB serial devices
 * - Automatic pairing over USB cable
 * - Fallback to manual device selection
 * - Visual feedback for cable connection status
 */
class USBConnectionManager : public QObject
{
    Q_OBJECT

public:
    struct USBDevice {
        QString portName;          // COM3, /dev/ttyACM0, etc.
        QString description;       // Device description
        QString manufacturer;      // Manufacturer name
        QString serialNumber;      // Serial number
        bool isConnected;          // Currently connected
        qint64 lastSeen;           // Timestamp
    };

    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    explicit USBConnectionManager(QObject* parent = nullptr);
    ~USBConnectionManager() override;

    // Device management
    QList<USBDevice> availableDevices() const;
    void refreshDevices();
    bool connectToDevice(const QString& portName);
    void disconnect();

    // Communication
    bool sendData(const QByteArray& data);
    QByteArray readData();

    // State
    ConnectionState state() const { return m_state; }
    QString currentPort() const { return m_currentPort; }
    QString errorString() const { return m_errorString; }

    // Auto-discovery
    void startAutoDiscovery();
    void stopAutoDiscovery();

signals:
    void deviceDiscovered(const USBDevice& device);
    void deviceRemoved(const QString& portName);
    void connected(const QString& portName);
    void disconnected();
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);
    void stateChanged(ConnectionState state);

private slots:
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
    void onDiscoveryTimer();
    void onHeartbeatTimer();

private:
    void updateDeviceList();
    bool tryConnectToPort(const QString& portName);
    void sendHandshake();
    bool waitForHandshakeResponse();
    void setState(ConnectionState state);
    QString detectDeviceType(const QString& portName);

    QList<USBDevice> m_devices;
    QSerialPort* m_serialPort;
    QTimer* m_discoveryTimer;
    QTimer* m_heartbeatTimer;

    ConnectionState m_state;
    QString m_currentPort;
    QString m_errorString;

    // Protocol constants
    static constexpr int HANDSHAKE_TIMEOUT_MS = 5000;
    static constexpr int HEARTBEAT_INTERVAL_MS = 2000;
    static constexpr int DISCOVERY_INTERVAL_MS = 1000;

    // Protocol messages
    static const QByteArray HANDSHAKE_REQUEST;
    static const QByteArray HANDSHAKE_RESPONSE;
    static const QByteArray HEARTBEAT;
};