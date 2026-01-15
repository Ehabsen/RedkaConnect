/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "PairingManager.h"

#include <QSettings>
#include <QUuid>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QCryptographicHash>

PairingManager::PairingManager(QObject* parent)
    : QObject(parent)
    , m_pinTimer(new QTimer(this))
{
    m_currentSession.isActive = false;
    
    connect(m_pinTimer, &QTimer::timeout, this, &PairingManager::onPinTimerTick);
    
    loadPairedDevices();
}

PairingManager::~PairingManager()
{
    savePairedDevices();
}

void PairingManager::initialize(const QString& deviceId, const QString& deviceName)
{
    m_deviceName = deviceName;
    
    // Load or generate device ID
    QSettings settings;
    m_deviceId = settings.value("device/id").toString();
    
    if (m_deviceId.isEmpty()) {
        m_deviceId = generateDeviceId();
        settings.setValue("device/id", m_deviceId);
    }
    
    // Override name if provided
    if (!deviceName.isEmpty()) {
        m_deviceName = deviceName;
        settings.setValue("device/name", deviceName);
    } else {
        m_deviceName = settings.value("device/name", "My Computer").toString();
    }
}

QString PairingManager::generatePairingPin()
{
    // Generate 6-digit PIN
    QString pin;
    for (int i = 0; i < PIN_LENGTH; ++i) {
        pin += QString::number(QRandomGenerator::global()->bounded(10));
    }
    
    // Setup session
    m_currentSession.pin = pin;
    m_currentSession.deviceId = m_deviceId;
    m_currentSession.deviceName = m_deviceName;
    m_currentSession.expiresAt = QDateTime::currentMSecsSinceEpoch() + (PIN_VALIDITY_SECONDS * 1000);
    m_currentSession.isActive = true;
    
    // Start expiry timer
    m_pinTimer->start(1000);
    
    emit pinGenerated(pin);
    return pin;
}

QString PairingManager::currentPin() const
{
    if (isPinValid()) {
        return m_currentSession.pin;
    }
    return QString();
}

bool PairingManager::isPinValid() const
{
    return m_currentSession.isActive && 
           QDateTime::currentMSecsSinceEpoch() < m_currentSession.expiresAt;
}

int PairingManager::pinTimeRemaining() const
{
    if (!m_currentSession.isActive) {
        return 0;
    }
    
    qint64 remaining = m_currentSession.expiresAt - QDateTime::currentMSecsSinceEpoch();
    return qMax(0, static_cast<int>(remaining / 1000));
}

bool PairingManager::verifyPin(const QString& pin, const QString& remoteDeviceId, 
                                const QString& remoteDeviceName)
{
    if (!isPinValid()) {
        emit pairingFailed("PIN has expired. Please generate a new one.");
        return false;
    }
    
    if (pin != m_currentSession.pin) {
        emit pairingFailed("Incorrect PIN. Please try again.");
        return false;
    }
    
    // PIN matches! Add device to paired list
    addPairedDevice(remoteDeviceId, remoteDeviceName);
    
    // Clear session
    cancelPairing();
    
    return true;
}

void PairingManager::cancelPairing()
{
    m_currentSession.isActive = false;
    m_currentSession.pin.clear();
    m_pinTimer->stop();
}

void PairingManager::onPinTimerTick()
{
    if (!isPinValid()) {
        cancelPairing();
        emit pinExpired();
    }
}

QString PairingManager::getQrCodeData() const
{
    if (!isPinValid()) {
        return QString();
    }
    
    // Get local IP address
    QString address;
    for (const auto& iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            for (const auto& entry : iface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    address = entry.ip().toString();
                    break;
                }
            }
        }
        if (!address.isEmpty()) break;
    }
    
    // Create JSON payload
    QJsonObject obj;
    obj["v"] = 1;                              // Version
    obj["id"] = m_deviceId;                    // Device ID
    obj["n"] = m_deviceName;                   // Name
    obj["p"] = m_currentSession.pin;           // PIN
    obj["a"] = address;                        // Address
    obj["t"] = m_currentSession.expiresAt;     // Expiry
    
    // Encode as compact JSON
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

