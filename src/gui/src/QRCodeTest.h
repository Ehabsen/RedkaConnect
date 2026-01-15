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

class QRCodeWidget;
class PairingManager;

/**
 * @brief Simple test window to demonstrate QR code generation and pasting
 *
 * This window shows:
 * - A QR code widget with generated pairing data
 * - A text field showing the raw JSON data
 * - Copy button to copy the JSON to clipboard
 * - A pairing dialog to test pasting
 *
 * Usage:
 * 1. Copy the JSON text
 * 2. Click "Test Paste" to open the pairing dialog
 * 3. Click "Paste" in the pairing dialog
 * 4. See the PIN auto-fill
 */
class QRCodeTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit QRCodeTest(QWidget* parent = nullptr);
    ~QRCodeTest() override;

private slots:
    void onCopyClicked();
    void onTestPasteClicked();
    void onGenerateNewClicked();

private:
    void setupUi();
    void generateTestData();

    QRCodeWidget* m_qrCode;
    PairingManager* m_pairingManager;
    QLabel* m_jsonLabel;
    QPushButton* m_copyButton;
    QPushButton* m_testPasteButton;
    QPushButton* m_generateNewButton;
};