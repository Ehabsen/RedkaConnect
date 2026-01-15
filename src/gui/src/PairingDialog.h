/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QPushButton;
class QLineEdit;
class QStackedWidget;
class QTimer;
class GlassPanel;
class QRCodeWidget;
class PairingManager;

/**
 * @brief AirDrop-style pairing dialog
 * 
 * Provides a simple, friendly pairing experience:
 * - Shows large PIN code
 * - Displays QR code for easy scanning
 * - Countdown timer for PIN expiry
 * - No technical jargon whatsoever
 */
class PairingDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode {
        ShowPin,    // We're sharing - show our PIN
        EnterPin    // We're connecting - enter their PIN
    };

    explicit PairingDialog(PairingManager* pairingManager, Mode mode, QWidget* parent = nullptr);
    ~PairingDialog() override;

    /**
     * @brief Set the device we're pairing with (for EnterPin mode)
     */
    void setRemoteDevice(const QString& deviceId, const QString& deviceName);

    /**
     * @brief Get the entered PIN (for EnterPin mode)
     */
    QString enteredPin() const;

    /**
     * @brief Check if pairing was successful
     */
    bool pairingSuccessful() const { return m_pairingSuccessful; }

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onTimerTick();
    void onPinDigitEntered(const QString& text);
    void onVerifyClicked();
    void onCancelClicked();

private:
    void setupShowPinUi();
    void setupEnterPinUi();
    void updateCountdown();
    void applyStylesheet();

    Mode m_mode;
    PairingManager* m_pairingManager;
    
    // UI - Show PIN mode
    QLabel* m_pinLabel;
    QLabel* m_countdownLabel;
    QRCodeWidget* m_qrCode;
    
    // UI - Enter PIN mode
    QLineEdit* m_pinInput[6];
    QLabel* m_deviceNameLabel;
    QPushButton* m_verifyButton;
    
    // Common
    QPushButton* m_cancelButton;
    QTimer* m_timer;
    
    // State
    QString m_remoteDeviceId;
    QString m_remoteDeviceName;
    bool m_pairingSuccessful;
};
