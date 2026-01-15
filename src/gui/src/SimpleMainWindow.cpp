/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "SimpleMainWindow.h"
#include "ScreenArrangementWidget.h"
#include "DeviceListWidget.h"
#include "NetworkDiscovery.h"
#include "SecurityManager.h"
#include "PortManager.h"
#include "AnimatedBackground.h"
#include "GlassPanel.h"
#include "AppConfig.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QEasingCurve>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QProcess>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

SimpleMainWindow::SimpleMainWindow(QSettings& settings, AppConfig& appConfig, QWidget* parent)
    : QMainWindow(parent)
    , m_settings(settings)
    , m_appConfig(appConfig)
    , m_state(ConnectionState::Disconnected)
    , m_transferMode(TransferMode::EdgeTransfer)
    , m_process(nullptr)
    , m_isServer(false)
    , m_glowIntensity(0.0)
    , m_glowEffect(nullptr)
    , m_pulseTimer(new QTimer(this))
    , m_discoveryTimer(new QTimer(this))
    , m_networkDiscovery(new NetworkDiscovery(this))
    , m_usbManager(new USBConnectionManager(this))
    , m_securityManager(new SecurityManager(this))
    , m_portManager(new PortManager(this))
    , m_fingerprintLabel(nullptr)
    , m_portSpinBox(nullptr)
{
    // Initialize security (auto-generates certificates if needed)
    m_securityManager->initialize(m_computerName);
    
    // Auto-select available port
    m_portManager->autoSelectPort();
    
    // Connect network discovery signals
    connect(m_networkDiscovery, &NetworkDiscovery::deviceDiscovered,
            this, &SimpleMainWindow::onNetworkDeviceDiscovered);
    connect(m_networkDiscovery, &NetworkDiscovery::deviceLost,
            this, &SimpleMainWindow::onNetworkDeviceLost);
    connect(m_networkDiscovery, &NetworkDiscovery::deviceUpdated,
            this, &SimpleMainWindow::onNetworkDeviceDiscovered);
    connect(m_networkDiscovery, &NetworkDiscovery::errorOccurred,
            this, &SimpleMainWindow::onNetworkError);

    // Connect USB manager signals
    connect(m_usbManager, &USBConnectionManager::deviceDiscovered,
            this, &SimpleMainWindow::onUSBDeviceDiscovered);
    connect(m_usbManager, &USBConnectionManager::deviceRemoved,
            this, &SimpleMainWindow::onUSBDeviceRemoved);
    connect(m_usbManager, &USBConnectionManager::connected,
            this, &SimpleMainWindow::onUSBConnected);
    connect(m_usbManager, &USBConnectionManager::disconnected,
            this, &SimpleMainWindow::onUSBDisconnected);
    connect(m_usbManager, &USBConnectionManager::errorOccurred,
            this, &SimpleMainWindow::onUSBError);

    // Get computer name
    m_computerName = settings.value("computerName", QHostInfo::localHostName()).toString();
    if (m_computerName.isEmpty()) {
        m_computerName = "This PC";
    }
    
    setWindowTitle("RedkaConnect");
    setFixedSize(520, 700);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    
    setupUi();
    setupTrayIcon();
    setupAnimations();
    applyStylesheet();
    loadSettings();
    
    setState(ConnectionState::Disconnected);
}

SimpleMainWindow::~SimpleMainWindow()
{
    stopProcess();
}

