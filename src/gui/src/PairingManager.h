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
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QTimer>

/**
 * @brief AirDrop-style pairing manager
 * 
 * Handles device pairing with:
 * - 6-digit PIN codes (no hex strings!)
 * - QR code data generation
 * - Paired device memory
 * - Auto-reconnect for known devices
 * 
 * User Experience:
 * 1. Device A shows PIN: "847293"
 * 2. User selects Device A on Device B
 * 3. User enters PIN on Device B
 * 4. Devices are now paired forever
 */
class PairingManager : public QObject
{
    Q_OBJECT

public:
    struct PairedDevice {
        QString id;              // Unique device identifier
        QString name;            // Human-readable name
        QString lastAddress;     // Last known IP:port
        QDateTime lastSeen;      // When we last connected
        QDateTime pairedAt;      // When pairing occurred
        bool isTrusted;          // Has PIN been verified
    };

    struct PairingSession {
        QString pin;             // 6-digit PIN
        QString deviceId;        // Our device ID
        QString deviceName;      // Our device name
        qint64 expiresAt;        // When PIN expires (ms since epoch)
        bool isActive;
    };

    explicit PairingManager(QObject* parent = nullptr);
    ~PairingManager() override;

    /**
     * @brief Initialize with device info
     */
    void initialize(const QString& deviceId, const QString& deviceName);

    // ==================== PIN PAIRING ====================
    
    /**
     * @brief Generate a new 6-digit PIN for pairing
     * PIN expires after 5 minutes
     * @return The PIN code (e.g., "847293")
     */
    QString generatePairingPin();

    /**
     * @brief Get current active PIN (or empty if expired)
     */
    QString currentPin() const;

    /**
     * @brief Check if PIN is still valid
     */
    bool isPinValid() const;

    /**
     * @brief Time remaining until PIN expires (seconds)
     */
    int pinTimeRemaining() const;

    /**
     * @brief Verify a PIN entered by remote device
     * @param pin The PIN to verify
     * @param remoteDeviceId The remote device's ID
     * @param remoteDeviceName The remote device's name
     * @return true if PIN matches and device is now trusted
     */
    bool verifyPin(const QString& pin, const QString& remoteDeviceId, 
                   const QString& remoteDeviceName);

    /**
     * @brief Cancel current pairing session
     */
    void cancelPairing();

    // ==================== QR CODE ====================
    
    /**
     * @brief Get data to encode in QR code
     * Contains: device ID, name, PIN, address
     * @return JSON string for QR encoding
     */
    QString getQrCodeData() const;

    /**
     * @brief Parse QR code data from another device
     * @param qrData The scanned QR data
     * @param deviceId Output: remote device ID
     * @param deviceName Output: remote device name
     * @param pin Output: the PIN to use
     * @param address Output: connection address
     * @return true if parsing succeeded
     */
    static bool parseQrCodeData(const QString& qrData, QString& deviceId,
                                QString& deviceName, QString& pin, QString& address);

    // ==================== PAIRED DEVICES ====================
    
    /**
     * @brief Check if a device is already paired/trusted
     */
    bool isDevicePaired(const QString& deviceId) const;

    /**
     * @brief Get info about a paired device
     */
    PairedDevice getPairedDevice(const QString& deviceId) const;

    /**
     * @brief Get all paired devices
     */
    QList<PairedDevice> pairedDevices() const;

    /**
     * @brief Forget/unpair a device
     */
    void forgetDevice(const QString& deviceId);

    /**
     * @brief Update last seen info for a device
     */
    void updateDeviceSeen(const QString& deviceId, const QString& address);

    /**
     * @brief Get our device ID
     */
    QString deviceId() const { return m_deviceId; }

    /**
     * @brief Get our device name
     */
    QString deviceName() const { return m_deviceName; }

signals:
    void pinGenerated(const QString& pin);
    void pinExpired();
    void devicePaired(const PairedDevice& device);
    void deviceForgotten(const QString& deviceId);
    void pairingFailed(const QString& reason);

private slots:
    void onPinTimerTick();

private:
    void loadPairedDevices();
    void savePairedDevices();
    QString generateDeviceId();
    void addPairedDevice(const QString& deviceId, const QString& deviceName);

    QString m_deviceId;
    QString m_deviceName;
    PairingSession m_currentSession;
    QMap<QString, PairedDevice> m_pairedDevices;
    QTimer* m_pinTimer;
    
    static constexpr int PIN_VALIDITY_SECONDS = 300;  // 5 minutes
    static constexpr int PIN_LENGTH = 6;
};
