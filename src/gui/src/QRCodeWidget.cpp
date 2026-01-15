/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "QRCodeWidget.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

// Galois Field exponent table (for Reed-Solomon)
const int QRCodeWidget::GF_EXP[512] = {
    1, 2, 4, 8, 16, 32, 64, 128, 29, 58, 116, 232, 205, 135, 19, 38,
    76, 152, 45, 90, 180, 117, 234, 201, 143, 3, 6, 12, 24, 48, 96, 192,
    157, 39, 78, 156, 37, 74, 148, 53, 106, 212, 181, 119, 238, 193, 159, 35,
    70, 140, 5, 10, 20, 40, 80, 160, 93, 186, 105, 210, 185, 111, 222, 161,
    95, 190, 97, 194, 153, 47, 94, 188, 101, 202, 137, 15, 30, 60, 120, 240,
    253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91, 182, 113, 226,
    217, 175, 67, 134, 17, 34, 68, 136, 13, 26, 52, 104, 208, 189, 103, 206,
    129, 31, 62, 124, 248, 237, 199, 147, 59, 118, 236, 197, 151, 51, 102, 204,
    133, 23, 46, 92, 184, 109, 218, 169, 79, 158, 33, 66, 132, 21, 42, 84,
    168, 77, 154, 41, 82, 164, 85, 170, 73, 146, 57, 114, 228, 213, 183, 115,
    230, 209, 191, 99, 198, 145, 63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
    227, 219, 171, 75, 150, 49, 98, 196, 149, 55, 110, 220, 165, 87, 174, 65,
    130, 25, 50, 100, 200, 141, 7, 14, 28, 56, 112, 224, 221, 167, 83, 166,
    81, 162, 89, 178, 121, 242, 249, 239, 195, 155, 43, 86, 172, 69, 138, 9,
    18, 36, 72, 144, 61, 122, 244, 245, 247, 243, 251, 235, 203, 139, 11, 22,
    44, 88, 176, 125, 250, 233, 207, 131, 27, 54, 108, 216, 173, 71, 142, 1,
    // Repeat for convenience
    2, 4, 8, 16, 32, 64, 128, 29, 58, 116, 232, 205, 135, 19, 38,
    76, 152, 45, 90, 180, 117, 234, 201, 143, 3, 6, 12, 24, 48, 96, 192,
    157, 39, 78, 156, 37, 74, 148, 53, 106, 212, 181, 119, 238, 193, 159, 35,
    70, 140, 5, 10, 20, 40, 80, 160, 93, 186, 105, 210, 185, 111, 222, 161,
    95, 190, 97, 194, 153, 47, 94, 188, 101, 202, 137, 15, 30, 60, 120, 240,
    253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91, 182, 113, 226,
    217, 175, 67, 134, 17, 34, 68, 136, 13, 26, 52, 104, 208, 189, 103, 206,
    129, 31, 62, 124, 248, 237, 199, 147, 59, 118, 236, 197, 151, 51, 102, 204,
    133, 23, 46, 92, 184, 109, 218, 169, 79, 158, 33, 66, 132, 21, 42, 84,
    168, 77, 154, 41, 82, 164, 85, 170, 73, 146, 57, 114, 228, 213, 183, 115,
    230, 209, 191, 99, 198, 145, 63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
    227, 219, 171, 75, 150, 49, 98, 196, 149, 55, 110, 220, 165, 87, 174, 65,
    130, 25, 50, 100, 200, 141, 7, 14, 28, 56, 112, 224, 221, 167, 83, 166,
    81, 162, 89, 178, 121, 242, 249, 239, 195, 155, 43, 86, 172, 69, 138, 9,
    18, 36, 72, 144, 61, 122, 244, 245, 247, 243, 251, 235, 203, 139, 11, 22,
    44, 88, 176, 125, 250, 233, 207, 131, 27, 54, 108, 216, 173, 71, 142, 1
};

