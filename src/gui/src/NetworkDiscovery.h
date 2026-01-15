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
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QMap>
#include <QDateTime>

/**
 * @brief Network discovery service for RedkaConnect
 * 
 * Uses UDP broadcast to discover other RedkaConnect instances
 * on the local network. This is a simple, cross-platform alternative
 * to Bonjour/Zeroconf that works without additional dependencies.
 * 
 * Protocol:
 * - Discovery port: 24801 (UDP)
 * - Broadcast message format: "REDKA|<version>|<name>|<mode>|<port>|<code>"
 *   - version: Protocol version (1)
 *   - name: Computer name
 *   - mode: "SERVER" or "CLIENT" 
 *   - port: TCP port for connection (default 24800)
 *   - code: Pairing code (for servers)
 */
class NetworkDiscovery : public QObject
{
    Q_OBJECT

public:
    static constexpr quint16 DISCOVERY_PORT = 24801;
    static constexpr quint16 CONNECTION_PORT = 24800;
    static constexpr int BROADCAST_INTERVAL_MS = 2000;      // How often to broadcast presence
    static constexpr int SCAN_INTERVAL_MS = 1000;           // How often to check for new devices
    static constexpr int DEVICE_TIMEOUT_MS = 10000;         // Remove device after this silence
    static constexpr int PROTOCOL_VERSION = 1;

    struct DiscoveredDevice {
        QString name;
        QString address;
        quint16 port;
        QString pairingCode;
        bool isServer;          // true = sharing, false = looking to connect
        QDateTime lastSeen;
        
        bool isValid() const { return !address.isEmpty(); }
        bool isExpired() const { 
            return lastSeen.msecsTo(QDateTime::currentDateTime()) > DEVICE_TIMEOUT_MS;
        }
    };

    explicit NetworkDiscovery(QObject* parent = nullptr);
    ~NetworkDiscovery() override;

    /**
     * @brief Start broadcasting as a server (sharing)
     * @param computerName This computer's name
     * @param pairingCode The pairing code to share
     */
    void startBroadcastingAsServer(const QString& computerName, const QString& pairingCode);

    /**
     * @brief Start listening for servers (connecting)
     * @param computerName This computer's name (for potential future features)
     */
    void startListeningForServers(const QString& computerName);

    /**
     * @brief Stop all discovery activity
     */
    void stop();

    /**
     * @brief Check if discovery is currently active
     */
    bool isActive() const { return m_isActive; }

    /**
     * @brief Get list of currently discovered devices
     */
    QList<DiscoveredDevice> discoveredDevices() const;

    /**
     * @brief Manually trigger a refresh/scan
     */
    void refresh();

    /**
     * @brief Set broadcast interval
     * @param intervalMs Milliseconds between broadcasts (default 2000)
     */
    void setBroadcastInterval(int intervalMs);

    /**
     * @brief Set scan/listen interval  
     * @param intervalMs Milliseconds between scans (default 1000)
     */
    void setScanInterval(int intervalMs);

    /**
     * @brief Set device timeout
     * @param timeoutMs Milliseconds before removing inactive device (default 10000)
     */
    void setDeviceTimeout(int timeoutMs);

signals:
    /**
     * @brief Emitted when a new device is discovered
     */
    void deviceDiscovered(const DiscoveredDevice& device);

    /**
     * @brief Emitted when a device goes offline (timeout)
     */
    void deviceLost(const QString& address);

    /**
     * @brief Emitted when a device's info is updated
     */
    void deviceUpdated(const DiscoveredDevice& device);

    /**
     * @brief Emitted on network errors
     */
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();
    void onBroadcastTimer();
    void onCleanupTimer();

private:
    void sendBroadcast();
    void processIncomingPacket(const QByteArray& data, const QHostAddress& sender);
    QByteArray createBroadcastPacket() const;
    DiscoveredDevice parsePacket(const QByteArray& data, const QHostAddress& sender) const;
    QList<QHostAddress> getBroadcastAddresses() const;
    void cleanupExpiredDevices();

    QUdpSocket* m_socket;
    QTimer* m_broadcastTimer;
    QTimer* m_cleanupTimer;

    QString m_computerName;
    QString m_pairingCode;
    bool m_isServer;
    bool m_isActive;

    int m_broadcastIntervalMs;
    int m_scanIntervalMs;
    int m_deviceTimeoutMs;

    // Map of address -> device info
    QMap<QString, DiscoveredDevice> m_devices;
};
