/*
 * RedkaConnect Demo -- Showcase of new UI features
 * Based on InputLeap
 */

#include "RedkaConnectDemo.h"
#include "GlassPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFrame>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>
#include <QInputDialog>

RedkaConnectDemo::RedkaConnectDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_stackedWidget(nullptr)
    , m_demoTimer(new QTimer(this))
    , m_demoStep(0)
{
    setWindowTitle("RedkaConnect Demo - New Features Showcase");
    setMinimumSize(500, 600);
    setMaximumSize(600, 700);

    setupUi();
    applyStylesheet();

    // Demo timer for simulated interactions
    connect(m_demoTimer, &QTimer::timeout, this, &RedkaConnectDemo::onTimerTick);
}

RedkaConnectDemo::~RedkaConnectDemo() = default;

void RedkaConnectDemo::setupUi()
{
    // Create central widget with animated background
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Animated background
    QWidget* background = new QWidget(centralWidget);
    background->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                              "stop:0 #080c15, stop:1 #0f172a);");
    background->setGeometry(centralWidget->rect());
    background->lower();

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->setAttribute(Qt::WA_TranslucentBackground);
    mainLayout->addWidget(m_stackedWidget);

    // Create all pages
    setupHomePage();
    setupSharePage();
    setupConnectPage();
    setupUSBPage();
    setupErrorPage();

    // Start with home page
    m_stackedWidget->setCurrentWidget(m_homePage);
}