void SimpleMainWindow::setupUi()
{
    // Create central widget with animated background
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Animated background layer
    AnimatedBackground* background = new AnimatedBackground(centralWidget);
    background->setStyle(AnimatedBackground::Style::Particles);
    background->setColors(
        QColor(8, 12, 21),       // Deep navy
        QColor(15, 23, 42),      // Slate
        QColor(6, 182, 212)      // Cyan accent
    );
    background->setParticleCount(40);
    background->setSpeed(0.8f);
    background->start();
    
    // Main layout over background
    QVBoxLayout* bgLayout = new QVBoxLayout(centralWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);
    
    // Make background fill the widget
    background->setGeometry(centralWidget->rect());
    background->lower();
    
    // Content container (sits on top of background)
    QWidget* contentWidget = new QWidget();
    contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    bgLayout->addWidget(contentWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(0);
    
    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->setAttribute(Qt::WA_TranslucentBackground);
    mainLayout->addWidget(m_stackedWidget);
    
    // Ensure background resizes with window
    connect(this, &QMainWindow::windowTitleChanged, this, [background, centralWidget]() {
        background->setGeometry(centralWidget->rect());
    });
    
    // ==================== HOME PAGE ====================
    m_homePage = new QWidget();
    m_homePage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* homeLayout = new QVBoxLayout(m_homePage);
    homeLayout->setContentsMargins(16, 32, 16, 24);
    homeLayout->setSpacing(16);
    
    // Logo with glow effect
    m_logoLabel = new QLabel();
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setText("⚡");
    m_logoLabel->setStyleSheet("font-size: 72px;");
    
    QGraphicsDropShadowEffect* logoGlow = new QGraphicsDropShadowEffect();
    logoGlow->setBlurRadius(40);
    logoGlow->setColor(QColor(6, 182, 212, 150));
    logoGlow->setOffset(0, 0);
    m_logoLabel->setGraphicsEffect(logoGlow);
    homeLayout->addWidget(m_logoLabel);
    
    // Title
    QLabel* titleLabel = new QLabel("RedkaConnect");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    homeLayout->addWidget(titleLabel);
    
    // Subtitle
    QLabel* subtitleLabel = new QLabel("Seamless keyboard & mouse sharing");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setObjectName("subtitleLabel");
    homeLayout->addWidget(subtitleLabel);
    
    homeLayout->addSpacing(24);
    
    // Skeuomorphic status display (monitor + cable)
    GlassPanel* statusPanel = new GlassPanel();
    statusPanel->setFixedHeight(64);
    statusPanel->setBorderRadius(20);
    statusPanel->setGlowColor(QColor(16, 185, 129, 80));
    QHBoxLayout* statusLayout = new QHBoxLayout(statusPanel);
    statusLayout->setContentsMargins(20, 8, 20, 8);
    statusLayout->setSpacing(12);

    // Monitor icon (realistic looking monitor)
    m_statusMonitorIcon = new QLabel();
    m_statusMonitorIcon->setText("🖥️");  // Computer monitor emoji
    m_statusMonitorIcon->setStyleSheet("font-size: 24px;");
    m_statusMonitorIcon->setFixedSize(32, 32);
    statusLayout->addWidget(m_statusMonitorIcon);

    // Cable connection icon
    m_statusCableIcon = new QLabel();
    m_statusCableIcon->setText("🔗");  // Link/cable emoji
    m_statusCableIcon->setStyleSheet("color: #10b981; font-size: 16px;");
    statusLayout->addWidget(m_statusCableIcon);

    // Status text
    QVBoxLayout* statusTextLayout = new QVBoxLayout();
    statusTextLayout->setContentsMargins(0, 0, 0, 0);
    statusTextLayout->setSpacing(2);

    m_statusTitleLabel = new QLabel("Ready to Connect");
    m_statusTitleLabel->setObjectName("statusTitle");
    statusTextLayout->addWidget(m_statusTitleLabel);

    m_statusMessageLabel = new QLabel("Monitor and cable are ready");
    m_statusMessageLabel->setObjectName("statusMessage");
    statusTextLayout->addWidget(m_statusMessageLabel);

    statusLayout->addLayout(statusTextLayout);
    statusLayout->addStretch();

    homeLayout->addWidget(statusPanel);
    
    homeLayout->addSpacing(24);
    
    // Main buttons in glass panels
    GlassPanel* sharePanel = new GlassPanel();
    sharePanel->setGlowColor(QColor(6, 182, 212, 100));
    sharePanel->setGlowIntensity(0.6);
    QVBoxLayout* sharePanelLayout = new QVBoxLayout(sharePanel);
    sharePanelLayout->setContentsMargins(24, 20, 24, 20);
    
    m_shareButton = new QPushButton("Share This Computer");
    m_shareButton->setObjectName("primaryButton");
    m_shareButton->setCursor(Qt::PointingHandCursor);
    m_shareButton->setMinimumHeight(56);
    m_shareButton->setToolTip("Let other computers control this one (Ctrl+S)");
    m_shareButton->setShortcut(QKeySequence("Ctrl+S"));
    connect(m_shareButton, &QPushButton::clicked, this, &SimpleMainWindow::onShareClicked);
    sharePanelLayout->addWidget(m_shareButton);
    
    QLabel* shareHint = new QLabel("📤 Others will use your keyboard & mouse");
    shareHint->setObjectName("buttonHint");
    shareHint->setAlignment(Qt::AlignCenter);
    sharePanelLayout->addWidget(shareHint);
    
    homeLayout->addWidget(sharePanel);
    
    homeLayout->addSpacing(12);
    
    GlassPanel* connectPanel = new GlassPanel();
    connectPanel->setGlowColor(QColor(139, 92, 246, 80));
    connectPanel->setGlowIntensity(0.4);
    QVBoxLayout* connectPanelLayout = new QVBoxLayout(connectPanel);
    connectPanelLayout->setContentsMargins(24, 20, 24, 20);
    
    m_connectButton = new QPushButton("Connect to Computer");
    m_connectButton->setObjectName("secondaryButton");
    m_connectButton->setCursor(Qt::PointingHandCursor);
    m_connectButton->setMinimumHeight(56);
    m_connectButton->setToolTip("Use another computer's keyboard & mouse (Ctrl+C)");
    m_connectButton->setShortcut(QKeySequence("Ctrl+J"));  // J for Join
    connect(m_connectButton, &QPushButton::clicked, this, &SimpleMainWindow::onConnectClicked);
    connectPanelLayout->addWidget(m_connectButton);
    
    QLabel* connectHint = new QLabel("📥 Control this PC from another");
    connectHint->setObjectName("buttonHint");
    connectHint->setAlignment(Qt::AlignCenter);
    connectPanelLayout->addWidget(connectHint);
    
    homeLayout->addWidget(connectPanel);

    homeLayout->addSpacing(12);

    // USB connection panel
    GlassPanel* usbPanel = new GlassPanel();
    usbPanel->setGlowColor(QColor(34, 197, 94, 80));  // Green for USB
    usbPanel->setGlowIntensity(0.4);
    QVBoxLayout* usbPanelLayout = new QVBoxLayout(usbPanel);
    usbPanelLayout->setContentsMargins(24, 20, 24, 20);

    m_usbConnectButton = new QPushButton("Connect via USB Cable");
    m_usbConnectButton->setObjectName("tertiaryButton");
    m_usbConnectButton->setCursor(Qt::PointingHandCursor);
    m_usbConnectButton->setMinimumHeight(56);
    m_usbConnectButton->setToolTip("Connect directly with a USB cable (Ctrl+U)");
    m_usbConnectButton->setShortcut(QKeySequence("Ctrl+U"));
    connect(m_usbConnectButton, &QPushButton::clicked, this, &SimpleMainWindow::onUSBConnectClicked);
    usbPanelLayout->addWidget(m_usbConnectButton);

    QLabel* usbHint = new QLabel("🔌 Direct USB cable connection");
    usbHint->setObjectName("buttonHint");
    usbHint->setAlignment(Qt::AlignCenter);
    usbPanelLayout->addWidget(usbHint);

    homeLayout->addWidget(usbPanel);

    homeLayout->addStretch();
    
    // Settings button
    m_settingsButton = new QPushButton("⚙  Settings");
    m_settingsButton->setObjectName("linkButton");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setShortcut(QKeySequence("Ctrl+,"));
    connect(m_settingsButton, &QPushButton::clicked, this, &SimpleMainWindow::onSettingsClicked);
    homeLayout->addWidget(m_settingsButton, 0, Qt::AlignCenter);
    
    m_stackedWidget->addWidget(m_homePage);
    
    // ==================== SHARE PAGE ====================
    m_sharePage = new QWidget();
    m_sharePage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* shareLayout = new QVBoxLayout(m_sharePage);
    shareLayout->setContentsMargins(16, 24, 16, 24);
    shareLayout->setSpacing(20);
    
    // Back button
    QPushButton* backFromShare = new QPushButton("← Back");
    backFromShare->setObjectName("linkButton");
    backFromShare->setCursor(Qt::PointingHandCursor);
    connect(backFromShare, &QPushButton::clicked, this, &SimpleMainWindow::onDisconnectClicked);
    shareLayout->addWidget(backFromShare, 0, Qt::AlignLeft);
    
    m_waitingLabel = new QLabel("Waiting for connection...");
    m_waitingLabel->setAlignment(Qt::AlignCenter);
    m_waitingLabel->setObjectName("pageTitle");
    shareLayout->addWidget(m_waitingLabel);
    
    // Animated icon
    QLabel* waitingAnim = new QLabel("📡");
    waitingAnim->setAlignment(Qt::AlignCenter);
    waitingAnim->setStyleSheet("font-size: 80px;");
    QGraphicsDropShadowEffect* animGlow = new QGraphicsDropShadowEffect();
    animGlow->setBlurRadius(50);
    animGlow->setColor(QColor(6, 182, 212, 120));
    animGlow->setOffset(0, 0);
    waitingAnim->setGraphicsEffect(animGlow);
    shareLayout->addWidget(waitingAnim);
    
    shareLayout->addSpacing(8);
    
    m_codeLabel = new QLabel("Your connection code:");
    m_codeLabel->setAlignment(Qt::AlignCenter);
    m_codeLabel->setObjectName("labelText");
    shareLayout->addWidget(m_codeLabel);
    
    // Code display in glass panel
    GlassPanel* codePanel = new GlassPanel();
    codePanel->setGlowColor(QColor(6, 182, 212, 120));
    codePanel->setGlowIntensity(0.8);
    codePanel->setBorderRadius(24);
    QHBoxLayout* codeLayout = new QHBoxLayout(codePanel);
    codeLayout->setContentsMargins(32, 24, 24, 24);
    
    m_codeValueLabel = new QLabel("000-000");
    m_codeValueLabel->setObjectName("codeValue");
    m_codeValueLabel->setAlignment(Qt::AlignCenter);
    codeLayout->addWidget(m_codeValueLabel, 1);
    
    m_copyCodeButton = new QPushButton("📋");
    m_copyCodeButton->setObjectName("iconButton");
    m_copyCodeButton->setCursor(Qt::PointingHandCursor);
    m_copyCodeButton->setToolTip("Copy to clipboard");
    m_copyCodeButton->setFixedSize(48, 48);
    connect(m_copyCodeButton, &QPushButton::clicked, this, &SimpleMainWindow::copyCodeToClipboard);
    codeLayout->addWidget(m_copyCodeButton);
    
    shareLayout->addWidget(codePanel);
    
    QLabel* instructionLabel = new QLabel("Enter this code on the other computer,\nor they'll appear automatically nearby");
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setObjectName("hintLabel");
    instructionLabel->setWordWrap(true);
    shareLayout->addWidget(instructionLabel);
    
    shareLayout->addStretch();
    
    m_cancelShareButton = new QPushButton("Stop Sharing");
    m_cancelShareButton->setObjectName("dangerButton");
    m_cancelShareButton->setCursor(Qt::PointingHandCursor);
    m_cancelShareButton->setMinimumHeight(52);
    connect(m_cancelShareButton, &QPushButton::clicked, this, &SimpleMainWindow::onDisconnectClicked);
    shareLayout->addWidget(m_cancelShareButton);
    
    m_stackedWidget->addWidget(m_sharePage);
    
    // ==================== CONNECT PAGE ====================
    m_connectPage = new QWidget();
    m_connectPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* connectLayout = new QVBoxLayout(m_connectPage);
    connectLayout->setContentsMargins(16, 24, 16, 24);
    connectLayout->setSpacing(16);
    
    // Header
    m_cancelConnectButton = new QPushButton("← Back");
    m_cancelConnectButton->setObjectName("linkButton");
    m_cancelConnectButton->setCursor(Qt::PointingHandCursor);
    connect(m_cancelConnectButton, &QPushButton::clicked, this, [this]() {
        m_discoveryTimer->stop();
        m_networkDiscovery->stop();
        m_stackedWidget->setCurrentWidget(m_homePage);
    });
    connectLayout->addWidget(m_cancelConnectButton, 0, Qt::AlignLeft);
    
    m_connectTitle = new QLabel("Available Computers");
    m_connectTitle->setObjectName("pageTitle");
    connectLayout->addWidget(m_connectTitle);
    
    QLabel* connectSubtitle = new QLabel("Computers sharing on your network");
    connectSubtitle->setObjectName("subtitleLabel");
    connectLayout->addWidget(connectSubtitle);
    
    connectLayout->addSpacing(8);
    
    // Device list in glass panel
    GlassPanel* devicePanel = new GlassPanel();
    devicePanel->setGlowIntensity(0.3);
    QVBoxLayout* devicePanelLayout = new QVBoxLayout(devicePanel);
    devicePanelLayout->setContentsMargins(8, 8, 8, 8);
    
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");
    scrollArea->viewport()->setStyleSheet("background: transparent;");
    
    m_deviceList = new DeviceListWidget();
    connect(m_deviceList, &DeviceListWidget::deviceSelected, 
            this, &SimpleMainWindow::onDeviceSelected);
    scrollArea->setWidget(m_deviceList);
    
    devicePanelLayout->addWidget(scrollArea);
    connectLayout->addWidget(devicePanel, 1);
    
    // Manual connect option
    GlassPanel* manualPanel = new GlassPanel();
    manualPanel->setGlowIntensity(0.2);
    manualPanel->setBorderRadius(16);
    QVBoxLayout* manualLayout = new QVBoxLayout(manualPanel);
    manualLayout->setContentsMargins(16, 12, 16, 12);
    
    QLabel* manualLabel = new QLabel("Can't find your computer?");
    manualLabel->setObjectName("hintLabel");
    manualLabel->setAlignment(Qt::AlignCenter);
    manualLayout->addWidget(manualLabel);
    
    m_manualConnectButton = new QPushButton("Enter Code Manually →");
    m_manualConnectButton->setObjectName("textButton");
    m_manualConnectButton->setCursor(Qt::PointingHandCursor);
    connect(m_manualConnectButton, &QPushButton::clicked, 
            this, &SimpleMainWindow::onManualConnectClicked);
    manualLayout->addWidget(m_manualConnectButton, 0, Qt::AlignCenter);
    
    connectLayout->addWidget(manualPanel);
    
    m_stackedWidget->addWidget(m_connectPage);
    
    // ==================== MANUAL CONNECT PAGE ====================
    m_manualConnectPage = new QWidget();
    m_manualConnectPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* manualConnectLayout = new QVBoxLayout(m_manualConnectPage);
    manualConnectLayout->setContentsMargins(16, 24, 16, 24);
    manualConnectLayout->setSpacing(20);
    
    m_backToConnectButton = new QPushButton("← Back");
    m_backToConnectButton->setObjectName("linkButton");
    m_backToConnectButton->setCursor(Qt::PointingHandCursor);
    connect(m_backToConnectButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_connectPage);
    });
    manualConnectLayout->addWidget(m_backToConnectButton, 0, Qt::AlignLeft);
    
    QLabel* manualIcon = new QLabel("🔗");
    manualIcon->setAlignment(Qt::AlignCenter);
    manualIcon->setStyleSheet("font-size: 72px;");
    manualConnectLayout->addWidget(manualIcon);
    
    m_enterCodeLabel = new QLabel("Enter connection code");
    m_enterCodeLabel->setAlignment(Qt::AlignCenter);
    m_enterCodeLabel->setObjectName("pageTitle");
    manualConnectLayout->addWidget(m_enterCodeLabel);
    
    manualConnectLayout->addSpacing(8);
    
    // Code input in glass panel
    GlassPanel* inputPanel = new GlassPanel();
    inputPanel->setGlowColor(QColor(139, 92, 246, 80));
    inputPanel->setBorderRadius(20);
    QVBoxLayout* inputPanelLayout = new QVBoxLayout(inputPanel);
    inputPanelLayout->setContentsMargins(24, 24, 24, 24);
    
    m_codeInput = new QLineEdit();
    m_codeInput->setObjectName("codeInput");
    m_codeInput->setPlaceholderText("000-000");
    m_codeInput->setAlignment(Qt::AlignCenter);
    m_codeInput->setMaxLength(7);
    m_codeInput->setMinimumHeight(72);
    connect(m_codeInput, &QLineEdit::returnPressed, this, &SimpleMainWindow::onCodeEntered);
    inputPanelLayout->addWidget(m_codeInput);
    
    QLabel* formatHint = new QLabel("Get the code from the sharing computer");
    formatHint->setAlignment(Qt::AlignCenter);
    formatHint->setObjectName("hintLabel");
    inputPanelLayout->addWidget(formatHint);
    
    manualConnectLayout->addWidget(inputPanel);
    
    manualConnectLayout->addSpacing(8);
    
    m_goButton = new QPushButton("Connect");
    m_goButton->setObjectName("primaryButton");
    m_goButton->setCursor(Qt::PointingHandCursor);
    m_goButton->setMinimumHeight(56);
    connect(m_goButton, &QPushButton::clicked, this, &SimpleMainWindow::onCodeEntered);
    manualConnectLayout->addWidget(m_goButton);
    
    manualConnectLayout->addStretch();
    
    m_stackedWidget->addWidget(m_manualConnectPage);
    
    // ==================== CONNECTED PAGE ====================
    m_connectedPage = new QWidget();
    m_connectedPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* connectedLayout = new QVBoxLayout(m_connectedPage);
    connectedLayout->setContentsMargins(16, 24, 16, 24);
    connectedLayout->setSpacing(16);
    
    // Status header in glass panel
    GlassPanel* connectedHeader = new GlassPanel();
    connectedHeader->setGlowColor(QColor(16, 185, 129, 100));
    connectedHeader->setGlowIntensity(0.7);
    connectedHeader->setBorderRadius(16);
    connectedHeader->setFixedHeight(64);
    QHBoxLayout* headerLayout = new QHBoxLayout(connectedHeader);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    QLabel* connectedIcon = new QLabel("🟢");
    connectedIcon->setStyleSheet("font-size: 20px;");
    headerLayout->addWidget(connectedIcon);
    
    m_connectedLabel = new QLabel("Connected");
    m_connectedLabel->setObjectName("connectedLabel");
    headerLayout->addWidget(m_connectedLabel);
    
    headerLayout->addStretch();
    
    m_peerNameLabel = new QLabel();
    m_peerNameLabel->setObjectName("peerNameLabel");
    headerLayout->addWidget(m_peerNameLabel);
    
    connectedLayout->addWidget(connectedHeader);
    
    // Screen arrangement in glass panel
    GlassPanel* arrangementPanel = new GlassPanel();
    arrangementPanel->setGlowIntensity(0.3);
    QVBoxLayout* arrangementLayout = new QVBoxLayout(arrangementPanel);
    arrangementLayout->setContentsMargins(16, 16, 16, 16);
    
    QLabel* arrangeLabel = new QLabel("Drag to position screens:");
    arrangeLabel->setObjectName("labelText");
    arrangementLayout->addWidget(arrangeLabel);
    
    m_screenArrangement = new ScreenArrangementWidget();
    m_screenArrangement->setLocalScreenName(m_computerName);
    connect(m_screenArrangement, &ScreenArrangementWidget::positionChanged,
            this, &SimpleMainWindow::onScreenArrangementChanged);
    arrangementLayout->addWidget(m_screenArrangement, 1);
    
    connectedLayout->addWidget(arrangementPanel, 1);
    
    // Transfer mode in glass panel
    GlassPanel* modePanel = new GlassPanel();
    modePanel->setGlowIntensity(0.2);
    modePanel->setBorderRadius(16);
    QHBoxLayout* modeLayout = new QHBoxLayout(modePanel);
    modeLayout->setContentsMargins(20, 16, 20, 16);
    
    QLabel* modeLabel = new QLabel("Switch screens:");
    modeLabel->setObjectName("labelText");
    modeLayout->addWidget(modeLabel);
    
    QComboBox* modeCombo = new QComboBox();
    modeCombo->setObjectName("modeCombo");
    modeCombo->addItem("🖱  Move to edge", static_cast<int>(TransferMode::EdgeTransfer));
    modeCombo->addItem("⌨  Hotkey", static_cast<int>(TransferMode::HotkeyTransfer));
    modeCombo->setMinimumHeight(44);
    modeCombo->setMinimumWidth(180);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SimpleMainWindow::onTransferModeChanged);
    modeLayout->addWidget(modeCombo);
    
    connectedLayout->addWidget(modePanel);
    
    m_disconnectButton = new QPushButton("Disconnect");
    m_disconnectButton->setObjectName("dangerButton");
    m_disconnectButton->setCursor(Qt::PointingHandCursor);
    m_disconnectButton->setMinimumHeight(52);
    connect(m_disconnectButton, &QPushButton::clicked, this, &SimpleMainWindow::onDisconnectClicked);
    connectedLayout->addWidget(m_disconnectButton);
    
    m_stackedWidget->addWidget(m_connectedPage);
    
    // ==================== USB CONNECT PAGE ====================
    m_usbConnectPage = new QWidget();
    m_usbConnectPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* usbConnectLayout = new QVBoxLayout(m_usbConnectPage);
    usbConnectLayout->setContentsMargins(16, 24, 16, 24);
    usbConnectLayout->setSpacing(20);

    m_backToHomeFromUSB = new QPushButton("← Back to Home");
    m_backToHomeFromUSB->setObjectName("linkButton");
    m_backToHomeFromUSB->setCursor(Qt::PointingHandCursor);
    connect(m_backToHomeFromUSB, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_homePage);
    });
    usbConnectLayout->addWidget(m_backToHomeFromUSB, 0, Qt::AlignLeft);

    // USB cable icon and title
    QWidget* usbHeaderWidget = new QWidget();
    QHBoxLayout* usbHeaderLayout = new QHBoxLayout(usbHeaderWidget);

    QLabel* usbCableIcon = new QLabel();
    usbCableIcon->setText("🔌");
    usbCableIcon->setStyleSheet("font-size: 48px;");
    usbHeaderLayout->addWidget(usbCableIcon);

    QVBoxLayout* usbTitleLayout = new QVBoxLayout();
    m_usbTitle = new QLabel("Connect via USB Cable");
    m_usbTitle->setObjectName("pageTitle");
    usbTitleLayout->addWidget(m_usbTitle);

    m_usbInstructions = new QLabel("Connect a USB cable between this computer and another running RedkaConnect.\nThe other computer should be set to 'Share This Computer' mode.");
    m_usbInstructions->setObjectName("pageSubtitle");
    m_usbInstructions->setWordWrap(true);
    usbTitleLayout->addWidget(m_usbInstructions);

    usbHeaderLayout->addLayout(usbTitleLayout);
    usbHeaderLayout->addStretch();
    usbConnectLayout->addWidget(usbHeaderWidget);

    usbConnectLayout->addSpacing(20);

    // USB device list in glass panel
    GlassPanel* usbDevicePanel = new GlassPanel();
    usbDevicePanel->setGlowColor(QColor(34, 197, 94, 80));
    QVBoxLayout* usbDevicePanelLayout = new QVBoxLayout(usbDevicePanel);
    usbDevicePanelLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* usbDeviceTitle = new QLabel("Available USB Connections");
    usbDeviceTitle->setObjectName("sectionTitle");
    usbDevicePanelLayout->addWidget(usbDeviceTitle);

    m_usbDeviceList = new QListWidget();
    m_usbDeviceList->setObjectName("deviceList");
    m_usbDeviceList->setMaximumHeight(200);
    connect(m_usbDeviceList, &QListWidget::itemDoubleClicked, this, &SimpleMainWindow::onUSBDeviceSelected);
    usbDevicePanelLayout->addWidget(m_usbDeviceList);

    // Refresh button
    m_usbRefreshButton = new QPushButton("🔄 Refresh USB Devices");
    m_usbRefreshButton->setObjectName("textButton");
    m_usbRefreshButton->setCursor(Qt::PointingHandCursor);
    connect(m_usbRefreshButton, &QPushButton::clicked, this, &SimpleMainWindow::onUSBRefreshClicked);
    usbDevicePanelLayout->addWidget(m_usbRefreshButton);

    usbConnectLayout->addWidget(usbDevicePanel);

    usbConnectLayout->addStretch();

    // Connect button
    QPushButton* usbConnectBtn = new QPushButton("Connect via USB");
    usbConnectBtn->setObjectName("primaryButton");
    usbConnectBtn->setCursor(Qt::PointingHandCursor);
    usbConnectBtn->setMinimumHeight(56);
    usbConnectBtn->setEnabled(false);
    connect(usbConnectBtn, &QPushButton::clicked, this, &SimpleMainWindow::onUSBDeviceSelected);
    usbConnectLayout->addWidget(usbConnectBtn);

    m_stackedWidget->addWidget(m_usbConnectPage);

    // ==================== SETTINGS PAGE ====================
    m_settingsPage = new QWidget();
    m_settingsPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* settingsLayout = new QVBoxLayout(m_settingsPage);
    settingsLayout->setContentsMargins(16, 24, 16, 24);
    settingsLayout->setSpacing(16);
    
    m_backButton = new QPushButton("← Back");
    m_backButton->setObjectName("linkButton");
    m_backButton->setCursor(Qt::PointingHandCursor);
    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        m_stackedWidget->setCurrentWidget(m_homePage);
    });
    settingsLayout->addWidget(m_backButton, 0, Qt::AlignLeft);
    
    m_settingsTitle = new QLabel("Settings");
    m_settingsTitle->setObjectName("pageTitle");
    settingsLayout->addWidget(m_settingsTitle);
    
    settingsLayout->addSpacing(8);
    
    // Computer name setting
    GlassPanel* namePanel = new GlassPanel();
    namePanel->setGlowIntensity(0.2);
    QVBoxLayout* namePanelLayout = new QVBoxLayout(namePanel);
    namePanelLayout->setContentsMargins(20, 16, 20, 16);
    
    QLabel* nameLabel = new QLabel("Computer Name");
    nameLabel->setObjectName("settingsLabel");
    namePanelLayout->addWidget(nameLabel);
    
    QLineEdit* nameEdit = new QLineEdit(m_computerName);
    nameEdit->setObjectName("settingsInput");
    nameEdit->setMinimumHeight(48);
    connect(nameEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_computerName = text;
        m_settings.setValue("computerName", text);
    });
    namePanelLayout->addWidget(nameEdit);
    
    settingsLayout->addWidget(namePanel);
    
    // Port setting
    GlassPanel* portPanel = new GlassPanel();
    portPanel->setGlowIntensity(0.2);
    QVBoxLayout* portPanelLayout = new QVBoxLayout(portPanel);
    portPanelLayout->setContentsMargins(20, 16, 20, 16);
    
    QLabel* portLabel = new QLabel("Connection Port");
    portLabel->setObjectName("settingsLabel");
    portPanelLayout->addWidget(portLabel);
    
    QHBoxLayout* portInputLayout = new QHBoxLayout();
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setObjectName("portSpinBox");
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(m_portManager->selectedPort());
    m_portSpinBox->setMinimumHeight(48);
    connect(m_portSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!m_portManager->setPort(static_cast<quint16>(value))) {
            // Port unavailable, revert
            m_portSpinBox->setValue(m_portManager->selectedPort());
            QMessageBox::warning(this, "Port Unavailable", 
                QString("Port %1 is already in use. Please choose another.").arg(value));
        }
    });
    portInputLayout->addWidget(m_portSpinBox);
    
    QPushButton* autoPortBtn = new QPushButton("Auto");
    autoPortBtn->setObjectName("smallButton");
    autoPortBtn->setFixedWidth(60);
    autoPortBtn->setMinimumHeight(48);
    connect(autoPortBtn, &QPushButton::clicked, this, [this]() {
        quint16 port = m_portManager->autoSelectPort();
        if (port > 0) {
            m_portSpinBox->setValue(port);
        }
    });
    portInputLayout->addWidget(autoPortBtn);
    portPanelLayout->addLayout(portInputLayout);
    
    QLabel* portHint = new QLabel("Default: 24800. Change if blocked by firewall.");
    portHint->setObjectName("hintLabel");
    portPanelLayout->addWidget(portHint);
    
    settingsLayout->addWidget(portPanel);
    
    // Security settings
    GlassPanel* securityPanel = new GlassPanel();
    securityPanel->setGlowIntensity(0.2);
    securityPanel->setGlowColor(QColor(16, 185, 129, 60));
    QVBoxLayout* securityPanelLayout = new QVBoxLayout(securityPanel);
    securityPanelLayout->setContentsMargins(20, 16, 20, 16);
    
    QLabel* securityTitle = new QLabel("🔒 Security");
    securityTitle->setObjectName("settingsLabel");
    securityPanelLayout->addWidget(securityTitle);
    
    QHBoxLayout* fingerprintLayout = new QHBoxLayout();
    QLabel* fpLabel = new QLabel("Your Fingerprint:");
    fpLabel->setObjectName("hintLabel");
    fingerprintLayout->addWidget(fpLabel);
    
    m_fingerprintLabel = new QLabel(m_securityManager->displayFingerprint());
    m_fingerprintLabel->setObjectName("fingerprintValue");
    m_fingerprintLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    fingerprintLayout->addWidget(m_fingerprintLabel);
    fingerprintLayout->addStretch();
    securityPanelLayout->addLayout(fingerprintLayout);
    
    QLabel* securityHint = new QLabel("Connections are encrypted with SSL/TLS.\nVerify fingerprints match when connecting to new devices.");
    securityHint->setObjectName("hintLabel");
    securityHint->setWordWrap(true);
    securityPanelLayout->addWidget(securityHint);
    
    settingsLayout->addWidget(securityPanel);
    
    // Default mode setting
    GlassPanel* defaultModePanel = new GlassPanel();
    defaultModePanel->setGlowIntensity(0.2);
    QVBoxLayout* defaultModePanelLayout = new QVBoxLayout(defaultModePanel);
    defaultModePanelLayout->setContentsMargins(20, 16, 20, 16);
    
    QLabel* defaultModeLabel = new QLabel("Default Transfer Mode");
    defaultModeLabel->setObjectName("settingsLabel");
    defaultModePanelLayout->addWidget(defaultModeLabel);
    
    QComboBox* defaultModeCombo = new QComboBox();
    defaultModeCombo->setObjectName("modeCombo");
    defaultModeCombo->addItem("Move mouse to screen edge");
    defaultModeCombo->addItem("Press hotkey to switch");
    defaultModeCombo->setMinimumHeight(48);
    defaultModePanelLayout->addWidget(defaultModeCombo);
    
    settingsLayout->addWidget(defaultModePanel);
    
    settingsLayout->addStretch();
    
    QLabel* versionLabel = new QLabel("RedkaConnect v1.0");
    versionLabel->setObjectName("versionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);
    settingsLayout->addWidget(versionLabel);

    m_stackedWidget->addWidget(m_settingsPage);

    // ==================== ERROR PAGE (Unplugged Cable) ====================
    m_errorPage = new QWidget();
    m_errorPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* errorLayout = new QVBoxLayout(m_errorPage);
    errorLayout->setContentsMargins(16, 24, 16, 24);
    errorLayout->setSpacing(20);

    // Large unplugged cable icon
    m_errorIconLabel = new QLabel();
    m_errorIconLabel->setAlignment(Qt::AlignCenter);
    m_errorIconLabel->setText("🔌❌");  // Plug disconnected emoji
    m_errorIconLabel->setStyleSheet("font-size: 96px;");

    QGraphicsDropShadowEffect* errorGlow = new QGraphicsDropShadowEffect();
    errorGlow->setBlurRadius(30);
    errorGlow->setColor(QColor(239, 68, 68, 100));  // Red glow
    errorGlow->setOffset(0, 0);
    m_errorIconLabel->setGraphicsEffect(errorGlow);
    errorLayout->addWidget(m_errorIconLabel);

    m_errorTitleLabel = new QLabel("Connection Lost");
    m_errorTitleLabel->setAlignment(Qt::AlignCenter);
    m_errorTitleLabel->setObjectName("errorTitle");
    errorLayout->addWidget(m_errorTitleLabel);

    m_errorMessageLabel = new QLabel("The cable was unplugged. Check your network connection.");
    m_errorMessageLabel->setAlignment(Qt::AlignCenter);
    m_errorMessageLabel->setObjectName("errorMessage");
    m_errorMessageLabel->setWordWrap(true);
    errorLayout->addWidget(m_errorMessageLabel);

    errorLayout->addSpacing(20);

    m_reconnectButton = new QPushButton("Plug Back In");
    m_reconnectButton->setObjectName("primaryButton");
    m_reconnectButton->setCursor(Qt::PointingHandCursor);
    m_reconnectButton->setMinimumHeight(56);
    m_reconnectButton->setToolTip("Try to reconnect (Ctrl+R)");
    m_reconnectButton->setShortcut(QKeySequence("Ctrl+R"));
    connect(m_reconnectButton, &QPushButton::clicked, this, &SimpleMainWindow::onDisconnectClicked);
    errorLayout->addWidget(m_reconnectButton);

    m_stackedWidget->addWidget(m_errorPage);
    
    // Setup discovery timer
    connect(m_discoveryTimer, &QTimer::timeout, this, &SimpleMainWindow::refreshDeviceList);
}

void SimpleMainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("RedkaConnect");
    
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("Show", this, &SimpleMainWindow::showWindow);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Quit", qApp, &QApplication::quit);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &SimpleMainWindow::onTrayActivated);
}

void SimpleMainWindow::setupAnimations()
{
    m_glowAnimation = new QPropertyAnimation(this, "glowIntensity", this);
    m_glowAnimation->setDuration(1500);
    m_glowAnimation->setLoopCount(-1);
    m_glowAnimation->setStartValue(0.3);
    m_glowAnimation->setEndValue(1.0);
    m_glowAnimation->setEasingCurve(QEasingCurve::InOutSine);
}

void SimpleMainWindow::applyStylesheet()
{
    QString css = R"(
        /* Global */
        QWidget {
            color: #e2e8f0;
            font-family: 'Segoe UI', 'SF Pro Display', system-ui, sans-serif;
            font-size: 14px;
        }
        
        /* Titles - Dark text on glass */
        #titleLabel {
            font-size: 42px;
            font-weight: 700;
            color: #f8fafc;
            letter-spacing: -1px;
        }
        
        #pageTitle {
            font-size: 28px;
            font-weight: 600;
            color: #f1f5f9;
        }
        
        #subtitleLabel {
            font-size: 15px;
            color: #94a3b8;
            font-weight: 400;
        }
        
        /* Status */
        #statusLabel {
            font-size: 14px;
            color: #10b981;
            font-weight: 500;
        }
        
        /* Primary Button - Cyan gradient */
        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #0891b2, stop:1 #06b6d4);
            border: none;
            border-radius: 14px;
            color: #0c1220;
            font-size: 17px;
            font-weight: 600;
            padding: 16px 32px;
        }
        
        #primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #06b6d4, stop:1 #22d3ee);
        }
        
        #primaryButton:pressed {
            background: #0891b2;
        }
        
        #primaryButton:focus {
            outline: 2px solid #22d3ee;
            outline-offset: 2px;
        }

        /* Tertiary Button - Green tint for USB */
        #tertiaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(34, 197, 94, 0.3), stop:1 rgba(22, 163, 74, 0.3));
            border: 1px solid rgba(34, 197, 94, 0.4);
            border-radius: 14px;
            color: #dcfce7;
            font-size: 17px;
            font-weight: 600;
            padding: 16px 32px;
        }

        #tertiaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(34, 197, 94, 0.45), stop:1 rgba(22, 163, 74, 0.45));
            border-color: rgba(34, 197, 94, 0.6);
        }

        #tertiaryButton:focus {
            outline: 2px solid #22c55e;
            outline-offset: 2px;
        }

        /* Secondary Button - Purple tint */
        #secondaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(139, 92, 246, 0.3), stop:1 rgba(168, 85, 247, 0.3));
            border: 1px solid rgba(139, 92, 246, 0.4);
            border-radius: 14px;
            color: #e9d5ff;
            font-size: 17px;
            font-weight: 600;
            padding: 16px 32px;
        }
        
        #secondaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(139, 92, 246, 0.45), stop:1 rgba(168, 85, 247, 0.45));
            border-color: rgba(168, 85, 247, 0.6);
        }
        
        #secondaryButton:focus {
            outline: 2px solid #a855f7;
            outline-offset: 2px;
        }
        
        /* Danger Button */
        #dangerButton {
            background: rgba(239, 68, 68, 0.15);
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 14px;
            color: #fca5a5;
            font-size: 15px;
            font-weight: 500;
            padding: 14px 28px;
        }
        
        #dangerButton:hover {
            background: rgba(239, 68, 68, 0.25);
            border-color: rgba(239, 68, 68, 0.5);
        }
        
        /* Link Button */
        #linkButton {
            background: transparent;
            border: none;
            color: #94a3b8;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 16px;
        }
        
        #linkButton:hover {
            color: #e2e8f0;
        }
        
        #linkButton:focus {
            color: #06b6d4;
        }
        
        /* Text Button */
        #textButton {
            background: transparent;
            border: none;
            color: #06b6d4;
            font-size: 14px;
            font-weight: 600;
            padding: 8px 16px;
        }
        
        #textButton:hover {
            color: #22d3ee;
        }
        
        /* Icon Button */
        #iconButton {
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 12px;
            font-size: 20px;
        }
        
        #iconButton:hover {
            background: rgba(255, 255, 255, 0.15);
        }
        
        /* Code Value */
        #codeValue {
            font-size: 48px;
            font-weight: 700;
            color: #22d3ee;
            letter-spacing: 8px;
            font-family: 'JetBrains Mono', 'Cascadia Code', 'Consolas', monospace;
        }
        
        /* Code Input */
        #codeInput {
            background: rgba(15, 23, 42, 0.6);
            border: 2px solid rgba(6, 182, 212, 0.3);
            border-radius: 16px;
            font-size: 36px;
            font-weight: 600;
            color: #f1f5f9;
            letter-spacing: 6px;
            font-family: 'JetBrains Mono', 'Cascadia Code', 'Consolas', monospace;
            padding: 12px;
            selection-background-color: rgba(6, 182, 212, 0.3);
        }
        
        #codeInput:focus {
            border-color: #06b6d4;
            background: rgba(15, 23, 42, 0.8);
        }
        
        #codeInput::placeholder {
            color: #475569;
        }
        
        /* Labels */
        #labelText {
            font-size: 14px;
            color: #94a3b8;
            font-weight: 500;
        }
        
        #hintLabel {
            font-size: 13px;
            color: #64748b;
            line-height: 1.5;
        }
        
        #buttonHint {
            font-size: 13px;
            color: #64748b;
            margin-top: 8px;
        }
        
        /* Connected state */
        #connectedLabel {
            font-size: 18px;
            font-weight: 600;
            color: #34d399;
        }
        
        #peerNameLabel {
            font-size: 14px;
            color: #94a3b8;
            font-weight: 500;
        }
        
        /* Settings */
        #settingsLabel {
            font-size: 13px;
            color: #94a3b8;
            font-weight: 500;
            margin-bottom: 8px;
        }
        
        #settingsInput {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: 12px;
            padding: 12px 16px;
            font-size: 15px;
            color: #e2e8f0;
        }
        
        #settingsInput:focus {
            border-color: #06b6d4;
        }
        
        #versionLabel {
            font-size: 12px;
            color: #475569;
        }
        
        /* Port SpinBox */
        #portSpinBox {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: 12px;
            padding: 12px 16px;
            font-size: 15px;
            color: #e2e8f0;
        }
        
        #portSpinBox:focus {
            border-color: #06b6d4;
        }
        
        /* Small Button */
        #smallButton {
            background: rgba(6, 182, 212, 0.2);
            border: 1px solid rgba(6, 182, 212, 0.3);
            border-radius: 12px;
            color: #06b6d4;
            font-size: 13px;
            font-weight: 600;
        }
        
        #smallButton:hover {
            background: rgba(6, 182, 212, 0.3);
        }
        
        /* Fingerprint Value */
        #fingerprintValue {
            font-family: 'JetBrains Mono', 'Cascadia Code', monospace;
            font-size: 14px;
            color: #10b981;
            font-weight: 600;
        }
        
        /* Combo Box */
        #modeCombo {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: 12px;
            padding: 10px 16px;
            font-size: 14px;
            color: #e2e8f0;
        }
        
        #modeCombo:hover {
            border-color: rgba(148, 163, 184, 0.35);
        }
        
        #modeCombo::drop-down {
            border: none;
            width: 30px;
        }
        
        #modeCombo QAbstractItemView {
            background: #1e293b;
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: 12px;
            selection-background-color: rgba(6, 182, 212, 0.2);
            padding: 4px;
        }
        
        /* Scrollbar */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 4px;
        }
        
        QScrollBar::handle:vertical {
            background: rgba(148, 163, 184, 0.3);
            border-radius: 4px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: rgba(148, 163, 184, 0.5);
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        
        /* Tooltips */
        QToolTip {
            background: #1e293b;
            border: 1px solid rgba(148, 163, 184, 0.2);
            border-radius: 8px;
            color: #e2e8f0;
            padding: 8px 12px;
            font-size: 13px;
        }
        
        /* Focus indicators for accessibility */
        QPushButton:focus {
            outline: 2px solid #06b6d4;
            outline-offset: 2px;
        }
        
        QLineEdit:focus {
            border-color: #06b6d4;
        }
        
        QComboBox:focus {
            border-color: #06b6d4;
        }

        /* Skeuomorphic Status Display */
        #statusTitle {
            font-size: 16px;
            font-weight: 600;
            color: #f1f5f9;
        }

        #statusMessage {
            font-size: 13px;
            color: #94a3b8;
            font-weight: 400;
        }

        /* Error Page Styles */
        #errorTitle {
            font-size: 28px;
            font-weight: 600;
            color: #fca5a5;
            text-align: center;
        }

        #errorMessage {
            font-size: 16px;
            color: #94a3b8;
            text-align: center;
            margin-bottom: 20px;
        }

        /* Button hints */
        #buttonHint {
            font-size: 12px;
            color: #64748b;
            font-style: italic;
        }

        /* Link button */
        #linkButton {
            background: transparent;
            border: none;
            color: #3b82f6;
            font-size: 14px;
            font-weight: 500;
            text-decoration: underline;
        }

        #linkButton:hover {
            color: #60a5fa;
        }

        /* Text button */
        #textButton {
            background: transparent;
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 10px;
            color: #94a3b8;
            font-size: 14px;
            font-weight: 500;
            padding: 12px 20px;
        }

        #textButton:hover {
            background: rgba(148, 163, 184, 0.1);
            border-color: rgba(148, 163, 184, 0.5);
        }

        /* Icon button */
        #iconButton {
            background: rgba(30, 41, 59, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 8px;
            color: #94a3b8;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 12px;
            min-width: 40px;
            min-height: 40px;
        }

        #iconButton:hover {
            background: rgba(148, 163, 184, 0.15);
            border-color: rgba(148, 163, 184, 0.5);
        }
    )";
    
    setStyleSheet(css);
}

