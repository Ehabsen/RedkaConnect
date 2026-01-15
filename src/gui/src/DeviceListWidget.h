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
#include <QList>
#include <QString>
#include <QTimer>

class QVBoxLayout;
class QLabel;

/**
 * @brief Represents a discovered device on the network
 */
struct DiscoveredDevice {
    QString name;
    QString address;
    int port;
    bool isServer;  // true = sharing, false = wanting to connect
    qint64 discoveredAt;
};

/**
 * @brief Widget that displays discovered devices
 * 
 * Shows auto-discovered InputLeap instances on the network
 * with a beautiful card-based design.
 */
class DeviceListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceListWidget(QWidget* parent = nullptr);
    ~DeviceListWidget() override;

    void addDevice(const DiscoveredDevice& device);
    void removeDevice(const QString& address);
    void clear();
    int deviceCount() const;

signals:
    void deviceSelected(const DiscoveredDevice& device);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void rebuildList();
    QWidget* createDeviceCard(const DiscoveredDevice& device);

    QVBoxLayout* m_layout;
    QLabel* m_emptyLabel;
    QList<DiscoveredDevice> m_devices;
    QTimer* m_cleanupTimer;
};

/**
 * @brief Individual device card widget
 */
class DeviceCardWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    explicit DeviceCardWidget(const DiscoveredDevice& device, QWidget* parent = nullptr);

    qreal hoverProgress() const { return m_hoverProgress; }
    void setHoverProgress(qreal progress);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    DiscoveredDevice m_device;
    qreal m_hoverProgress;
    bool m_pressed;
};