// Galois Field logarithm table
const int QRCodeWidget::GF_LOG[256] = {
    -1, 0, 1, 25, 2, 50, 26, 198, 3, 223, 51, 238, 27, 104, 199, 75,
    4, 100, 224, 14, 52, 141, 239, 129, 28, 193, 105, 248, 200, 8, 76, 113,
    5, 138, 101, 47, 225, 36, 15, 33, 53, 147, 142, 218, 240, 18, 130, 69,
    29, 181, 194, 125, 106, 39, 249, 185, 201, 154, 9, 120, 77, 228, 114, 166,
    6, 191, 139, 98, 102, 221, 48, 253, 226, 152, 37, 179, 16, 145, 34, 136,
    54, 208, 148, 206, 143, 150, 219, 189, 241, 210, 19, 92, 131, 56, 70, 64,
    30, 66, 182, 163, 195, 72, 126, 110, 107, 58, 40, 84, 250, 133, 186, 61,
    202, 94, 155, 159, 10, 21, 121, 43, 78, 212, 229, 172, 115, 243, 167, 87,
    7, 112, 192, 247, 140, 128, 99, 13, 103, 74, 222, 237, 49, 197, 254, 24,
    227, 165, 153, 119, 38, 184, 180, 124, 17, 68, 146, 217, 35, 32, 137, 46,
    55, 63, 209, 91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190, 97,
    242, 86, 211, 171, 20, 42, 93, 158, 132, 60, 57, 83, 71, 109, 65, 162,
    31, 45, 67, 216, 183, 123, 164, 118, 196, 23, 73, 236, 127, 12, 111, 246,
    108, 161, 59, 82, 41, 157, 85, 170, 251, 96, 134, 177, 187, 204, 62, 90,
    203, 89, 95, 176, 156, 169, 160, 81, 11, 245, 22, 235, 122, 117, 44, 215,
    79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168, 80, 88, 175
};

QRCodeWidget::QRCodeWidget(QWidget* parent)
    : QWidget(parent)
    , m_version(2)
    , m_moduleCount(25)
    , m_displaySize(200)
    , m_quietZone(4)
    , m_foregroundColor(Qt::black)
    , m_backgroundColor(Qt::white)
{
    setMinimumSize(100, 100);
}

QRCodeWidget::~QRCodeWidget() = default;

void QRCodeWidget::setData(const QString& data)
{
    if (m_data != data) {
        m_data = data;
        generateQRCode();
        update();
    }
}

void QRCodeWidget::setCodeSize(int size)
{
    m_displaySize = size;
    setMinimumSize(size, size);
    update();
}

void QRCodeWidget::setColors(const QColor& foreground, const QColor& background)
{
    m_foregroundColor = foreground;
    m_backgroundColor = background;
    update();
}

void QRCodeWidget::setQuietZone(int modules)
{
    m_quietZone = modules;
    update();
}

QSize QRCodeWidget::sizeHint() const
{
    return QSize(m_displaySize, m_displaySize);
}

QSize QRCodeWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QImage QRCodeWidget::toImage(int size) const
{
    if (m_modules.isEmpty()) {
        return QImage();
    }
    
    int imgSize = size > 0 ? size : m_displaySize;
    int totalModules = m_moduleCount + 2 * m_quietZone;
    int moduleSize = imgSize / totalModules;
    int actualSize = moduleSize * totalModules;
    
    QImage image(actualSize, actualSize, QImage::Format_RGB32);
    image.fill(m_backgroundColor);
    
    for (int y = 0; y < m_moduleCount; ++y) {
        for (int x = 0; x < m_moduleCount; ++x) {
            if (getModule(x, y)) {
                int px = (x + m_quietZone) * moduleSize;
                int py = (y + m_quietZone) * moduleSize;
                for (int dy = 0; dy < moduleSize; ++dy) {
                    for (int dx = 0; dx < moduleSize; ++dx) {
                        image.setPixelColor(px + dx, py + dy, m_foregroundColor);
                    }
                }
            }
        }
    }
    
    return image;
}

void QRCodeWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Background
    painter.fillRect(rect(), m_backgroundColor);
    
    if (m_modules.isEmpty() || m_data.isEmpty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No Data");
        return;
    }
    
    // Calculate module size
    int availableSize = qMin(width(), height());
    int totalModules = m_moduleCount + 2 * m_quietZone;
    int moduleSize = availableSize / totalModules;
    int codeSize = moduleSize * totalModules;
    
    // Center the code
    int offsetX = (width() - codeSize) / 2;
    int offsetY = (height() - codeSize) / 2;
    
    // Draw modules
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_foregroundColor);
    
    for (int y = 0; y < m_moduleCount; ++y) {
        for (int x = 0; x < m_moduleCount; ++x) {
            if (getModule(x, y)) {
                int px = offsetX + (x + m_quietZone) * moduleSize;
                int py = offsetY + (y + m_quietZone) * moduleSize;
                painter.drawRect(px, py, moduleSize, moduleSize);
            }
        }
    }
}

int QRCodeWidget::selectVersion(int dataLength)
{
    // Simplified: use version 2 (25x25) for small data, version 3 (29x29) for larger
    // Version 2 can hold ~32 alphanumeric or ~20 bytes
    // Version 3 can hold ~53 alphanumeric or ~32 bytes
    if (dataLength <= 20) return 2;
    if (dataLength <= 32) return 3;
    if (dataLength <= 46) return 4;
    return 5;  // Up to ~60 bytes
}

void QRCodeWidget::generateQRCode()
{
    if (m_data.isEmpty()) {
        m_modules.clear();
        return;
    }
    
    // Select appropriate version
    QByteArray utf8Data = m_data.toUtf8();
    m_version = selectVersion(utf8Data.length());
    m_moduleCount = 17 + m_version * 4;
    
    // Initialize module grids
    m_modules.resize(m_moduleCount);
    m_isFunction.resize(m_moduleCount);
    for (int i = 0; i < m_moduleCount; ++i) {
        m_modules[i].resize(m_moduleCount);
        m_modules[i].fill(false);
        m_isFunction[i].resize(m_moduleCount);
        m_isFunction[i].fill(false);
    }
    
    // Add function patterns first
    addFinderPatterns();
    addTimingPatterns();
    if (m_version >= 2) {
        addAlignmentPatterns();
    }
    
    // Encode data
    QVector<bool> dataBits = encodeData(m_data);
    
    // Add error correction
    addErrorCorrection(dataBits);
    
    // Place data modules
    placeModules(dataBits);
    
    // Add format info
    addFormatInfo();
    
    // Apply mask (pattern 0)
    applyMask();
}

QVector<bool> QRCodeWidget::encodeData(const QString& data)
{
    QVector<bool> bits;
    QByteArray utf8 = data.toUtf8();
    
    // Mode indicator: Byte mode = 0100
    bits << false << true << false << false;
    
    // Character count (8 bits for version 1-9 in byte mode)
    int count = utf8.length();
    for (int i = 7; i >= 0; --i) {
        bits << ((count >> i) & 1);
    }
    
    // Data bytes
    for (int i = 0; i < utf8.length(); ++i) {
        unsigned char byte = static_cast<unsigned char>(utf8[i]);
        for (int j = 7; j >= 0; --j) {
            bits << ((byte >> j) & 1);
        }
    }
    
    // Terminator (up to 4 zeros)
    int capacity = 0;
    switch (m_version) {
        case 2: capacity = 272; break;  // 34 bytes * 8
        case 3: capacity = 440; break;  // 55 bytes * 8
        case 4: capacity = 640; break;  // 80 bytes * 8
        default: capacity = 864; break; // 108 bytes * 8
    }
    
    int terminatorLength = qMin(4, capacity - bits.size());
    for (int i = 0; i < terminatorLength; ++i) {
        bits << false;
    }
    
    // Pad to byte boundary
    while (bits.size() % 8 != 0) {
        bits << false;
    }
    
    // Pad with alternating bytes
    bool padByte = true;
    while (bits.size() < capacity) {
        unsigned char pad = padByte ? 0xEC : 0x11;
        for (int i = 7; i >= 0; --i) {
            bits << ((pad >> i) & 1);
        }
        padByte = !padByte;
    }
    
    return bits;
}

