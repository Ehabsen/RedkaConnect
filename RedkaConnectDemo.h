/*
 * RedkaConnect Demo -- Showcase of new UI features
 * Based on InputLeap
 *
 * This demo shows:
 * - Skeuomorphic interface with monitor icons
 * - QR code generation and clipboard paste
 * - USB connection metaphors
 * - Cable error handling
 */

#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

class QLabel;
class QPushButton;
class GlassPanel;

// Demo class to showcase RedkaConnect features
class RedkaConnectDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit RedkaConnectDemo(QWidget* parent = nullptr);
    ~RedkaConnectDemo() override;

private slots:
    void onShareClicked();
    void onConnectClicked();
    void onUSBClicked();
    void onSettingsClicked();
    void onBackToHomeClicked();
    void onTimerTick();
    void onSimulateErrorClicked();
    void onPlugBackInClicked();

private:
    void setupUi();
    void setupHomePage();
    void setupSharePage();
    void setupConnectPage();
    void setupUSBPage();
    void setupErrorPage();
    void applyStylesheet();

    // UI Components
    QStackedWidget* m_stackedWidget;

    // Home page
    QWidget* m_homePage;
    QLabel* m_logoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_shareButton;
    QPushButton* m_connectButton;
    QPushButton* m_usbButton;
    QPushButton* m_settingsButton;

    // Share page (simulated)
    QWidget* m_sharePage;
    QLabel* m_codeLabel;
    QLabel* m_codeValueLabel;
    QPushButton* m_copyCodeButton;
    QPushButton* m_cancelShareButton;
    QLabel* m_waitingLabel;

    // Connect page (simulated)
    QWidget* m_connectPage;
    QLabel* m_connectTitle;
    QPushButton* m_manualConnectButton;
    QPushButton* m_cancelConnectButton;

    // USB page (simulated)
    QWidget* m_usbPage;
    QLabel* m_usbTitle;
    QLabel* m_usbInstructions;
    QListWidget* m_usbDeviceList;
    QPushButton* m_usbConnectButton;
    QPushButton* m_usbRefreshButton;
    QPushButton* m_backToHomeFromUSB;

    // Error page (cable unplugged)
    QWidget* m_errorPage;
    QLabel* m_errorIconLabel;
    QLabel* m_errorTitleLabel;
    QLabel* m_errorMessageLabel;
    QPushButton* m_reconnectButton;

    // Demo state
    QTimer* m_demoTimer;
    int m_demoStep;
    QString m_currentCode;
};