void SimpleMainWindow::setState(ConnectionState state)
{
    m_state = state;

    switch (state) {
        case ConnectionState::Disconnected:
            m_statusLabel->setText("Ready to connect");

            // Update skeuomorphic status display
            if (m_statusCableIcon) {
                m_statusCableIcon->setText("🔗");
                m_statusCableIcon->setStyleSheet("color: #10b981; font-size: 16px;");
            }
            if (m_statusMonitorIcon) {
                m_statusMonitorIcon->setStyleSheet("font-size: 24px; opacity: 1.0;");
            }
            if (m_statusTitleLabel) {
                m_statusTitleLabel->setText("Ready to Connect");
            }
            if (m_statusMessageLabel) {
                m_statusMessageLabel->setText("Monitor and cable are ready");
            }

            m_stackedWidget->setCurrentWidget(m_homePage);
            m_glowAnimation->stop();
            break;

        case ConnectionState::Waiting:
            m_waitingLabel->setText("Waiting for connection...");

            // Update status for waiting
            if (m_statusCableIcon) {
                m_statusCableIcon->setText("⏳");
                m_statusCableIcon->setStyleSheet("color: #f59e0b; font-size: 16px;");
            }
            if (m_statusTitleLabel) {
                m_statusTitleLabel->setText("Sharing Computer");
            }
            if (m_statusMessageLabel) {
                m_statusMessageLabel->setText("Waiting for someone to connect");
            }

            m_stackedWidget->setCurrentWidget(m_sharePage);
            m_glowAnimation->start();
            break;

        case ConnectionState::Connecting:
            // Update status for connecting
            if (m_statusCableIcon) {
                m_statusCableIcon->setText("🔄");
                m_statusCableIcon->setStyleSheet("color: #3b82f6; font-size: 16px;");
            }
            if (m_statusTitleLabel) {
                m_statusTitleLabel->setText("Connecting");
            }
            if (m_statusMessageLabel) {
                m_statusMessageLabel->setText("Plugging in the cable...");
            }

            m_glowAnimation->start();
            break;

        case ConnectionState::Connected:
            // Update status for connected
            if (m_statusCableIcon) {
                m_statusCableIcon->setText("🔗");
                m_statusCableIcon->setStyleSheet("color: #10b981; font-size: 16px;");
            }
            if (m_statusMonitorIcon) {
                m_statusMonitorIcon->setStyleSheet("font-size: 24px; opacity: 1.0;");
            }
            if (m_statusTitleLabel) {
                m_statusTitleLabel->setText("Connected");
            }
            if (m_statusMessageLabel) {
                m_statusMessageLabel->setText(QString("Cable connected to %1").arg(m_peerName));
            }

            m_stackedWidget->setCurrentWidget(m_connectedPage);
            m_glowAnimation->stop();
            m_trayIcon->showMessage("RedkaConnect",
                QString("Connected to %1").arg(m_peerName),
                QSystemTrayIcon::Information, 3000);
            break;
    }
}

