/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "DeviceListWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QDateTime>

// ============ DeviceListWidget ============

DeviceListWidget::DeviceListWidget(QWidget* parent)
    : QWidget(parent)
    , m_cleanupTimer(new QTimer(this))
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(12);

    m_emptyLabel = new QLabel("🔍 Searching for RedkaConnect devices...");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(R"(
        QLabel {
            color: #6e7681;
            font-size: 14px;
            padding: 40px;
        }
    )");
    m_layout->addWidget(m_emptyLabel);
    m_layout->addStretch();

    // Cleanup old devices every 10 seconds
    connect(m_cleanupTimer, &QTimer::timeout, this, [this]() {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        QList<QString> toRemove;
        for (const auto& device : m_devices) {
            if (now - device.discoveredAt > 30000) {  // 30 second timeout
                toRemove.append(device.address);
            }
        }
        for (const auto& addr : toRemove) {
            removeDevice(addr);
        }
    });
    m_cleanupTimer->start(10000);
}

DeviceListWidget::~DeviceListWidget() = default;

void DeviceListWidget::addDevice(const DiscoveredDevice& device)
{
    // Update if exists
    for (auto& existing : m_devices) {
        if (existing.address == device.address) {
            existing.discoveredAt = QDateTime::currentMSecsSinceEpoch();
            existing.name = device.name;
            existing.isServer = device.isServer;
            return;
        }
    }

    // Add new device
    DiscoveredDevice newDevice = device;
    newDevice.discoveredAt = QDateTime::currentMSecsSinceEpoch();
    m_devices.append(newDevice);
    rebuildList();
}

void DeviceListWidget::removeDevice(const QString& address)
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].address == address) {
            m_devices.removeAt(i);
            rebuildList();
            return;
        }
    }
}

void DeviceListWidget::clear()
{
    m_devices.clear();
    rebuildList();
}

int DeviceListWidget::deviceCount() const
{
    return m_devices.size();
}

void DeviceListWidget::rebuildList()
{
    // Remove all widgets except empty label
    QLayoutItem* item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyLabel) {
            delete item->widget();
        }
        delete item;
    }

    if (m_devices.isEmpty()) {
        m_emptyLabel->show();
        m_layout->addWidget(m_emptyLabel);
    } else {
        m_emptyLabel->hide();
        for (const auto& device : m_devices) {
            auto* card = new DeviceCardWidget(device, this);
            connect(card, &DeviceCardWidget::clicked, this, [this, device]() {
                emit deviceSelected(device);
            });
            m_layout->addWidget(card);
        }
    }
    m_layout->addStretch();
}

void DeviceListWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    // Transparent background
}

// ============ DeviceCardWidget ============

DeviceCardWidget::DeviceCardWidget(const DiscoveredDevice& device, QWidget* parent)
    : QWidget(parent)
    , m_device(device)
    , m_hoverProgress(0.0)
    , m_pressed(false)
{
    setFixedHeight(80);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
}

void DeviceCardWidget::setHoverProgress(qreal progress)
{
    m_hoverProgress = progress;
    update();
}

void DeviceCardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect().adjusted(2, 2, -2, -2);

    // Background with hover effect
    QColor bgColor = QColor(22, 27, 34);
    QColor borderColor = QColor(48, 54, 61);
    
    if (m_hoverProgress > 0) {
        bgColor = QColor(
            bgColor.red() + (30 - bgColor.red()) * m_hoverProgress * 0.3,
            bgColor.green() + (35 - bgColor.green()) * m_hoverProgress * 0.3,
            bgColor.blue() + (42 - bgColor.blue()) * m_hoverProgress * 0.3
        );
        borderColor = QColor(95, 184, 143, 100 + 100 * m_hoverProgress);
    }

    if (m_pressed) {
        bgColor = bgColor.darker(110);
    }

    // Draw card
    QPainterPath path;
    path.addRoundedRect(rect, 12, 12);
    
    painter.fillPath(path, bgColor);
    painter.setPen(QPen(borderColor, 1.5));
    painter.drawPath(path);

    // Icon
    QRectF iconRect(rect.left() + 16, rect.center().y() - 20, 40, 40);
    QString icon = m_device.isServer ? "🖥️" : "💻";
    painter.setFont(QFont("Segoe UI Emoji", 20));
    painter.drawText(iconRect, Qt::AlignCenter, icon);

    // Device name
    painter.setPen(QColor(230, 237, 243));
    QFont nameFont("Segoe UI", 14, QFont::DemiBold);
    painter.setFont(nameFont);
    QRectF nameRect(iconRect.right() + 12, rect.top() + 16, rect.width() - iconRect.width() - 80, 24);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, m_device.name);

    // Address
    painter.setPen(QColor(139, 148, 158));
    QFont addrFont("Segoe UI", 11);
    painter.setFont(addrFont);
    QRectF addrRect(iconRect.right() + 12, nameRect.bottom(), nameRect.width(), 20);
    painter.drawText(addrRect, Qt::AlignLeft | Qt::AlignVCenter, m_device.address);

    // Status badge
    QString status = m_device.isServer ? "Sharing" : "Available";
    QColor badgeColor = m_device.isServer ? QColor(95, 184, 143) : QColor(108, 142, 191);
    
    QFont badgeFont("Segoe UI", 10, QFont::Medium);
    painter.setFont(badgeFont);
    QFontMetrics fm(badgeFont);
    int badgeWidth = fm.horizontalAdvance(status) + 16;
    
    QRectF badgeRect(rect.right() - badgeWidth - 16, rect.center().y() - 12, badgeWidth, 24);
    QPainterPath badgePath;
    badgePath.addRoundedRect(badgeRect, 12, 12);
    painter.fillPath(badgePath, QColor(badgeColor.red(), badgeColor.green(), badgeColor.blue(), 30));
    painter.setPen(badgeColor);
    painter.drawText(badgeRect, Qt::AlignCenter, status);
}

void DeviceCardWidget::enterEvent(QEvent* event)
{
    Q_UNUSED(event)
    auto* anim = new QPropertyAnimation(this, "hoverProgress", this);
    anim->setDuration(150);
    anim->setStartValue(m_hoverProgress);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DeviceCardWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    auto* anim = new QPropertyAnimation(this, "hoverProgress", this);
    anim->setDuration(200);
    anim->setStartValue(m_hoverProgress);
    anim->setEndValue(0.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DeviceCardWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
}

void DeviceCardWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        update();
        if (rect().contains(event->pos())) {
            emit clicked();
        }
    }
}