void RedkaConnectDemo::setupHomePage()
{
    m_homePage = new QWidget();
    m_homePage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout(m_homePage);
    layout->setContentsMargins(24, 32, 24, 24);
    layout->setSpacing(16);

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
    layout->addWidget(m_logoLabel);

    // Title
    QLabel* titleLabel = new QLabel("RedkaConnect Demo");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    layout->addWidget(titleLabel);

    // Subtitle
    QLabel* subtitleLabel = new QLabel("Showcasing new skeuomorphic features");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setObjectName("subtitleLabel");
    layout->addWidget(subtitleLabel);

    layout->addSpacing(24);

    // Status in glass panel (skeuomorphic)
    GlassPanel* statusPanel = new GlassPanel();
    statusPanel->setFixedHeight(64);
    statusPanel->setBorderRadius(20);
    statusPanel->setGlowColor(QColor(16, 185, 129, 80));
    QHBoxLayout* statusLayout = new QHBoxLayout(statusPanel);
    statusLayout->setContentsMargins(20, 8, 20, 8);
    statusLayout->setSpacing(12);

    // Monitor icon
    QLabel* monitorIcon = new QLabel("🖥️");
    monitorIcon->setStyleSheet("font-size: 24px;");
    monitorIcon->setFixedSize(32, 32);
    statusLayout->addWidget(monitorIcon);

    // Cable connection icon
    QLabel* cableIcon = new QLabel("🔗");
    cableIcon->setStyleSheet("color: #10b981; font-size: 16px;");
    statusLayout->addWidget(cableIcon);

    // Status text
    QVBoxLayout* statusTextLayout = new QVBoxLayout();
    statusTextLayout->setContentsMargins(0, 0, 0, 0);
    statusTextLayout->setSpacing(2);

    QLabel* statusTitle = new QLabel("Ready to Connect");
    statusTitle->setObjectName("statusTitle");
    statusTextLayout->addWidget(statusTitle);

    QLabel* statusMessage = new QLabel("Monitor and cable are ready");
    statusMessage->setObjectName("statusMessage");
    statusTextLayout->addWidget(statusMessage);

    statusLayout->addLayout(statusTextLayout);
    statusLayout->addStretch();

    layout->addWidget(statusPanel);

    layout->addSpacing(24);

    // Main action buttons
    GlassPanel* sharePanel = new GlassPanel();
    sharePanel->setGlowColor(QColor(6, 182, 212, 100));
    sharePanel->setGlowIntensity(0.6);
    QVBoxLayout* sharePanelLayout = new QVBoxLayout(sharePanel);
    sharePanelLayout->setContentsMargins(24, 20, 24, 20);

    m_shareButton = new QPushButton("Share This Computer (Network)");
    m_shareButton->setObjectName("primaryButton");
    m_shareButton->setCursor(Qt::PointingHandCursor);
    m_shareButton->setMinimumHeight(56);
    connect(m_shareButton, &QPushButton::clicked, this, &RedkaConnectDemo::onShareClicked);
    sharePanelLayout->addWidget(m_shareButton);

    QLabel* shareHint = new QLabel("📤 Others will use your keyboard & mouse");
    shareHint->setObjectName("buttonHint");
    shareHint->setAlignment(Qt::AlignCenter);
    sharePanelLayout->addWidget(shareHint);

    layout->addWidget(sharePanel);

    layout->addSpacing(12);

    GlassPanel* connectPanel = new GlassPanel();
    connectPanel->setGlowColor(QColor(139, 92, 246, 80));
    connectPanel->setGlowIntensity(0.4);
    QVBoxLayout* connectPanelLayout = new QVBoxLayout(connectPanel);
    connectPanelLayout->setContentsMargins(24, 20, 24, 20);

    m_connectButton = new QPushButton("Connect to Computer (Network)");
    m_connectButton->setObjectName("secondaryButton");
    m_connectButton->setCursor(Qt::PointingHandCursor);
    m_connectButton->setMinimumHeight(56);
    connect(m_connectButton, &QPushButton::clicked, this, &RedkaConnectDemo::onConnectClicked);
    connectPanelLayout->addWidget(m_connectButton);

    QLabel* connectHint = new QLabel("📥 Control this PC from another");
    connectHint->setObjectName("buttonHint");
    connectHint->setAlignment(Qt::AlignCenter);
    connectPanelLayout->addWidget(connectHint);

    layout->addWidget(connectPanel);

    layout->addSpacing(12);

    // USB connection panel (new feature!)
    GlassPanel* usbPanel = new GlassPanel();
    usbPanel->setGlowColor(QColor(34, 197, 94, 80));
    usbPanel->setGlowIntensity(0.4);
    QVBoxLayout* usbPanelLayout = new QVBoxLayout(usbPanel);
    usbPanelLayout->setContentsMargins(24, 20, 24, 20);

    m_usbButton = new QPushButton("Connect via USB Cable (NEW!)");
    m_usbButton->setObjectName("tertiaryButton");
    m_usbButton->setCursor(Qt::PointingHandCursor);
    m_usbButton->setMinimumHeight(56);
    connect(m_usbButton, &QPushButton::clicked, this, &RedkaConnectDemo::onUSBClicked);
    usbPanelLayout->addWidget(m_usbButton);

    QLabel* usbHint = new QLabel("🔌 Direct USB cable connection");
    usbHint->setObjectName("buttonHint");
    usbHint->setAlignment(Qt::AlignCenter);
    usbPanelLayout->addWidget(usbHint);

    layout->addWidget(usbPanel);

    layout->addStretch();

    // Settings button
    m_settingsButton = new QPushButton("⚙️ Demo Controls");
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setFixedSize(140, 40);
    m_settingsButton->setToolTip("Demo controls and error simulation");
    connect(m_settingsButton, &QPushButton::clicked, this, &RedkaConnectDemo::onSettingsClicked);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_settingsButton);
    layout->addLayout(bottomLayout);

    m_stackedWidget->addWidget(m_homePage);
}