void QRCodeWidget::addErrorCorrection(QVector<bool>& dataBits)
{
    // Convert bits to bytes
    QVector<int> dataBytes;
    for (int i = 0; i < dataBits.size(); i += 8) {
        int byte = 0;
        for (int j = 0; j < 8 && i + j < dataBits.size(); ++j) {
            byte = (byte << 1) | (dataBits[i + j] ? 1 : 0);
        }
        dataBytes << byte;
    }
    
    // ECC codewords count (for version 2-5, L level)
    int eccCount = 0;
    switch (m_version) {
        case 2: eccCount = 10; break;
        case 3: eccCount = 15; break;
        case 4: eccCount = 20; break;
        default: eccCount = 26; break;
    }
    
    // Calculate ECC
    QVector<int> ecc = calculateECC(dataBytes, eccCount);
    
    // Append ECC to data bits
    for (int byte : ecc) {
        for (int i = 7; i >= 0; --i) {
            dataBits << ((byte >> i) & 1);
        }
    }
}

QVector<int> QRCodeWidget::calculateECC(const QVector<int>& data, int eccCount)
{
    // Generator polynomial coefficients for given ECC count
    static const int generators[][30] = {
        {0},  // placeholder
        {0},  // 1
        {0},  // 2
        {0},  // 3
        {0},  // 4
        {0},  // 5
        {0},  // 6
        {87, 229, 146, 149, 238, 102, 21},  // 7
        {0},  // 8
        {0},  // 9
        {251, 67, 46, 61, 118, 70, 64, 94, 32, 45},  // 10
        {0}, {0}, {0}, {0},
        {8, 183, 61, 91, 202, 37, 51, 58, 58, 237, 140, 124, 5, 99, 105},  // 15
        {0}, {0}, {0}, {0},
        {17, 60, 79, 50, 61, 163, 26, 187, 202, 180, 221, 225, 83, 239, 156, 164, 212, 212, 188, 190}  // 20
    };
    
    // Simple Reed-Solomon implementation
    QVector<int> result(eccCount, 0);
    
    for (int i = 0; i < data.size(); ++i) {
        int factor = data[i] ^ result[0];
        
        // Shift result
        for (int j = 0; j < eccCount - 1; ++j) {
            result[j] = result[j + 1];
        }
        result[eccCount - 1] = 0;
        
        // XOR with generator polynomial
        if (factor != 0) {
            int logFactor = GF_LOG[factor];
            for (int j = 0; j < eccCount && j < 30; ++j) {
                int coef = eccCount < 30 ? generators[eccCount][j] : 0;
                if (coef != 0) {
                    result[j] ^= GF_EXP[(logFactor + GF_LOG[GF_EXP[coef]]) % 255];
                }
            }
        }
    }
    
    return result;
}

int QRCodeWidget::gfMultiply(int a, int b)
{
    if (a == 0 || b == 0) return 0;
    return GF_EXP[GF_LOG[a] + GF_LOG[b]];
}

void QRCodeWidget::setModule(int x, int y, bool black)
{
    if (x >= 0 && x < m_moduleCount && y >= 0 && y < m_moduleCount) {
        m_modules[y][x] = black;
    }
}

bool QRCodeWidget::getModule(int x, int y) const
{
    if (x >= 0 && x < m_moduleCount && y >= 0 && y < m_moduleCount) {
        return m_modules[y][x];
    }
    return false;
}

void QRCodeWidget::addFinderPatterns()
{
    auto drawFinder = [this](int cx, int cy) {
        for (int dy = -4; dy <= 4; ++dy) {
            for (int dx = -4; dx <= 4; ++dx) {
                int x = cx + dx;
                int y = cy + dy;
                if (x < 0 || x >= m_moduleCount || y < 0 || y >= m_moduleCount) {
                    continue;
                }
                
                int dist = qMax(qAbs(dx), qAbs(dy));
                bool black = (dist <= 3 && dist != 2);
                
                setModule(x, y, black);
                m_isFunction[y][x] = true;
            }
        }
    };
    
    // Three finder patterns
    drawFinder(3, 3);                           // Top-left
    drawFinder(m_moduleCount - 4, 3);           // Top-right
    drawFinder(3, m_moduleCount - 4);           // Bottom-left
}

