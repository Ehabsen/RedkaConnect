/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "PairingDialog.h"
#include "PairingManager.h"
#include "QRCodeWidget.h"
#include "GlassPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

PairingDialog::PairingDialog(PairingManager* pairingManager, Mode mode, QWidget* parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_pairingManager(pairingManager)
    , m_pinLabel(nullptr)
    , m_countdownLabel(nullptr)
    , m_qrCode(nullptr)
    , m_deviceNameLabel(nullptr)
    , m_verifyButton(nullptr)
    , m_cancelButton(nullptr)
    , m_timer(new QTimer(this))
    , m_pairingSuccessful(false)
{
    setWindowTitle(mode == Mode::ShowPin ? "Share Your Screen" : "Enter PIN");
    setFixedSize(420, 520);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    if (mode == Mode::ShowPin) {
        setupShowPinUi();
    } else {
        setupEnterPinUi();
    }
    
    applyStylesheet();
    
    connect(m_timer, &QTimer::timeout, this, &PairingDialog::onTimerTick);
}

PairingDialog::~PairingDialog() = default;

void PairingDialog::setRemoteDevice(const QString& deviceId, const QString& deviceName)
{
    m_remoteDeviceId = deviceId;
    m_remoteDeviceName = deviceName;
    
    if (m_deviceNameLabel) {
        m_deviceNameLabel->setText(QString("Connecting to \"%1\"").arg(deviceName));
    }
}

QString PairingDialog::enteredPin() const
{
    QString pin;
    for (int i = 0; i < 6; ++i) {
        if (m_pinInput[i]) {
            pin += m_pinInput[i]->text();
        }
    }
    return pin;
}

void PairingDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    
    if (m_mode == Mode::ShowPin) {
        // Generate PIN
        QString pin = m_pairingManager->generatePairingPin();
        m_pinLabel->setText(pin.left(3) + " " + pin.mid(3));
        
        // Generate QR code
        m_qrCode->setData(m_pairingManager->getQrCodeData());
        
        // Start countdown
        m_timer->start(1000);
        updateCountdown();
    } else {
        // Focus first input
        if (m_pinInput[0]) {
            m_pinInput[0]->setFocus();
        }
    }
}

void PairingDialog::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw rounded background
    QRect bgRect = rect().adjusted(10, 10, -10, -10);
    
    // Shadow
    for (int i = 10; i > 0; --i) {
        QColor shadow(0, 0, 0, 20 - i * 2);
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadow);
        painter.drawRoundedRect(bgRect.adjusted(-i, -i, i, i), 24 + i, 24 + i);
    }
    
    // Background gradient
    QLinearGradient bg(bgRect.topLeft(), bgRect.bottomRight());
    bg.setColorAt(0, QColor(15, 23, 42));
    bg.setColorAt(1, QColor(8, 12, 21));
    painter.setBrush(bg);
    painter.setPen(QPen(QColor(255, 255, 255, 20), 1));
    painter.drawRoundedRect(bgRect, 24, 24);
}

void PairingDialog::setupShowPinUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(16);
    
    // Title
    QLabel* title = new QLabel("Ready to Connect");
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    
    // Subtitle
    QLabel* subtitle = new QLabel("Ask the other person to enter this PIN\nor scan the QR code");
    subtitle->setObjectName("dialogSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    
    layout->addSpacing(16);
    
    // PIN Display
    m_pinLabel = new QLabel("--- ---");
    m_pinLabel->setObjectName("pinDisplay");
    m_pinLabel->setAlignment(Qt::AlignCenter);
    
    QGraphicsDropShadowEffect* pinGlow = new QGraphicsDropShadowEffect();
    pinGlow->setBlurRadius(30);
    pinGlow->setColor(QColor(6, 182, 212, 150));
    pinGlow->setOffset(0, 0);
    m_pinLabel->setGraphicsEffect(pinGlow);
    
    layout->addWidget(m_pinLabel);
    
    // Countdown
    m_countdownLabel = new QLabel("Expires in 5:00");
    m_countdownLabel->setObjectName("countdownLabel");
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_countdownLabel);
    
    layout->addSpacing(8);
    
    // Divider with "or"
    QHBoxLayout* dividerLayout = new QHBoxLayout();
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("background: rgba(255,255,255,0.1);");
    dividerLayout->addWidget(line1);
    
    QLabel* orLabel = new QLabel("or scan");
    orLabel->setObjectName("orLabel");
    dividerLayout->addWidget(orLabel);
    
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("background: rgba(255,255,255,0.1);");
    dividerLayout->addWidget(line2);
    
    layout->addLayout(dividerLayout);
    
    layout->addSpacing(8);
    
    // QR Code
    m_qrCode = new QRCodeWidget();
    m_qrCode->setCodeSize(150);
    m_qrCode->setColors(Qt::white, QColor(15, 23, 42));
    m_qrCode->setFixedSize(150, 150);
    
    QHBoxLayout* qrLayout = new QHBoxLayout();
    qrLayout->addStretch();
    qrLayout->addWidget(m_qrCode);
    qrLayout->addStretch();
    layout->addLayout(qrLayout);
    
    layout->addStretch();
    
    // Cancel button
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    m_cancelButton->setMinimumHeight(48);
    connect(m_cancelButton, &QPushButton::clicked, this, &PairingDialog::onCancelClicked);
    layout->addWidget(m_cancelButton);
}