void RedkaConnectDemo::setupSharePage()
{
    m_sharePage = new QWidget();
    m_sharePage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout(m_sharePage);
    layout->setContentsMargins(24, 32, 24, 24);
    layout->setSpacing(20);

    QLabel* backButton = new QLabel("← Back to Home");
    backButton->setObjectName("linkButton");
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &QLabel::linkActivated, this, [this]() { /* dummy */ });
    backButton->installEventFilter(this);
    layout->addWidget(backButton, 0, Qt::AlignLeft);

    // Share icon
    QLabel* shareIcon = new QLabel("📤");
    shareIcon->setAlignment(Qt::AlignCenter);
    shareIcon->setStyleSheet("font-size: 72px;");
    layout->addWidget(shareIcon);

    QLabel* shareTitle = new QLabel("Sharing Computer");
    shareTitle->setAlignment(Qt::AlignCenter);
    shareTitle->setObjectName("pageTitle");
    layout->addWidget(shareTitle);

    QLabel* shareDesc = new QLabel("Your pairing code is ready");
    shareDesc->setAlignment(Qt::AlignCenter);
    shareDesc->setObjectName("pageSubtitle");
    layout->addWidget(shareDesc);

    layout->addSpacing(20);

    // Code display in glass panel
    GlassPanel* codePanel = new GlassPanel();
    codePanel->setGlowColor(QColor(6, 182, 212, 100));
    QVBoxLayout* codePanelLayout = new QVBoxLayout(codePanel);
    codePanelLayout->setContentsMargins(24, 24, 24, 24);

    m_codeLabel = new QLabel("Your pairing code:");
    m_codeLabel->setAlignment(Qt::AlignCenter);
    m_codeLabel->setObjectName("codeLabel");
    codePanelLayout->addWidget(m_codeLabel);

    m_codeValueLabel = new QLabel("000-000");
    m_codeValueLabel->setAlignment(Qt::AlignCenter);
    m_codeValueLabel->setObjectName("codeValue");
    codePanelLayout->addWidget(m_codeValueLabel);

    m_copyCodeButton = new QPushButton("📋 Copy Code");
    m_copyCodeButton->setObjectName("textButton");
    m_copyCodeButton->setCursor(Qt::PointingHandCursor);
    connect(m_copyCodeButton, &QPushButton::clicked, [this]() {
        QApplication::clipboard()->setText(m_codeValueLabel->text());
        QMessageBox::information(this, "Demo", "Code copied to clipboard!\n\nIn real app, this would be shared with connecting computer.");
    });
    codePanelLayout->addWidget(m_copyCodeButton);

    layout->addWidget(codePanel);

    layout->addSpacing(20);

    m_waitingLabel = new QLabel("Waiting for someone to connect...");
    m_waitingLabel->setAlignment(Qt::AlignCenter);
    m_waitingLabel->setObjectName("waitingLabel");
    layout->addWidget(m_waitingLabel);

    layout->addStretch();

    m_cancelShareButton = new QPushButton("Stop Sharing");
    m_cancelShareButton->setObjectName("dangerButton");
    m_cancelShareButton->setCursor(Qt::PointingHandCursor);
    m_cancelShareButton->setMinimumHeight(48);
    connect(m_cancelShareButton, &QPushButton::clicked, this, &RedkaConnectDemo::onBackToHomeClicked);
    layout->addWidget(m_cancelShareButton);

    m_stackedWidget->addWidget(m_sharePage);
}

void RedkaConnectDemo::setupConnectPage()
{
    m_connectPage = new QWidget();
    m_connectPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout(m_connectPage);
    layout->setContentsMargins(24, 32, 24, 24);
    layout->setSpacing(20);

    QLabel* backButton = new QLabel("← Back to Home");
    backButton->setObjectName("linkButton");
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &QLabel::linkActivated, this, [this]() { /* dummy */ });
    backButton->installEventFilter(this);
    layout->addWidget(backButton, 0, Qt::AlignLeft);

    QLabel* connectIcon = new QLabel("🔍");
    connectIcon->setAlignment(Qt::AlignCenter);
    connectIcon->setStyleSheet("font-size: 72px;");
    layout->addWidget(connectIcon);

    m_connectTitle = new QLabel("Looking for Computers");
    m_connectTitle->setAlignment(Qt::AlignCenter);
    m_connectTitle->setObjectName("pageTitle");
    layout->addWidget(m_connectTitle);

    QLabel* connectDesc = new QLabel("Searching for RedkaConnect computers on your network...");
    connectDesc->setAlignment(Qt::AlignCenter);
    connectDesc->setWordWrap(true);
    connectDesc->setObjectName("pageSubtitle");
    layout->addWidget(connectDesc);

    layout->addSpacing(32);

    // Manual connect option
    GlassPanel* manualPanel = new GlassPanel();
    manualPanel->setGlowColor(QColor(139, 92, 246, 80));
    QVBoxLayout* manualPanelLayout = new QVBoxLayout(manualPanel);
    manualPanelLayout->setContentsMargins(24, 20, 24, 20);

    m_manualConnectButton = new QPushButton("Enter Code Manually →");
    m_manualConnectButton->setObjectName("textButton");
    m_manualConnectButton->setCursor(Qt::PointingHandCursor);
    connect(m_manualConnectButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Demo", "Manual code entry would open here!\n\nIn real app, you could paste QR codes or type pairing codes.");
    });
    manualPanelLayout->addWidget(m_manualConnectButton);

    QLabel* manualLabel = new QLabel("Enter a pairing code or scan QR code");
    manualLabel->setAlignment(Qt::AlignCenter);
    manualLabel->setObjectName("manualLabel");
    manualPanelLayout->addWidget(manualLabel);

    layout->addWidget(manualPanel);

    layout->addStretch();

    m_cancelConnectButton = new QPushButton("Stop Looking");
    m_cancelConnectButton->setObjectName("secondaryButton");
    m_cancelConnectButton->setCursor(Qt::PointingHandCursor);
    m_cancelConnectButton->setMinimumHeight(48);
    connect(m_cancelConnectButton, &QPushButton::clicked, this, &RedkaConnectDemo::onBackToHomeClicked);
    layout->addWidget(m_cancelConnectButton);

    m_stackedWidget->addWidget(m_connectPage);
}

