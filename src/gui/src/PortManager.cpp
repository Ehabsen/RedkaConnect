/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "PortManager.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QSettings>

PortManager::PortManager(QObject* parent)
    : QObject(parent)
    , m_selectedPort(DEFAULT_CONNECTION_PORT)
{
    // Load saved port preference
    QSettings settings;
    m_selectedPort = settings.value("network/port", DEFAULT_CONNECTION_PORT).toUInt();
}

PortManager::~PortManager() = default;

bool PortManager::isPortAvailable(quint16 port) const
{
    QTcpServer testServer;
    bool available = testServer.listen(QHostAddress::Any, port);
    if (available) {
        testServer.close();
    }
    return available;
}

quint16 PortManager::findAvailablePort(quint16 preferredPort) const
{
    // Try preferred port first
    if (isPortAvailable(preferredPort)) {
        return preferredPort;
    }
    
    // Try ports in our range
    for (quint16 port = PORT_RANGE_START; port <= PORT_RANGE_END; ++port) {
        if (port != preferredPort && isPortAvailable(port)) {
            return port;
        }
    }
    
    // No port available in our range
    return 0;
}

QList<PortManager::PortInfo> PortManager::scanPortRange() const
{
    QList<PortInfo> results;
    
    for (quint16 port = PORT_RANGE_START; port <= PORT_RANGE_END; ++port) {
        PortInfo info;
        info.port = port;
        info.isAvailable = isPortAvailable(port);
        info.usedBy = info.isAvailable ? "" : "Unknown";
        results.append(info);
    }
    
    return results;
}

bool PortManager::setPort(quint16 port)
{
    if (!isPortAvailable(port)) {
        emit portUnavailable(port);
        return false;
    }
    
    m_selectedPort = port;
    
    // Save preference
    QSettings settings;
    settings.setValue("network/port", port);
    
    emit portChanged(port);
    return true;
}

quint16 PortManager::autoSelectPort()
{
    quint16 port = findAvailablePort(m_selectedPort);
    if (port > 0) {
        m_selectedPort = port;
        emit portChanged(port);
    }
    return port;
}

QString PortManager::encodePortInCode(const QString& ipCode, quint16 port)
{
    // Port offset from default (0-99 range fits in 2 digits)
    int offset = port - DEFAULT_CONNECTION_PORT;
    
    // Clamp to valid range
    if (offset < 0) offset = 0;
    if (offset > 99) offset = 99;
    
    // The IP code is 6 digits (XXX-XXX format = XXXXXX)
    // We append 2 more digits for port offset
    // Format: XXXXXX-PP or XXX-XXXPP
    
    QString cleanCode = ipCode;
    cleanCode.remove('-');
    
    // Add port offset as last 2 digits
    return QString("%1%2").arg(cleanCode).arg(offset, 2, 10, QChar('0'));
}

quint16 PortManager::decodePortFromCode(const QString& code)
{
    QString cleanCode = code;
    cleanCode.remove('-');
    cleanCode.remove(' ');
    
    if (cleanCode.length() < 2) {
        return DEFAULT_CONNECTION_PORT;
    }
    
    // Last 2 digits are port offset
    bool ok;
    int offset = cleanCode.right(2).toInt(&ok);
    if (!ok) {
        // Try just last digit for backwards compatibility
        offset = cleanCode.right(1).toInt(&ok);
        if (!ok) offset = 0;
    }
    
    return DEFAULT_CONNECTION_PORT + offset;
}

QString PortManager::generateConnectionCode(int thirdOctet, int fourthOctet, quint16 port)
{
    // Encode IP octets (each 0-255, we use 3 digits each)
    // Format: TTT-FFFPP
    // T = third octet (000-255)
    // F = fourth octet (000-255)  
    // P = port offset (00-99)
    
    int portOffset = port - DEFAULT_CONNECTION_PORT;
    if (portOffset < 0) portOffset = 0;
    if (portOffset > 99) portOffset = 99;
    
    QString code = QString("%1%2%3")
        .arg(thirdOctet, 3, 10, QChar('0'))
        .arg(fourthOctet, 3, 10, QChar('0'))
        .arg(portOffset, 2, 10, QChar('0'));
    
    // Format as XXXX-XXXX for readability
    return code.left(4) + "-" + code.mid(4);
}

bool PortManager::parseConnectionCode(const QString& code, int& thirdOctet, int& fourthOctet, quint16& port)
{
    QString cleanCode = code;
    cleanCode.remove('-');
    cleanCode.remove(' ');
    cleanCode = cleanCode.toUpper();
    
    // Need at least 8 digits
    if (cleanCode.length() < 8) {
        // Try old 6-digit format for backwards compatibility
        if (cleanCode.length() >= 6) {
            bool ok1, ok2;
            thirdOctet = cleanCode.left(3).toInt(&ok1);
            fourthOctet = cleanCode.mid(3, 3).toInt(&ok2);
            port = DEFAULT_CONNECTION_PORT;
            return ok1 && ok2 && thirdOctet <= 255 && fourthOctet <= 255;
        }
        return false;
    }
    
    bool ok1, ok2, ok3;
    thirdOctet = cleanCode.left(3).toInt(&ok1);
    fourthOctet = cleanCode.mid(3, 3).toInt(&ok2);
    int portOffset = cleanCode.mid(6, 2).toInt(&ok3);
    
    if (!ok1 || !ok2 || !ok3) {
        return false;
    }
    
    if (thirdOctet > 255 || fourthOctet > 255) {
        return false;
    }
    
    port = DEFAULT_CONNECTION_PORT + portOffset;
    return true;
}