void SimpleMainWindow::setGlowIntensity(double intensity)
{
    m_glowIntensity = intensity;
}

void SimpleMainWindow::onShareClicked()
{
    m_currentCode = generatePairingCode();
    m_codeValueLabel->setText(m_currentCode);
    m_isServer = true;
    
    m_networkDiscovery->startBroadcastingAsServer(m_computerName, m_currentCode);
    startServer();
    setState(ConnectionState::Waiting);
}

void SimpleMainWindow::onConnectClicked()
{
    m_deviceList->clear();
    m_stackedWidget->setCurrentWidget(m_connectPage);
    m_networkDiscovery->startListeningForServers(m_computerName);
    m_discoveryTimer->start(1000);
}

void SimpleMainWindow::onManualConnectClicked()
{
    m_codeInput->clear();
    m_stackedWidget->setCurrentWidget(m_manualConnectPage);
    m_codeInput->setFocus();
}

void SimpleMainWindow::onDisconnectClicked()
{
    stopProcess();
    m_discoveryTimer->stop();
    m_networkDiscovery->stop();
    setState(ConnectionState::Disconnected);
}

void SimpleMainWindow::onSettingsClicked()
{
    m_stackedWidget->setCurrentWidget(m_settingsPage);
}

void SimpleMainWindow::onCodeEntered()
{
    QString code = m_codeInput->text().trimmed().toUpper();
    
    if (!validateCode(code)) {
        m_codeInput->setStyleSheet("border-color: #ef4444;");
        QTimer::singleShot(1500, this, [this]() {
            applyStylesheet();
        });
        return;
    }
    
    m_currentCode = code;
    m_isServer = false;
    
    QString address = codeToAddress(code);
    startClient(address);
}