void RedkaConnectDemo::setupUSBPage()
{
    m_usbPage = new QWidget();
    m_usbPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout(m_usbPage);
    layout->setContentsMargins(24, 32, 24, 24);
    layout->setSpacing(20);

    m_backToHomeFromUSB = new QPushButton("← Back to Home");
    m_backToHomeFromUSB->setObjectName("linkButton");
    m_backToHomeFromUSB->setCursor(Qt::PointingHandCursor);
    connect(m_backToHomeFromUSB, &QPushButton::clicked, this, &RedkaConnectDemo::onBackToHomeClicked);
    layout->addWidget(m_backToHomeFromUSB, 0, Qt::AlignLeft);

    // USB cable icon and title
    QWidget* usbHeaderWidget = new QWidget();
    QHBoxLayout* usbHeaderLayout = new QHBoxLayout(usbHeaderWidget);

    QLabel* usbCableIcon = new QLabel("🔌");
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
    layout->addWidget(usbHeaderWidget);

    layout->addSpacing(20);

    // USB device list in glass panel
    GlassPanel* usbDevicePanel = new GlassPanel();
    usbDevicePanel->setGlowColor(QColor(34, 197, 94, 80));
    QVBoxLayout* usbDevicePanelLayout = new QVBoxLayout(usbDevicePanel);
    usbDevicePanelLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* usbDeviceTitle = new QLabel("Available USB Connections");
    usbDeviceTitle->setObjectName("sectionTitle");
    usbDevicePanelLayout->addWidget(usbDeviceTitle);

    m_usbDeviceList = new QListWidget();
    m_usbDeviceList->setMaximumHeight(200);
    // Add some demo USB devices
    m_usbDeviceList->addItem("🔌 USB Serial Device (COM3)");
    m_usbDeviceList->addItem("🔌 RedkaConnect Device (COM5)");
    m_usbDeviceList->addItem("🔌 CDC-ACM Device (COM7)");
    usbDevicePanelLayout->addWidget(m_usbDeviceList);

    // Refresh button
    m_usbRefreshButton = new QPushButton("🔄 Refresh USB Devices");
    m_usbRefreshButton->setObjectName("textButton");
    m_usbRefreshButton->setCursor(Qt::PointingHandCursor);
    connect(m_usbRefreshButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Demo", "Refreshing USB devices...\n\nIn real app, this would scan for new USB serial devices.");
    });
    usbDevicePanelLayout->addWidget(m_usbRefreshButton);

    layout->addWidget(usbDevicePanel);

    layout->addStretch();

    // Connect button
    m_usbConnectButton = new QPushButton("Connect via USB");
    m_usbConnectButton->setObjectName("primaryButton");
    m_usbConnectButton->setCursor(Qt::PointingHandCursor);
    m_usbConnectButton->setMinimumHeight(56);
    connect(m_usbConnectButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Demo", "USB Connection Established! 🎉\n\nIn real app, this would:\n• Send handshake to other computer\n• Establish secure connection\n• Start mouse/keyboard sharing\n\nThis demonstrates the new USB cable support feature!");
    });
    layout->addWidget(m_usbConnectButton);

    m_stackedWidget->addWidget(m_usbPage);
}