void PairingDialog::setupEnterPinUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(16);

    // Title
    QLabel* title = new QLabel("Enter PIN");
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Device name
    m_deviceNameLabel = new QLabel("Connecting to device...");
    m_deviceNameLabel->setObjectName("dialogSubtitle");
    m_deviceNameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_deviceNameLabel);

    layout->addSpacing(32);

    // PIN Input boxes
    QHBoxLayout* pinLayout = new QHBoxLayout();
    pinLayout->setSpacing(8);
    pinLayout->addStretch();

    for (int i = 0; i < 6; ++i) {
        m_pinInput[i] = new QLineEdit();
        m_pinInput[i]->setObjectName("pinInputBox");
        m_pinInput[i]->setMaxLength(1);
        m_pinInput[i]->setAlignment(Qt::AlignCenter);
        m_pinInput[i]->setFixedSize(48, 64);

        connect(m_pinInput[i], &QLineEdit::textChanged, this, &PairingDialog::onPinDigitEntered);

        pinLayout->addWidget(m_pinInput[i]);

        // Add dash after 3rd digit
        if (i == 2) {
            QLabel* dash = new QLabel("-");
            dash->setObjectName("pinDash");
            pinLayout->addWidget(dash);
        }
    }

    pinLayout->addStretch();
    layout->addLayout(pinLayout);

    // Helper text
    QLabel* helper = new QLabel("Enter the 6-digit PIN shown on the other device");
    helper->setObjectName("helperText");
    helper->setAlignment(Qt::AlignCenter);
    helper->setWordWrap(true);
    layout->addWidget(helper);

    // Add QR paste option
    QHBoxLayout* qrLayout = new QHBoxLayout();
    qrLayout->setSpacing(8);
    qrLayout->addStretch();

    QLabel* qrIcon = new QLabel("📱");
    qrIcon->setObjectName("qrIcon");
    qrLayout->addWidget(qrIcon);

    QLabel* qrText = new QLabel("Scanned QR code?");
    qrText->setObjectName("qrText");
    qrLayout->addWidget(qrText);

    QPushButton* pasteQrButton = new QPushButton("Paste");
    pasteQrButton->setObjectName("pasteQrButton");
    pasteQrButton->setCursor(Qt::PointingHandCursor);
    pasteQrButton->setFixedSize(60, 32);
    connect(pasteQrButton, &QPushButton::clicked, this, &PairingDialog::onPasteQrClicked);
    qrLayout->addWidget(pasteQrButton);

    qrLayout->addStretch();
    layout->addLayout(qrLayout);

    layout->addStretch();

    // Buttons
    m_verifyButton = new QPushButton("Connect");
    m_verifyButton->setObjectName("primaryButton");
    m_verifyButton->setCursor(Qt::PointingHandCursor);
    m_verifyButton->setMinimumHeight(52);
    m_verifyButton->setEnabled(false);
    connect(m_verifyButton, &QPushButton::clicked, this, &PairingDialog::onVerifyClicked);
    layout->addWidget(m_verifyButton);

    layout->addSpacing(8);

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    m_cancelButton->setMinimumHeight(44);
    connect(m_cancelButton, &QPushButton::clicked, this, &PairingDialog::onCancelClicked);
    layout->addWidget(m_cancelButton);
}

void PairingDialog::onTimerTick()
{
    updateCountdown();
    
    if (!m_pairingManager->isPinValid()) {
        m_timer->stop();
        reject();
    }
}

void PairingDialog::updateCountdown()
{
    int remaining = m_pairingManager->pinTimeRemaining();
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    
    m_countdownLabel->setText(QString("Expires in %1:%2")
        .arg(minutes).arg(seconds, 2, 10, QChar('0')));
    
    // Change color when low
    if (remaining < 60) {
        m_countdownLabel->setStyleSheet("color: #f59e0b;");
    }
    if (remaining < 30) {
        m_countdownLabel->setStyleSheet("color: #ef4444;");
    }
}

void PairingDialog::onPinDigitEntered(const QString& text)
{
    QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
    if (!sender) return;
    
    // Find which input this is
    int currentIndex = -1;
    for (int i = 0; i < 6; ++i) {
        if (m_pinInput[i] == sender) {
            currentIndex = i;
            break;
        }
    }
    
    // Auto-advance to next box
    if (!text.isEmpty() && currentIndex < 5) {
        m_pinInput[currentIndex + 1]->setFocus();
    }
    
    // Check if all filled
    QString fullPin = enteredPin();
    m_verifyButton->setEnabled(fullPin.length() == 6);
}