void SimpleMainWindow::onDeviceSelected(const DiscoveredDevice& device)
{
    m_discoveryTimer->stop();
    m_networkDiscovery->stop();
    m_peerName = device.name;
    m_peerAddress = device.address;
    m_isServer = false;
    
    startClient(device.address);
}

void SimpleMainWindow::onNetworkDeviceDiscovered(const NetworkDiscovery::DiscoveredDevice& device)
{
    ::DiscoveredDevice listDevice;
    listDevice.name = device.name;
    listDevice.address = device.address;
    listDevice.port = device.port;
    listDevice.isServer = device.isServer;
    listDevice.discoveredAt = QDateTime::currentMSecsSinceEpoch();
    
    m_deviceList->addDevice(listDevice);
}

void SimpleMainWindow::onNetworkDeviceLost(const QString& address)
{
    m_deviceList->removeDevice(address);
}

void SimpleMainWindow::onNetworkError(const QString& error)
{
    qWarning() << "Network discovery error:" << error;
    // Show user-friendly error instead of technical details
    showConnectionError("Network cable unplugged", "Can't find other computers on the network.");
}

void SimpleMainWindow::refreshDeviceList()
{
    if (m_networkDiscovery->isActive()) {
        m_networkDiscovery->refresh();
    }
}

QString SimpleMainWindow::generatePairingCode()
{
    QString localIP;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            const auto addresses = iface.addressEntries();
            for (const auto& addr : addresses) {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    localIP = addr.ip().toString();
                    break;
                }
            }
        }
        if (!localIP.isEmpty()) break;
    }
    
    QStringList parts = localIP.split('.');
    if (parts.size() == 4) {
        int oct3 = parts[2].toInt();
        int oct4 = parts[3].toInt();
        quint16 port = m_portManager->selectedPort();
        
        // Use PortManager to generate code with embedded port
        return PortManager::generateConnectionCode(oct3, oct4, port);
    }
    
    // Fallback: random code with default port
    return QString("%1-%2%3")
        .arg(QRandomGenerator::global()->bounded(1000), 3, 10, QChar('0'))
        .arg(QRandomGenerator::global()->bounded(1000), 3, 10, QChar('0'))
        .arg("00");  // Default port offset
}

