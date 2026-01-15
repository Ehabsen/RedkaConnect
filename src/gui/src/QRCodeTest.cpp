/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "QRCodeTest.h"
#include "QRCodeWidget.h"
#include "PairingManager.h"
#include "PairingDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

QRCodeTest::QRCodeTest(QWidget* parent)
    : QMainWindow(parent)
    , m_qrCode(nullptr)
    , m_pairingManager(new PairingManager(this))
    , m_jsonLabel(nullptr)
    , m_copyButton(nullptr)
    , m_testPasteButton(nullptr)
    , m_generateNewButton(nullptr)
{
    setWindowTitle("QR Code Test - RedkaConnect");
    setMinimumSize(600, 500);

    m_pairingManager->initialize("test-device-123", "Test Computer");
    setupUi();
    generateTestData();
}

QRCodeTest::~QRCodeTest() = default;

void QRCodeTest::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel* title = new QLabel("QR Code Test");
    title->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(title);

    // QR Code section
    QGroupBox* qrGroup = new QGroupBox("Generated QR Code");
    QVBoxLayout* qrLayout = new QVBoxLayout(qrGroup);

    m_qrCode = new QRCodeWidget();
    m_qrCode->setCodeSize(200);
    qrLayout->addWidget(m_qrCode, 0, Qt::AlignCenter);

    mainLayout->addWidget(qrGroup);

    // JSON data section
    QGroupBox* jsonGroup = new QGroupBox("QR Code Data (JSON)");
    QVBoxLayout* jsonLayout = new QVBoxLayout(jsonGroup);

    m_jsonLabel = new QLabel();
    m_jsonLabel->setWordWrap(true);
    m_jsonLabel->setStyleSheet("font-family: monospace; background-color: #f5f5f5; padding: 10px; border: 1px solid #ddd; border-radius: 5px;");
    m_jsonLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    jsonLayout->addWidget(m_jsonLabel);

    QHBoxLayout* jsonButtonLayout = new QHBoxLayout();

    m_copyButton = new QPushButton("Copy JSON");
    m_copyButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; }"
                                "QPushButton:hover { background-color: #45a049; }");
    connect(m_copyButton, &QPushButton::clicked, this, &QRCodeTest::onCopyClicked);
    jsonButtonLayout->addWidget(m_copyButton);

    jsonButtonLayout->addStretch();

    jsonLayout->addLayout(jsonButtonLayout);
    mainLayout->addWidget(jsonGroup);

    // Test section
    QGroupBox* testGroup = new QGroupBox("Test QR Code Pasting");
    QVBoxLayout* testLayout = new QVBoxLayout(testGroup);

    QLabel* testDesc = new QLabel("1. Copy the JSON above to clipboard\n"
                                  "2. Click 'Test Paste' to open pairing dialog\n"
                                  "3. In the dialog, click the 'Paste' button\n"
                                  "4. See the PIN auto-fill from QR code data!");
    testDesc->setWordWrap(true);
    testLayout->addWidget(testDesc);

    QHBoxLayout* testButtonLayout = new QHBoxLayout();

    m_testPasteButton = new QPushButton("Test Paste");
    m_testPasteButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; border: none; padding: 10px 20px; border-radius: 4px; }"
                                     "QPushButton:hover { background-color: #1976D2; }");
    connect(m_testPasteButton, &QPushButton::clicked, this, &QRCodeTest::onTestPasteClicked);
    testButtonLayout->addWidget(m_testPasteButton);

    m_generateNewButton = new QPushButton("Generate New PIN");
    m_generateNewButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; border: none; padding: 10px 20px; border-radius: 4px; }"
                                       "QPushButton:hover { background-color: #F57C00; }");
    connect(m_generateNewButton, &QPushButton::clicked, this, &QRCodeTest::onGenerateNewClicked);
    testButtonLayout->addWidget(m_generateNewButton);

    testButtonLayout->addStretch();

    testLayout->addLayout(testButtonLayout);
    mainLayout->addWidget(testGroup);

    mainLayout->addStretch();
}

void QRCodeTest::generateTestData()
{
    // Generate a test PIN
    QString pin = m_pairingManager->generatePairingPin();

    // Get the QR code data
    QString qrData = m_pairingManager->getQrCodeData();

    // Update UI
    m_qrCode->setData(qrData);
    m_jsonLabel->setText(qrData);
}

void QRCodeTest::onCopyClicked()
{
    QApplication::clipboard()->setText(m_jsonLabel->text());
    QMessageBox::information(this, "Copied", "JSON data copied to clipboard!");
}

void QRCodeTest::onTestPasteClicked()
{
    // Create pairing dialog in "EnterPin" mode
    PairingDialog* dialog = new PairingDialog(m_pairingManager, PairingDialog::Mode::EnterPin);
    dialog->setRemoteDevice("test-device-123", "Test Computer");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void QRCodeTest::onGenerateNewClicked()
{
    generateTestData();
}