bool PairingManager::parseQrCodeData(const QString& qrData, QString& deviceId,
                                      QString& deviceName, QString& pin, QString& address)
{
    QJsonDocument doc = QJsonDocument::fromJson(qrData.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    
    // Check version
    if (obj["v"].toInt() != 1) {
        return false;
    }
    
    // Check if expired
    qint64 expiry = obj["t"].toVariant().toLongLong();
    if (QDateTime::currentMSecsSinceEpoch() > expiry) {
        return false;
    }
    
    deviceId = obj["id"].toString();
    deviceName = obj["n"].toString();
    pin = obj["p"].toString();
    address = obj["a"].toString();
    
    return !deviceId.isEmpty() && !pin.isEmpty();
}

bool PairingManager::isDevicePaired(const QString& deviceId) const
{
    return m_pairedDevices.contains(deviceId);
}

PairingManager::PairedDevice PairingManager::getPairedDevice(const QString& deviceId) const
{
    return m_pairedDevices.value(deviceId);
}

QList<PairingManager::PairedDevice> PairingManager::pairedDevices() const
{
    return m_pairedDevices.values();
}

void PairingManager::forgetDevice(const QString& deviceId)
{
    if (m_pairedDevices.remove(deviceId) > 0) {
        savePairedDevices();
        emit deviceForgotten(deviceId);
    }
}

void PairingManager::updateDeviceSeen(const QString& deviceId, const QString& address)
{
    if (m_pairedDevices.contains(deviceId)) {
        m_pairedDevices[deviceId].lastAddress = address;
        m_pairedDevices[deviceId].lastSeen = QDateTime::currentDateTime();
        savePairedDevices();
    }
}

void PairingManager::addPairedDevice(const QString& deviceId, const QString& deviceName)
{
    PairedDevice device;
    device.id = deviceId;
    device.name = deviceName;
    device.pairedAt = QDateTime::currentDateTime();
    device.lastSeen = QDateTime::currentDateTime();
    device.isTrusted = true;
    
    m_pairedDevices[deviceId] = device;
    savePairedDevices();
    
    emit devicePaired(device);
}

void PairingManager::loadPairedDevices()
{
    QSettings settings;
    int count = settings.beginReadArray("pairedDevices");
    
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        
        PairedDevice device;
        device.id = settings.value("id").toString();
        device.name = settings.value("name").toString();
        device.lastAddress = settings.value("lastAddress").toString();
        device.pairedAt = settings.value("pairedAt").toDateTime();
        device.lastSeen = settings.value("lastSeen").toDateTime();
        device.isTrusted = settings.value("isTrusted", true).toBool();
        
        if (!device.id.isEmpty()) {
            m_pairedDevices[device.id] = device;
        }
    }
    
    settings.endArray();
}

void PairingManager::savePairedDevices()
{
    QSettings settings;
    settings.beginWriteArray("pairedDevices");
    
    int i = 0;
    for (const auto& device : m_pairedDevices) {
        settings.setArrayIndex(i++);
        settings.setValue("id", device.id);
        settings.setValue("name", device.name);
        settings.setValue("lastAddress", device.lastAddress);
        settings.setValue("pairedAt", device.pairedAt);
        settings.setValue("lastSeen", device.lastSeen);
        settings.setValue("isTrusted", device.isTrusted);
    }
    
    settings.endArray();
}

QString PairingManager::generateDeviceId()
{
    // Generate unique device ID from hardware info
    QString hwInfo;
    
    // Use MAC addresses
    for (const auto& iface : QNetworkInterface::allInterfaces()) {
        if (!iface.hardwareAddress().isEmpty() && 
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            hwInfo += iface.hardwareAddress();
        }
    }
    
    // Add some randomness
    hwInfo += QUuid::createUuid().toString();
    
    // Hash to create consistent ID
    QByteArray hash = QCryptographicHash::hash(hwInfo.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex().left(16);  // 16 char ID
}