QString SimpleMainWindow::codeToAddress(const QString& code)
{
    int thirdOctet, fourthOctet;
    quint16 port;
    
    if (!PortManager::parseConnectionCode(code, thirdOctet, fourthOctet, port)) {
        return QString();
    }
    
    // Get network prefix from local IP
    QString localIP;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            const auto addresses = iface.addressEntries();
            for (const auto& addr : addresses) {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    localIP = addr.ip().toString();
                    break;
                }
            }
        }
        if (!localIP.isEmpty()) break;
    }
    
    QStringList parts = localIP.split('.');
    if (parts.size() >= 2) {
        // Store the port for the connection
        m_portManager->setPort(port);
        
        return QString("%1.%2.%3.%4:%5")
            .arg(parts[0]).arg(parts[1])
            .arg(thirdOctet).arg(fourthOctet)
            .arg(port);
    }
    
    return QString();
}

bool SimpleMainWindow::validateCode(const QString& code)
{
    QString clean = code;
    clean.remove('-');
    clean.remove(' ');
    
    if (clean.length() != 6) return false;
    
    for (const QChar& c : clean) {
        if (!c.isDigit()) return false;
    }
    
    return true;
}

void SimpleMainWindow::startServer()
{
    stopProcess();
    
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SimpleMainWindow::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &SimpleMainWindow::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SimpleMainWindow::onProcessFinished);
    
    QString program = QCoreApplication::applicationDirPath() + "/input-leaps.exe";
    QStringList args;
    args << "--name" << m_computerName;
    args << "--no-daemon";
    args << "--debug" << "INFO";
    
    m_process->start(program, args);
}

