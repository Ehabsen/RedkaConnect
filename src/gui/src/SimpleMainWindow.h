/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <memory>

class QLabel;
class QPushButton;
class QLineEdit;
class QSpinBox;
class QStackedWidget;
class QFrame;
class QScrollArea;
class ScreenArrangementWidget;
class DeviceListWidget;
class NetworkDiscovery;
class SecurityManager;
class PortManager;
class AppConfig;

// Forward declarations
struct DiscoveredDevice;  // From DeviceListWidget.h

namespace Ui {
    class SimpleMainWindow;
}

/**
 * @brief Simplified, modern main window for RedkaConnect
 * 
 * Design Philosophy:
 * - No server/client distinction visible to user
 * - One-click connection via pairing codes
 * - Visual drag-and-drop screen arrangement
 * - Minimal settings, maximum usability
 */
class SimpleMainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(double glowIntensity READ glowIntensity WRITE setGlowIntensity)

public:
    enum class ConnectionState {
        Disconnected,
        Waiting,      // Waiting for peer to connect
        Connecting,   // Actively connecting to peer
        Connected
    };
    Q_ENUM(ConnectionState)

    enum class TransferMode {
        EdgeTransfer,   // Mouse moves to screen edge
        HotkeyTransfer  // Press hotkey to switch
    };
    Q_ENUM(TransferMode)

    enum class ConnectionType {
        Network,
        USB
    };
    Q_ENUM(ConnectionType)

    explicit SimpleMainWindow(QSettings& settings, AppConfig& appConfig, QWidget* parent = nullptr);
    ~SimpleMainWindow() override;

    // Property for glow animation
    double glowIntensity() const { return m_glowIntensity; }
    void setGlowIntensity(double intensity);

public slots:
    void showWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onShareClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onSettingsClicked();
    void onCodeEntered();
    void onManualConnectClicked();
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void updateConnectionStatus();
    void copyCodeToClipboard();
    void onScreenArrangementChanged();
    void onTransferModeChanged(int index);
    void onDeviceSelected(const DiscoveredDevice& device);
    void onNetworkDeviceDiscovered(const NetworkDiscovery::DiscoveredDevice& device);
    void onNetworkDeviceLost(const QString& address);
    void onNetworkError(const QString& error);
    void refreshDeviceList();

    // USB slots
    void onUSBConnectClicked();
    void onUSBDeviceSelected();
    void onUSBDeviceDiscovered(const USBConnectionManager::USBDevice& device);
    void onUSBDeviceRemoved(const QString& portName);
    void onUSBConnected(const QString& portName);
    void onUSBDisconnected();
    void onUSBError(const QString& error);
    void onUSBRefreshClicked();
    void onUSBBackClicked();

private:
    void setupUi();
    void setupTrayIcon();
    void setupAnimations();
    void applyStylesheet();
    
    void switchToShareMode();
    void switchToConnectMode();
    void switchToConnectedMode();
    void switchToSettingsMode();
    
    void startServer();
    void startClient(const QString& address);
    void stopProcess();
    
    QString generatePairingCode();
    QString codeToAddress(const QString& code);
    bool validateCode(const QString& code);
    
    void saveSettings();
    void loadSettings();
    
    void setState(ConnectionState state);
    void animateStateChange();
    void pulseGlow();

    // UI Components
    QStackedWidget* m_stackedWidget;
    
    // Home page
    QWidget* m_homePage;
    QLabel* m_logoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_shareButton;
    QPushButton* m_connectButton;
    QPushButton* m_settingsButton;

    // Status display widgets (skeuomorphic)
    QLabel* m_statusMonitorIcon;
    QLabel* m_statusCableIcon;
    QLabel* m_statusTitleLabel;
    QLabel* m_statusMessageLabel;
    
    // Share page (waiting for connection)
    QWidget* m_sharePage;
    QLabel* m_codeLabel;
    QLabel* m_codeValueLabel;
    QPushButton* m_copyCodeButton;
    QPushButton* m_cancelShareButton;
    QLabel* m_waitingLabel;
    
    // Connect page (auto-discovery + manual)
    QWidget* m_connectPage;
    QLabel* m_connectTitle;
    DeviceListWidget* m_deviceList;
    QPushButton* m_manualConnectButton;
    QPushButton* m_cancelConnectButton;
    QTimer* m_discoveryTimer;
    
    // Manual connect page (enter code)
    QWidget* m_manualConnectPage;
    QLabel* m_enterCodeLabel;
    QLineEdit* m_codeInput;
    QPushButton* m_goButton;
    QPushButton* m_backToConnectButton;

    // USB connect page (cable connection)
    QWidget* m_usbConnectPage;
    QLabel* m_usbTitle;
    QLabel* m_usbInstructions;
    QListWidget* m_usbDeviceList;
    QPushButton* m_usbConnectButton;
    QPushButton* m_usbRefreshButton;
    QPushButton* m_backToHomeFromUSB;
    
    // Connected page
    QWidget* m_connectedPage;
    QLabel* m_connectedLabel;
    QLabel* m_peerNameLabel;
    ScreenArrangementWidget* m_screenArrangement;
    QPushButton* m_disconnectButton;
    
    // Settings page
    QWidget* m_settingsPage;
    QLabel* m_settingsTitle;
    QFrame* m_transferModeFrame;
    QFrame* m_hotkeyFrame;
    QFrame* m_computerNameFrame;
    QPushButton* m_backButton;

    // Error page (unplugged cable)
    QWidget* m_errorPage;
    QLabel* m_errorIconLabel;
    QLabel* m_errorTitleLabel;
    QLabel* m_errorMessageLabel;
    QPushButton* m_reconnectButton;
    
    // System tray
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    // State
    ConnectionState m_state;
    TransferMode m_transferMode;
    ConnectionType m_connectionType;
    QString m_computerName;
    QString m_currentCode;
    QString m_peerName;
    QString m_peerAddress;
    
    // Process
    QProcess* m_process;
    bool m_isServer;
    
    // Network Discovery
    NetworkDiscovery* m_networkDiscovery;

    // USB Connection
    USBConnectionManager* m_usbManager;
    
    // Security & Port Management
    SecurityManager* m_securityManager;
    PortManager* m_portManager;
    QLabel* m_fingerprintLabel;
    QSpinBox* m_portSpinBox;
    
    // Settings
    QSettings& m_settings;
    AppConfig& m_appConfig;
    
    // Animation
    QPropertyAnimation* m_glowAnimation;
    double m_glowIntensity;
    QGraphicsDropShadowEffect* m_glowEffect;
    QTimer* m_pulseTimer;
};