void PairingDialog::onVerifyClicked()
{
    QString pin = enteredPin();
    
    if (pin.length() != 6) {
        return;
    }
    
    // Verify with pairing manager
    if (m_pairingManager->verifyPin(pin, m_remoteDeviceId, m_remoteDeviceName)) {
        m_pairingSuccessful = true;
        accept();
    } else {
        // Show error - shake the inputs
        for (int i = 0; i < 6; ++i) {
            m_pinInput[i]->setStyleSheet("border-color: #ef4444;");
            m_pinInput[i]->clear();
        }
        m_pinInput[0]->setFocus();
        
        // Reset style after delay
        QTimer::singleShot(1000, this, [this]() {
            applyStylesheet();
        });
    }
}

void PairingDialog::onPasteQrClicked()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString qrData = clipboard->text();

    if (qrData.isEmpty()) {
        QMessageBox::information(this, tr("RedkaConnect"),
            tr("No text found in clipboard. Try copying the scanned QR code text first."));
        return;
    }

    // Try to parse the QR data
    QString deviceId, deviceName, pin, address;
    if (m_pairingManager->parseQrCodeData(qrData, deviceId, deviceName, pin, address)) {
        // Success! Auto-fill the PIN
        if (pin.length() == 6) {
            for (int i = 0; i < 6; ++i) {
                m_pinInput[i]->setText(pin.mid(i, 1));
            }

            // Update device info if available
            if (!deviceName.isEmpty()) {
                setRemoteDevice(deviceId, deviceName);
            }

            // Auto-verify after a short delay
            QTimer::singleShot(500, this, [this]() {
                onVerifyClicked();
            });

            QMessageBox::information(this, tr("RedkaConnect"),
                tr("QR code scanned successfully! PIN auto-filled."));
        }
    } else {
        QMessageBox::warning(this, tr("RedkaConnect"),
            tr("Invalid QR code data. Make sure you scanned the correct QR code and copied the text properly."));
    }
}

void PairingDialog::onCancelClicked()
{
    if (m_mode == Mode::ShowPin) {
        m_pairingManager->cancelPairing();
    }
    reject();
}

void PairingDialog::applyStylesheet()
{
    QString css = R"(
        #dialogTitle {
            font-size: 24px;
            font-weight: 600;
            color: #f1f5f9;
        }
        
        #dialogSubtitle {
            font-size: 14px;
            color: #94a3b8;
        }
        
        #pinDisplay {
            font-size: 56px;
            font-weight: 700;
            color: #22d3ee;
            letter-spacing: 12px;
            font-family: 'JetBrains Mono', 'Cascadia Code', monospace;
        }
        
        #countdownLabel {
            font-size: 14px;
            color: #10b981;
        }
        
        #orLabel {
            font-size: 13px;
            color: #64748b;
            padding: 0 12px;
        }
        
        #pinInputBox {
            background: rgba(30, 41, 59, 0.8);
            border: 2px solid rgba(100, 116, 139, 0.3);
            border-radius: 12px;
            font-size: 28px;
            font-weight: 600;
            color: #f1f5f9;
            font-family: 'JetBrains Mono', monospace;
        }
        
        #pinInputBox:focus {
            border-color: #06b6d4;
            background: rgba(30, 41, 59, 1);
        }
        
        #pinDash {
            font-size: 24px;
            color: #64748b;
        }
        
        #helperText {
            font-size: 13px;
            color: #64748b;
        }
        
        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #0891b2, stop:1 #06b6d4);
            border: none;
            border-radius: 12px;
            color: #0c1220;
            font-size: 16px;
            font-weight: 600;
        }
        
        #primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #06b6d4, stop:1 #22d3ee);
        }
        
        #primaryButton:disabled {
            background: rgba(100, 116, 139, 0.3);
            color: #64748b;
        }
        
        #cancelButton {
            background: transparent;
            border: 1px solid rgba(100, 116, 139, 0.3);
            border-radius: 12px;
            color: #94a3b8;
            font-size: 14px;
            font-weight: 500;
        }
        
        #cancelButton:hover {
            background: rgba(100, 116, 139, 0.15);
            border-color: rgba(100, 116, 139, 0.5);
        }

        #qrIcon {
            font-size: 16px;
            color: #94a3b8;
        }

        #qrText {
            font-size: 13px;
            color: #64748b;
        }

        #pasteQrButton {
            background: rgba(30, 41, 59, 0.8);
            border: 1px solid rgba(100, 116, 139, 0.3);
            border-radius: 6px;
            color: #94a3b8;
            font-size: 12px;
            font-weight: 500;
        }

        #pasteQrButton:hover {
            background: rgba(100, 116, 139, 0.15);
            border-color: rgba(100, 116, 139, 0.5);
        }
    )";
    
    setStyleSheet(css);
}
