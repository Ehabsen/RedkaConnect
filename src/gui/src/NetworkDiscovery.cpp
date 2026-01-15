/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "NetworkDiscovery.h"

#include <QNetworkInterface>
#include <QNetworkDatagram>

NetworkDiscovery::NetworkDiscovery(QObject* parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
    , m_broadcastTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_isServer(false)
    , m_isActive(false)
    , m_broadcastIntervalMs(BROADCAST_INTERVAL_MS)
    , m_scanIntervalMs(SCAN_INTERVAL_MS)
    , m_deviceTimeoutMs(DEVICE_TIMEOUT_MS)
{
    connect(m_socket, &QUdpSocket::readyRead, this, &NetworkDiscovery::onReadyRead);
    connect(m_broadcastTimer, &QTimer::timeout, this, &NetworkDiscovery::onBroadcastTimer);
    connect(m_cleanupTimer, &QTimer::timeout, this, &NetworkDiscovery::onCleanupTimer);
}

NetworkDiscovery::~NetworkDiscovery()
{
    stop();
}

void NetworkDiscovery::startBroadcastingAsServer(const QString& computerName, const QString& pairingCode)
{
    stop();  // Stop any existing activity
    
    m_computerName = computerName;
    m_pairingCode = pairingCode;
    m_isServer = true;
    m_isActive = true;
    
    // Bind to discovery port to also receive discovery requests
    if (!m_socket->bind(QHostAddress::Any, DISCOVERY_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit errorOccurred(QString("Failed to bind to discovery port %1: %2")
            .arg(DISCOVERY_PORT).arg(m_socket->errorString()));
        return;
    }
    
    // Start broadcasting our presence
    m_broadcastTimer->start(m_broadcastIntervalMs);
    
    // Also start cleanup timer for any responses
    m_cleanupTimer->start(m_scanIntervalMs);
    
    // Send initial broadcast immediately
    sendBroadcast();
}

void NetworkDiscovery::startListeningForServers(const QString& computerName)
{
    stop();  // Stop any existing activity
    
    m_computerName = computerName;
    m_pairingCode.clear();
    m_isServer = false;
    m_isActive = true;
    
    // Bind to discovery port to receive broadcasts
    if (!m_socket->bind(QHostAddress::Any, DISCOVERY_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit errorOccurred(QString("Failed to bind to discovery port %1: %2")
            .arg(DISCOVERY_PORT).arg(m_socket->errorString()));
        return;
    }
    
    // Start cleanup timer to remove stale devices
    m_cleanupTimer->start(m_scanIntervalMs);
    
    // Optionally broadcast that we're looking for servers
    // This can help servers know there's interest
    m_broadcastTimer->start(m_broadcastIntervalMs * 2);  // Less frequent for clients
    sendBroadcast();
}

void NetworkDiscovery::stop()
{
    m_isActive = false;
    m_broadcastTimer->stop();
    m_cleanupTimer->stop();
    m_socket->close();
    m_devices.clear();
}

QList<NetworkDiscovery::DiscoveredDevice> NetworkDiscovery::discoveredDevices() const
{
    QList<DiscoveredDevice> result;
    for (const auto& device : m_devices) {
        if (!device.isExpired()) {
            result.append(device);
        }
    }
    return result;
}

void NetworkDiscovery::refresh()
{
    if (m_isActive) {
        sendBroadcast();
        cleanupExpiredDevices();
    }
}

void NetworkDiscovery::setBroadcastInterval(int intervalMs)
{
    m_broadcastIntervalMs = intervalMs;
    if (m_broadcastTimer->isActive()) {
        m_broadcastTimer->setInterval(intervalMs);
    }
}

void NetworkDiscovery::setScanInterval(int intervalMs)
{
    m_scanIntervalMs = intervalMs;
    if (m_cleanupTimer->isActive()) {
        m_cleanupTimer->setInterval(intervalMs);
    }
}

void NetworkDiscovery::setDeviceTimeout(int timeoutMs)
{
    m_deviceTimeoutMs = timeoutMs;
}

void NetworkDiscovery::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        processIncomingPacket(datagram.data(), datagram.senderAddress());
    }
}

void NetworkDiscovery::onBroadcastTimer()
{
    sendBroadcast();
}

void NetworkDiscovery::onCleanupTimer()
{
    cleanupExpiredDevices();
}

void NetworkDiscovery::sendBroadcast()
{
    QByteArray packet = createBroadcastPacket();
    
    // Send to all broadcast addresses
    for (const QHostAddress& broadcastAddr : getBroadcastAddresses()) {
        m_socket->writeDatagram(packet, broadcastAddr, DISCOVERY_PORT);
    }
    
    // Also send to 255.255.255.255 as fallback
    m_socket->writeDatagram(packet, QHostAddress::Broadcast, DISCOVERY_PORT);
}

void NetworkDiscovery::processIncomingPacket(const QByteArray& data, const QHostAddress& sender)
{
    DiscoveredDevice device = parsePacket(data, sender);
    
    if (!device.isValid()) {
        return;  // Invalid packet
    }
    
    // Ignore our own broadcasts
    bool isOwnBroadcast = false;
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip() == sender || 
                (sender.isLoopback()) ||
                (entry.ip().toString() == device.address)) {
                // Check if it's actually us by comparing name and code
                if (device.name == m_computerName && 
                    (device.pairingCode == m_pairingCode || m_pairingCode.isEmpty())) {
                    isOwnBroadcast = true;
                    break;
                }
            }
        }
        if (isOwnBroadcast) break;
    }
    
    if (isOwnBroadcast) {
        return;
    }
    
    // When listening for servers, only show servers
    // When broadcasting as server, we could potentially show clients looking to connect
    if (!m_isServer && !device.isServer) {
        return;  // Client doesn't care about other clients
    }
    
    // Check if device already exists
    QString key = device.address;
    bool isNew = !m_devices.contains(key);
    bool isUpdated = false;
    
    if (!isNew) {
        // Check if any info changed
        const auto& existing = m_devices[key];
        isUpdated = (existing.name != device.name || 
                     existing.pairingCode != device.pairingCode ||
                     existing.isServer != device.isServer);
    }
    
    // Update device info
    m_devices[key] = device;
    
    // Emit appropriate signal
    if (isNew) {
        emit deviceDiscovered(device);
    } else if (isUpdated) {
        emit deviceUpdated(device);
    }
}

