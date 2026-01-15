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
#include <QList>

class QTcpServer;

/**
 * @brief Manages port selection and availability for RedkaConnect
 * 
 * Features:
 * - Auto-detects available ports
 * - Checks if ports are in use
 * - Suggests alternative ports if default is busy
 * - Encodes/decodes port in connection codes
 */
class PortManager : public QObject
{
    Q_OBJECT

public:
    // Default ports
    static constexpr quint16 DEFAULT_CONNECTION_PORT = 24800;
    static constexpr quint16 DEFAULT_DISCOVERY_PORT = 24801;
    
    // Port range for auto-selection
    static constexpr quint16 PORT_RANGE_START = 24800;
    static constexpr quint16 PORT_RANGE_END = 24899;

    struct PortInfo {
        quint16 port;
        bool isAvailable;
        QString usedBy;  // Process name if known
    };

    explicit PortManager(QObject* parent = nullptr);
    ~PortManager() override;

    /**
     * @brief Check if a specific port is available
     * @param port Port number to check
     * @return true if port can be bound
     */
    bool isPortAvailable(quint16 port) const;

    /**
     * @brief Find an available port starting from preferred port
     * @param preferredPort Port to try first
     * @return Available port, or 0 if none found
     */
    quint16 findAvailablePort(quint16 preferredPort = DEFAULT_CONNECTION_PORT) const;

    /**
     * @brief Get list of ports in our range with availability status
     * @return List of PortInfo structures
     */
    QList<PortInfo> scanPortRange() const;

    /**
     * @brief Get the currently selected port
     */
    quint16 selectedPort() const { return m_selectedPort; }

    /**
     * @brief Set the port to use
     * @return true if port is available and was set
     */
    bool setPort(quint16 port);

    /**
     * @brief Auto-select an available port
     * @return The selected port, or 0 if none available
     */
    quint16 autoSelectPort();

    /**
     * @brief Encode port into a connection code
     * 
     * Format: The last digit of the code encodes port offset from 24800
     * e.g., port 24805 -> offset 5 -> code ends with 5
     * 
     * @param ipCode The IP-based portion of the code (e.g., "192-168")
     * @param port The port to encode
     * @return Full connection code including port
     */
    static QString encodePortInCode(const QString& ipCode, quint16 port);

    /**
     * @brief Decode port from a connection code
     * @param code The full connection code
     * @return The encoded port number
     */
    static quint16 decodePortFromCode(const QString& code);

    /**
     * @brief Generate a full connection code with IP and port
     * @param thirdOctet Third octet of IP (0-255)
     * @param fourthOctet Fourth octet of IP (0-255)  
     * @param port Port number
     * @return Connection code in format "XXX-XXXY" where Y encodes port
     */
    static QString generateConnectionCode(int thirdOctet, int fourthOctet, quint16 port);

    /**
     * @brief Parse a connection code to extract IP octets and port
     * @param code The connection code
     * @param thirdOctet Output: third IP octet
     * @param fourthOctet Output: fourth IP octet
     * @param port Output: port number
     * @return true if parsing succeeded
     */
    static bool parseConnectionCode(const QString& code, int& thirdOctet, int& fourthOctet, quint16& port);

signals:
    void portChanged(quint16 newPort);
    void portUnavailable(quint16 port);

private:
    quint16 m_selectedPort;
};
