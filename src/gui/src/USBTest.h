/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

class USBConnectionManager;

/**
 * @brief USB Connection Test Window
 *
 * Simple test interface to demonstrate USB cable connectivity.
 * Shows available USB devices, allows connection testing,
 * and provides basic data exchange demonstration.
 */
class USBTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit USBTest(QWidget* parent = nullptr);
    ~USBTest() override;

private slots:
    void onDeviceDiscovered(const USBConnectionManager::USBDevice& device);
    void onDeviceRemoved(const QString& portName);
    void onConnected(const QString& portName);
    void onDisconnected();
    void onDataReceived(const QByteArray& data);
    void onError(const QString& error);

    void onConnectClicked();
    void onDisconnectClicked();
    void onSendTestClicked();
    void onRefreshClicked();

private:
    void setupUi();
    void updateStatus(const QString& status);

    USBConnectionManager* m_usbManager;

    // UI Components
    QListWidget* m_deviceList;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    QPushButton* m_sendTestButton;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
    QTextEdit* m_logText;
};