QByteArray NetworkDiscovery::createBroadcastPacket() const
{
    // Format: REDKA|<version>|<name>|<mode>|<port>|<code>
    QString packet = QString("REDKA|%1|%2|%3|%4|%5")
        .arg(PROTOCOL_VERSION)
        .arg(m_computerName)
        .arg(m_isServer ? "SERVER" : "CLIENT")
        .arg(CONNECTION_PORT)
        .arg(m_pairingCode);
    
    return packet.toUtf8();
}

NetworkDiscovery::DiscoveredDevice NetworkDiscovery::parsePacket(const QByteArray& data, const QHostAddress& sender) const
{
    DiscoveredDevice device;
    
    QString packet = QString::fromUtf8(data);
    QStringList parts = packet.split('|');
    
    // Validate packet format
    if (parts.size() < 5 || parts[0] != "REDKA") {
        return device;  // Invalid
    }
    
    bool ok;
    int version = parts[1].toInt(&ok);
    if (!ok || version < 1) {
        return device;  // Invalid version
    }
    
    device.name = parts[2];
    device.isServer = (parts[3] == "SERVER");
    device.port = parts[4].toUShort(&ok);
    if (!ok) {
        device.port = CONNECTION_PORT;
    }
    
    // Pairing code (optional, may be empty for clients)
    if (parts.size() > 5) {
        device.pairingCode = parts[5];
    }
    
    // Get proper IP address
    if (sender.isLoopback()) {
        device.address = "127.0.0.1";
    } else if (sender.protocol() == QAbstractSocket::IPv4Protocol) {
        device.address = sender.toString();
    } else if (sender.protocol() == QAbstractSocket::IPv6Protocol) {
        // Try to get IPv4 representation if available
        bool conversionOk;
        QHostAddress ipv4 = QHostAddress(sender.toIPv4Address(&conversionOk));
        if (conversionOk && !ipv4.isNull()) {
            device.address = ipv4.toString();
        } else {
            device.address = sender.toString();
        }
    } else {
        device.address = sender.toString();
    }
    
    device.lastSeen = QDateTime::currentDateTime();
    
    return device;
}

QList<QHostAddress> NetworkDiscovery::getBroadcastAddresses() const
{
    QList<QHostAddress> addresses;
    
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        // Skip interfaces that are down or loopback
        if (!iface.flags().testFlag(QNetworkInterface::IsUp) ||
            !iface.flags().testFlag(QNetworkInterface::IsRunning) ||
            iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QHostAddress broadcast = entry.broadcast();
                if (!broadcast.isNull() && !addresses.contains(broadcast)) {
                    addresses.append(broadcast);
                }
            }
        }
    }
    
    return addresses;
}

void NetworkDiscovery::cleanupExpiredDevices()
{
    QStringList expiredKeys;
    
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        if (it.value().lastSeen.msecsTo(QDateTime::currentDateTime()) > m_deviceTimeoutMs) {
            expiredKeys.append(it.key());
        }
    }
    
    for (const QString& key : expiredKeys) {
        m_devices.remove(key);
        emit deviceLost(key);
    }
}
