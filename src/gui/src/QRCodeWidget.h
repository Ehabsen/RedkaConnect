/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QWidget>
#include <QString>
#include <QImage>
#include <QVector>

/**
 * @brief Real QR Code generator widget
 * 
 * Implements QR Code generation using the QR Code Model 2 specification.
 * This is a simplified implementation suitable for small data payloads
 * (up to ~100 characters) using error correction level L.
 * 
 * For production use with larger data or camera scanning,
 * consider integrating ZXing-cpp or QZXing library.
 */
class QRCodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRCodeWidget(QWidget* parent = nullptr);
    ~QRCodeWidget() override;

    /**
     * @brief Set the data to encode in the QR code
     * @param data Text to encode (max ~100 chars for reliable scanning)
     */
    void setData(const QString& data);

    QString data() const { return m_data; }
    void setCodeSize(int size);
    void setColors(const QColor& foreground, const QColor& background);
    void setQuietZone(int modules);

    /**
     * @brief Get the generated QR code as an image
     */
    QImage toImage(int size = 0) const;

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    // QR Code generation
    void generateQRCode();
    int selectVersion(int dataLength);
    QVector<bool> encodeData(const QString& data);
    void addErrorCorrection(QVector<bool>& data);
    void placeModules(const QVector<bool>& data);
    void addFinderPatterns();
    void addAlignmentPatterns();
    void addTimingPatterns();
    void addFormatInfo();
    void applyMask();
    
    // Reed-Solomon error correction
    QVector<int> calculateECC(const QVector<int>& data, int eccCount);
    int gfMultiply(int a, int b);
    
    // Utility
    void setModule(int x, int y, bool black);
    bool getModule(int x, int y) const;
    
    QString m_data;
    int m_version;           // QR version (1-40)
    int m_moduleCount;       // Modules per side
    QVector<QVector<bool>> m_modules;
    QVector<QVector<bool>> m_isFunction;  // Track function patterns
    
    int m_displaySize;
    int m_quietZone;
    QColor m_foregroundColor;
    QColor m_backgroundColor;
    
    // Galois field tables for Reed-Solomon
    static const int GF_EXP[512];
    static const int GF_LOG[256];
};
