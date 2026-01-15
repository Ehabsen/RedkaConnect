/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "USBTest.h"
#include "USBConnectionManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QApplication>
#include <QDateTime>

USBTest::USBTest(QWidget* parent)
    : QMainWindow(parent)
    , m_usbManager(new USBConnectionManager(this))
{
    setWindowTitle("USB Connection Test - RedkaConnect");
    setMinimumSize(600, 400);

    setupUi();

    // Connect USB manager signals
    connect(m_usbManager, &USBConnectionManager::deviceDiscovered,
            this, &USBTest::onDeviceDiscovered);
    connect(m_usbManager, &USBConnectionManager::deviceRemoved,
            this, &USBTest::onDeviceRemoved);
    connect(m_usbManager, &USBConnectionManager::connected,
            this, &USBTest::onConnected);
    connect(m_usbManager, &USBConnectionManager::disconnected,
            this, &USBTest::onDisconnected);
    connect(m_usbManager, &USBConnectionManager::dataReceived,
            this, &USBTest::onDataReceived);
    connect(m_usbManager, &USBConnectionManager::errorOccurred,
            this, &USBTest::onError);

    // Start discovery
    m_usbManager->startAutoDiscovery();
    updateStatus("Ready - Plug in a USB cable to another RedkaConnect device");
}

USBTest::~USBTest() = default;

void USBTest::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel* title = new QLabel("🔌 USB Connection Test");
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(title);

    // Status
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setStyleSheet("font-size: 14px; color: #666; padding: 5px; background-color: #f0f0f0; border-radius: 3px;");
    mainLayout->addWidget(m_statusLabel);

    // Device list section
    QGroupBox* deviceGroup = new QGroupBox("Available USB Devices");
    QVBoxLayout* deviceLayout = new QVBoxLayout(deviceGroup);

    m_deviceList = new QListWidget();
    m_deviceList->setMaximumHeight(150);
    deviceLayout->addWidget(m_deviceList);

    QHBoxLayout* deviceButtonLayout = new QHBoxLayout();
    m_connectButton = new QPushButton("🔌 Connect");
    m_connectButton->setEnabled(false);
    connect(m_connectButton, &QPushButton::clicked, this, &USBTest::onConnectClicked);

    m_refreshButton = new QPushButton("🔄 Refresh");
    connect(m_refreshButton, &QPushButton::clicked, this, &USBTest::onRefreshClicked);

    deviceButtonLayout->addWidget(m_connectButton);
    deviceButtonLayout->addWidget(m_refreshButton);
    deviceButtonLayout->addStretch();

    deviceLayout->addLayout(deviceButtonLayout);
    mainLayout->addWidget(deviceGroup);

    // Connection controls
    QGroupBox* connectionGroup = new QGroupBox("Connection Test");
    QVBoxLayout* connectionLayout = new QVBoxLayout(connectionGroup);

    QHBoxLayout* connectionButtonLayout = new QHBoxLayout();
    m_disconnectButton = new QPushButton("❌ Disconnect");
    m_disconnectButton->setEnabled(false);
    connect(m_disconnectButton, &QPushButton::clicked, this, &USBTest::onDisconnectClicked);

    m_sendTestButton = new QPushButton("📤 Send Test Message");
    m_sendTestButton->setEnabled(false);
    connect(m_sendTestButton, &QPushButton::clicked, this, &USBTest::onSendTestClicked);

    connectionButtonLayout->addWidget(m_disconnectButton);
    connectionButtonLayout->addWidget(m_sendTestButton);
    connectionButtonLayout->addStretch();

    connectionLayout->addLayout(connectionButtonLayout);
    mainLayout->addWidget(connectionGroup);

    // Log
    QGroupBox* logGroup = new QGroupBox("Communication Log");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);

    m_logText = new QTextEdit();
    m_logText->setMaximumHeight(150);
    m_logText->setReadOnly(true);
    logLayout->addWidget(m_logText);

    mainLayout->addWidget(logGroup);

    // Instructions
    QLabel* instructions = new QLabel(
        "Instructions:\n"
        "1. Connect a USB cable between this computer and another running RedkaConnect\n"
        "2. The other computer should be in 'Share This Computer' mode\n"
        "3. Select a device from the list above and click 'Connect'\n"
        "4. Try sending a test message to verify the connection"
    );
    instructions->setWordWrap(true);
    instructions->setStyleSheet("font-size: 12px; color: #666; padding: 10px; background-color: #f9f9f9; border-radius: 5px;");
    mainLayout->addWidget(instructions);
}