void RedkaConnectDemo::setupErrorPage()
{
    m_errorPage = new QWidget();
    m_errorPage->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* layout = new QVBoxLayout(m_errorPage);
    layout->setContentsMargins(24, 32, 24, 24);
    layout->setSpacing(20);

    // Large unplugged cable icon
    m_errorIconLabel = new QLabel();
    m_errorIconLabel->setAlignment(Qt::AlignCenter);
    m_errorIconLabel->setText("🔌❌");
    m_errorIconLabel->setStyleSheet("font-size: 96px;");

    QGraphicsDropShadowEffect* errorGlow = new QGraphicsDropShadowEffect();
    errorGlow->setBlurRadius(30);
    errorGlow->setColor(QColor(239, 68, 68, 100));
    errorGlow->setOffset(0, 0);
    m_errorIconLabel->setGraphicsEffect(errorGlow);
    layout->addWidget(m_errorIconLabel);

    m_errorTitleLabel = new QLabel("Connection Lost");
    m_errorTitleLabel->setAlignment(Qt::AlignCenter);
    m_errorTitleLabel->setObjectName("errorTitle");
    layout->addWidget(m_errorTitleLabel);

    m_errorMessageLabel = new QLabel("The cable was unplugged. Check your network connection.");
    m_errorMessageLabel->setAlignment(Qt::AlignCenter);
    m_errorMessageLabel->setObjectName("errorMessage");
    m_errorMessageLabel->setWordWrap(true);
    layout->addWidget(m_errorMessageLabel);

    layout->addSpacing(20);

    m_reconnectButton = new QPushButton("Plug Back In");
    m_reconnectButton->setObjectName("primaryButton");
    m_reconnectButton->setCursor(Qt::PointingHandCursor);
    m_reconnectButton->setMinimumHeight(56);
    connect(m_reconnectButton, &QPushButton::clicked, this, &RedkaConnectDemo::onPlugBackInClicked);
    layout->addWidget(m_reconnectButton);

    layout->addStretch();

    // Demo controls
    QPushButton* demoErrorButton = new QPushButton("⚡ Simulate Error (Demo)");
    demoErrorButton->setObjectName("dangerButton");
    demoErrorButton->setCursor(Qt::PointingHandCursor);
    connect(demoErrorButton, &QPushButton::clicked, this, &RedkaConnectDemo::onSimulateErrorClicked);
    layout->addWidget(demoErrorButton);

    m_stackedWidget->addWidget(m_errorPage);
}

void RedkaConnectDemo::applyStylesheet()
{
    QString css = R"(
        /* Global */
        QWidget {
            color: #e2e8f0;
            font-family: 'Segoe UI', 'SF Pro Display', system-ui, sans-serif;
            font-size: 14px;
        }

        /* Titles */
        #titleLabel {
            font-size: 32px;
            font-weight: 700;
            color: #f8fafc;
            letter-spacing: -1px;
        }

        #pageTitle {
            font-size: 28px;
            font-weight: 600;
            color: #f1f5f9;
        }

        #subtitleLabel, #pageSubtitle {
            font-size: 15px;
            color: #94a3b8;
            font-weight: 400;
        }

        /* Status display */
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

        /* Buttons */
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

        #dangerButton {
            background: rgba(239, 68, 68, 0.15);
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 14px;
            color: #fca5a5;
            font-size: 15px;
            font-weight: 500;
            padding: 14px 28px;
        }

        #textButton {
            background: rgba(30, 41, 59, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 10px;
            color: #94a3b8;
            font-size: 14px;
            font-weight: 500;
            padding: 12px 20px;
        }

        #textButton:hover {
            background: rgba(148, 163, 184, 0.15);
            border-color: rgba(148, 163, 184, 0.5);
        }

        #settingsButton {
            background: rgba(30, 41, 59, 0.6);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 8px;
            color: #94a3b8;
            font-size: 12px;
            padding: 8px 12px;
        }

        /* Links */
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

        /* Other elements */
        #buttonHint {
            font-size: 12px;
            color: #64748b;
            font-style: italic;
        }

        #codeValue {
            font-size: 36px;
            font-weight: 700;
            color: #22d3ee;
            letter-spacing: 8px;
            font-family: 'JetBrains Mono', monospace;
        }

        #waitingLabel {
            font-size: 16px;
            color: #94a3b8;
        }

        #errorTitle {
            font-size: 28px;
            font-weight: 600;
            color: #fca5a5;
        }

        #errorMessage {
            font-size: 16px;
            color: #94a3b8;
        }

        /* QListWidget styling */
        QListWidget {
            background: rgba(30, 41, 59, 0.8);
            border: 1px solid rgba(148, 163, 184, 0.3);
            border-radius: 8px;
            color: #e2e8f0;
            selection-background-color: rgba(6, 182, 212, 0.3);
        }

        QListWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid rgba(148, 163, 184, 0.1);
        }

        QListWidget::item:hover {
            background: rgba(148, 163, 184, 0.1);
        }

        QListWidget::item:selected {
            background: rgba(6, 182, 212, 0.2);
        }
    )";

    setStyleSheet(css);
}