void SimpleMainWindow::startClient(const QString& address)
{
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Connection Error", 
            "Could not determine the server address.\n"
            "Make sure both computers are on the same network.");
        return;
    }
    
    stopProcess();
    
    m_peerAddress = address;
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &SimpleMainWindow::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &SimpleMainWindow::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SimpleMainWindow::onProcessFinished);
    
    QString program = QCoreApplication::applicationDirPath() + "/input-leapc.exe";
    QStringList args;
    args << "--name" << m_computerName;
    args << "--no-daemon";
    args << "--debug" << "INFO";
    args << address;
    
    m_process->start(program, args);
}

void SimpleMainWindow::stopProcess()
{
    if (m_process) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void SimpleMainWindow::onProcessOutput()
{
    if (!m_process) return;
    
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    
    if (output.contains("connected to server", Qt::CaseInsensitive) || 
        output.contains("client connected", Qt::CaseInsensitive)) {
        if (m_peerName.isEmpty()) {
            m_peerName = "Remote PC";
        }
        m_peerNameLabel->setText(m_peerName);
        m_screenArrangement->setRemoteScreenName(m_peerName);
        setState(ConnectionState::Connected);
    }
}

void SimpleMainWindow::onProcessError()
{
    if (!m_process) return;
    QString error = QString::fromUtf8(m_process->readAllStandardError());

    // Show unplugged cable error page instead of technical error message
    showConnectionError("The cable was unplugged", "Connection to the other computer was lost.");
}

void SimpleMainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(status)
    
    if (m_state == ConnectionState::Connected) {
        setState(ConnectionState::Disconnected);
        m_trayIcon->showMessage("RedkaConnect", "Disconnected", 
            QSystemTrayIcon::Warning, 3000);
    }
}

void SimpleMainWindow::showConnectionError(const QString& title, const QString& message)
{
    // Update error page content
    m_errorTitleLabel->setText(title);
    m_errorMessageLabel->setText(message);

    // Switch to error page
    m_stackedWidget->setCurrentWidget(m_errorPage);

    // Update status icons to show disconnected state
    if (m_statusCableIcon) {
        m_statusCableIcon->setText("❌");  // Broken link
        m_statusCableIcon->setStyleSheet("color: #ef4444; font-size: 16px;");
    }
    if (m_statusMonitorIcon) {
        m_statusMonitorIcon->setStyleSheet("font-size: 24px; opacity: 0.5;");
    }
    if (m_statusTitleLabel) {
        m_statusTitleLabel->setText("Disconnected");
    }
    if (m_statusMessageLabel) {
        m_statusMessageLabel->setText("Cable unplugged");
    }

    // Set disconnected state
    setState(ConnectionState::Disconnected);
}

void SimpleMainWindow::onUSBConnectClicked()
{
    m_connectionType = ConnectionType::USB;
    m_stackedWidget->setCurrentWidget(m_usbConnectPage);

    // Start USB device discovery
    m_usbManager->startAutoDiscovery();
    m_usbManager->refreshDevices();
}

void SimpleMainWindow::onUSBDeviceSelected()
{
    QListWidgetItem* currentItem = m_usbDeviceList->currentItem();
    if (!currentItem) {
        return;
    }

    QString portName = currentItem->data(Qt::UserRole).toString();
    if (portName.isEmpty()) {
        return;
    }

    // Try to connect via USB
    if (m_usbManager->connectToDevice(portName)) {
        m_connectionType = ConnectionType::USB;
        // Start the actual RedkaConnect client/server process
        // This would need to be integrated with the existing process management
        m_peerName = QString("USB Device (%1)").arg(portName);
        setState(ConnectionState::Connected);
    } else {
        // Show error
        showConnectionError("USB Connection Failed",
            QString("Could not connect to USB device on port %1.\nMake sure the other computer is running RedkaConnect in share mode.").arg(portName));
    }
}

void SimpleMainWindow::onUSBDeviceDiscovered(const USBConnectionManager::USBDevice& device)
{
    // Add to the device list
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(QString("🔌 %1 (%2)").arg(device.description, device.portName));
    item->setData(Qt::UserRole, device.portName);
    m_usbDeviceList->addItem(item);
}

void SimpleMainWindow::onUSBDeviceRemoved(const QString& portName)
{
    // Remove from device list
    for (int i = 0; i < m_usbDeviceList->count(); ++i) {
        QListWidgetItem* item = m_usbDeviceList->item(i);
        if (item->data(Qt::UserRole).toString() == portName) {
            delete m_usbDeviceList->takeItem(i);
            break;
        }
    }
}

void SimpleMainWindow::onUSBConnected(const QString& portName)
{
    qDebug() << "USB connected to port:" << portName;
    m_currentPort = portName;
}

void SimpleMainWindow::onUSBDisconnected()
{
    qDebug() << "USB disconnected";
    if (m_connectionType == ConnectionType::USB && m_state == ConnectionState::Connected) {
        showConnectionError("USB Cable Disconnected", "The USB cable was unplugged. Please reconnect the cable.");
    }
}

void SimpleMainWindow::onUSBError(const QString& error)
{
    qWarning() << "USB error:" << error;
    showConnectionError("USB Connection Error", error);
}

void SimpleMainWindow::onUSBRefreshClicked()
{
    m_usbDeviceList->clear();
    m_usbManager->refreshDevices();
}

void SimpleMainWindow::onUSBBackClicked()
{
    m_usbManager->stopAutoDiscovery();
    m_stackedWidget->setCurrentWidget(m_homePage);
}

void SimpleMainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        showWindow();
    }
}

void SimpleMainWindow::showWindow()
{
    show();
    raise();
    activateWindow();
}

void SimpleMainWindow::copyCodeToClipboard()
{
    QApplication::clipboard()->setText(m_currentCode);
    
    QString originalText = m_copyCodeButton->text();
    m_copyCodeButton->setText("✓");
    QTimer::singleShot(1500, this, [this, originalText]() {
        m_copyCodeButton->setText(originalText);
    });
}

void SimpleMainWindow::onScreenArrangementChanged()
{
    auto pos = m_screenArrangement->screenPosition();
    
    QString direction;
    switch (pos) {
        case ScreenArrangementWidget::ScreenPosition::Left: direction = "left"; break;
        case ScreenArrangementWidget::ScreenPosition::Right: direction = "right"; break;
        case ScreenArrangementWidget::ScreenPosition::Top: direction = "up"; break;
        case ScreenArrangementWidget::ScreenPosition::Bottom: direction = "down"; break;
    }
}

void SimpleMainWindow::onTransferModeChanged(int index)
{
    m_transferMode = static_cast<TransferMode>(index);
}

void SimpleMainWindow::updateConnectionStatus() {}
void SimpleMainWindow::pulseGlow() {}

void SimpleMainWindow::saveSettings()
{
    m_settings.setValue("computerName", m_computerName);
    m_settings.setValue("transferMode", static_cast<int>(m_transferMode));
}

void SimpleMainWindow::loadSettings()
{
    m_computerName = m_settings.value("computerName", QHostInfo::localHostName()).toString();
    m_transferMode = static_cast<TransferMode>(
        m_settings.value("transferMode", static_cast<int>(TransferMode::EdgeTransfer)).toInt());
}

void SimpleMainWindow::closeEvent(QCloseEvent* event)
{
    if (m_state == ConnectionState::Connected) {
        hide();
        event->ignore();
        m_trayIcon->showMessage("RedkaConnect", 
            "Running in background. Double-click tray to show.",
            QSystemTrayIcon::Information, 2000);
    } else {
        saveSettings();
        stopProcess();
        event->accept();
    }
}

void SimpleMainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && m_state == ConnectionState::Connected) {
            QTimer::singleShot(0, this, &SimpleMainWindow::hide);
        }
    }
}