void QRCodeWidget::addTimingPatterns()
{
    for (int i = 8; i < m_moduleCount - 8; ++i) {
        bool black = (i % 2 == 0);
        
        // Horizontal
        if (!m_isFunction[6][i]) {
            setModule(i, 6, black);
            m_isFunction[6][i] = true;
        }
        
        // Vertical
        if (!m_isFunction[i][6]) {
            setModule(6, i, black);
            m_isFunction[i][6] = true;
        }
    }
}

void QRCodeWidget::addAlignmentPatterns()
{
    // Alignment pattern positions for versions 2-6
    static const int positions[][7] = {
        {6, 18},           // Version 2
        {6, 22},           // Version 3
        {6, 26},           // Version 4
        {6, 30},           // Version 5
        {6, 34}            // Version 6
    };
    
    if (m_version < 2 || m_version > 6) return;
    
    const int* pos = positions[m_version - 2];
    int count = 2;
    
    for (int i = 0; i < count; ++i) {
        for (int j = 0; j < count; ++j) {
            int cx = pos[i];
            int cy = pos[j];
            
            // Skip if overlaps with finder
            if ((cx <= 8 && cy <= 8) ||
                (cx <= 8 && cy >= m_moduleCount - 9) ||
                (cx >= m_moduleCount - 9 && cy <= 8)) {
                continue;
            }
            
            // Draw alignment pattern
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    int dist = qMax(qAbs(dx), qAbs(dy));
                    bool black = (dist != 1);
                    setModule(cx + dx, cy + dy, black);
                    m_isFunction[cy + dy][cx + dx] = true;
                }
            }
        }
    }
}

void QRCodeWidget::addFormatInfo()
{
    // Format info for mask 0, error correction L
    // Pre-calculated: 111011111000100
    int formatBits = 0x77C4;
    
    // Place format info
    for (int i = 0; i < 6; ++i) {
        bool bit = (formatBits >> i) & 1;
        setModule(8, i, bit);
        m_isFunction[i][8] = true;
        setModule(m_moduleCount - 1 - i, 8, bit);
    }
    
    setModule(8, 7, (formatBits >> 6) & 1);
    m_isFunction[7][8] = true;
    setModule(8, 8, (formatBits >> 7) & 1);
    m_isFunction[8][8] = true;
    setModule(7, 8, (formatBits >> 8) & 1);
    m_isFunction[8][7] = true;
    
    for (int i = 9; i < 15; ++i) {
        bool bit = (formatBits >> i) & 1;
        setModule(14 - i, 8, bit);
        m_isFunction[8][14 - i] = true;
        setModule(8, m_moduleCount - 15 + i, bit);
    }
    
    // Dark module
    setModule(8, m_moduleCount - 8, true);
    m_isFunction[m_moduleCount - 8][8] = true;
}

void QRCodeWidget::placeModules(const QVector<bool>& data)
{
    int bitIndex = 0;
    bool upward = true;
    
    for (int col = m_moduleCount - 1; col >= 0; col -= 2) {
        if (col == 6) col = 5;  // Skip timing pattern column
        
        for (int row = 0; row < m_moduleCount; ++row) {
            int actualRow = upward ? (m_moduleCount - 1 - row) : row;
            
            for (int c = 0; c < 2; ++c) {
                int x = col - c;
                
                if (!m_isFunction[actualRow][x]) {
                    if (bitIndex < data.size()) {
                        setModule(x, actualRow, data[bitIndex]);
                    }
                    bitIndex++;
                }
            }
        }
        upward = !upward;
    }
}

void QRCodeWidget::applyMask()
{
    // Mask pattern 0: (row + column) mod 2 == 0
    for (int y = 0; y < m_moduleCount; ++y) {
        for (int x = 0; x < m_moduleCount; ++x) {
            if (!m_isFunction[y][x]) {
                if ((x + y) % 2 == 0) {
                    m_modules[y][x] = !m_modules[y][x];
                }
            }
        }
    }
}