void RedkaConnectDemo::onShareClicked()
{
    // Generate a demo pairing code
    int code1 = QRandomGenerator::global()->bounded(100, 999);
    int code2 = QRandomGenerator::global()->bounded(100, 999);
    m_currentCode = QString("%1-%2").arg(code1, 3, 10, QChar('0')).arg(code2, 3, 10, QChar('0'));

    m_codeValueLabel->setText(m_currentCode);
    m_stackedWidget->setCurrentWidget(m_sharePage);

    // Start demo timer to simulate connection
    m_demoStep = 0;
    m_demoTimer->start(2000);
}

void RedkaConnectDemo::onConnectClicked()
{
    m_stackedWidget->setCurrentWidget(m_connectPage);
}

void RedkaConnectDemo::onUSBClicked()
{
    m_stackedWidget->setCurrentWidget(m_usbPage);
}

void RedkaConnectDemo::onSettingsClicked()
{
    QStringList options;
    options << "Simulate Connection Lost Error"
            << "Show QR Code Demo"
            << "Back to Home";

    bool ok;
    QString choice = QInputDialog::getItem(this, "Demo Controls",
                                         "Choose a demo action:", options, 0, false, &ok);

    if (ok) {
        if (choice == "Simulate Connection Lost Error") {
            onSimulateErrorClicked();
        } else if (choice == "Show QR Code Demo") {
            QMessageBox::information(this, "QR Code Demo",
                "QR Code Demo:\n\n"
                "1. On sharing computer: QR code appears\n"
                "2. Connecting computer scans with phone camera\n"
                "3. Copy decoded JSON to clipboard\n"
                "4. Click 'Paste' in pairing dialog\n"
                "5. PIN auto-fills and connection establishes!\n\n"
                "This works without needing webcam scanning!");
        } else if (choice == "Back to Home") {
            m_stackedWidget->setCurrentWidget(m_homePage);
        }
    }
}

void RedkaConnectDemo::onBackToHomeClicked()
{
    m_demoTimer->stop();
    m_stackedWidget->setCurrentWidget(m_homePage);
}

void RedkaConnectDemo::onTimerTick()
{
    m_demoStep++;

    switch (m_demoStep) {
        case 1:
            m_waitingLabel->setText("Waiting for someone to connect... (Demo: 3 seconds)");
            break;
        case 2:
            m_waitingLabel->setText("Waiting for someone to connect... (Demo: 1 second)");
            break;
        case 3:
            m_waitingLabel->setText("Connection established! 🎉");
            QMessageBox::information(this, "Demo Complete",
                "Demo Connection Successful!\n\n"
                "In the real app, mouse and keyboard sharing would start now.\n\n"
                "Features demonstrated:\n"
                "• Skeuomorphic interface with monitor icons\n"
                "• Pairing code generation\n"
                "• Visual status feedback\n"
                "• Professional UI design");
            m_demoTimer->stop();
            break;
    }
}

void RedkaConnectDemo::onSimulateErrorClicked()
{
    m_errorTitleLabel->setText("Connection Lost");
    m_errorMessageLabel->setText("The cable was unplugged. Check your network connection.");
    m_stackedWidget->setCurrentWidget(m_errorPage);
}

void RedkaConnectDemo::onPlugBackInClicked()
{
    QMessageBox::information(this, "Demo", "Reconnecting...\n\nIn real app, this would:\n• Re-establish network connection\n• Resume mouse/keyboard sharing\n• Update status indicators");
    m_stackedWidget->setCurrentWidget(m_homePage);
}

bool RedkaConnectDemo::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == findChild<QLabel*>("linkButton")) {
            onBackToHomeClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}