void USBTest::onDeviceDiscovered(const USBConnectionManager::USBDevice& device)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(QString("🔌 %1 (%2)").arg(device.description, device.portName));
    item->setData(Qt::UserRole, device.portName);
    m_deviceList->addItem(item);

    m_connectButton->setEnabled(true);
    updateStatus(QString("Found USB device: %1").arg(device.description));
    logMessage(QString("Device discovered: %1 on port %2").arg(device.description, device.portName));
}

void USBTest::onDeviceRemoved(const QString& portName)
{
    // Remove from list
    for (int i = 0; i < m_deviceList->count(); ++i) {
        QListWidgetItem* item = m_deviceList->item(i);
        if (item->data(Qt::UserRole).toString() == portName) {
            delete m_deviceList->takeItem(i);
            break;
        }
    }

    if (m_deviceList->count() == 0) {
        m_connectButton->setEnabled(false);
    }

    updateStatus("USB device removed");
    logMessage(QString("Device removed: %1").arg(portName));
}

void USBTest::onConnected(const QString& portName)
{
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_sendTestButton->setEnabled(true);
    updateStatus(QString("Connected to USB device on port %1").arg(portName));
    logMessage(QString("Connected to port: %1").arg(portName));
}

void USBTest::onDisconnected()
{
    m_disconnectButton->setEnabled(false);
    m_sendTestButton->setEnabled(false);
    m_connectButton->setEnabled(m_deviceList->count() > 0);
    updateStatus("Disconnected from USB device");
    logMessage("Disconnected");
}

void USBTest::onDataReceived(const QByteArray& data)
{
    QString message = QString("Received: %1").arg(QString::fromUtf8(data));
    logMessage(message);
}

void USBTest::onError(const QString& error)
{
    updateStatus(QString("Error: %1").arg(error));
    logMessage(QString("Error: %1").arg(error));
}

void USBTest::onConnectClicked()
{
    QListWidgetItem* currentItem = m_deviceList->currentItem();
    if (!currentItem) {
        updateStatus("Please select a USB device first");
        return;
    }

    QString portName = currentItem->data(Qt::UserRole).toString();
    updateStatus(QString("Connecting to %1...").arg(portName));
    logMessage(QString("Attempting to connect to port: %1").arg(portName));

    if (!m_usbManager->connectToDevice(portName)) {
        updateStatus("Connection failed - check that the other device is running RedkaConnect");
        logMessage("Connection failed");
    }
}

void USBTest::onDisconnectClicked()
{
    m_usbManager->disconnect();
}

void USBTest::onSendTestClicked()
{
    QString testMessage = QString("Test message from USB Test at %1").arg(QDateTime::currentDateTime().toString());
    QByteArray data = testMessage.toUtf8();

    if (m_usbManager->sendData(data)) {
        logMessage(QString("Sent: %1").arg(testMessage));
    } else {
        logMessage("Failed to send test message");
    }
}

void USBTest::onRefreshClicked()
{
    m_deviceList->clear();
    m_usbManager->refreshDevices();
    updateStatus("Refreshing USB device list...");
}

void USBTest::updateStatus(const QString& status)
{
    m_statusLabel->setText(status);
}

void USBTest::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logText->append(QString("[%1] %2").arg(timestamp, message));
}