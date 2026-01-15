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
#include <QPoint>
#include <QString>
#include <QRect>
#include <QPropertyAnimation>

/**
 * @brief Visual screen arrangement widget
 * 
 * Displays monitors as draggable rectangles that users can
 * position relative to each other. The local screen is fixed
 * in the center, and the remote screen can be dragged to any
 * of the four sides.
 */
class ScreenArrangementWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF remoteScreenPos READ remoteScreenPos WRITE setRemoteScreenPos)

public:
    enum class ScreenPosition {
        Left,
        Right,
        Top,
        Bottom
    };
    Q_ENUM(ScreenPosition)

    explicit ScreenArrangementWidget(QWidget* parent = nullptr);
    ~ScreenArrangementWidget() override;

    void setLocalScreenName(const QString& name);
    void setRemoteScreenName(const QString& name);
    void setScreenPosition(ScreenPosition position);
    ScreenPosition screenPosition() const { return m_position; }

    QPointF remoteScreenPos() const { return m_remoteScreenPos; }
    void setRemoteScreenPos(const QPointF& pos);

signals:
    void positionChanged(ScreenPosition position);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateScreenRects();
    void snapToPosition();
    ScreenPosition positionFromPoint(const QPointF& center) const;
    QPointF positionToPoint(ScreenPosition pos) const;
    void animateToPosition(ScreenPosition pos);

    void drawScreen(QPainter& painter, const QRectF& rect, const QString& name, 
                    bool isLocal, bool isHovered, bool isDragging);
    void drawConnectionLine(QPainter& painter);
    void drawDropZones(QPainter& painter);

    QString m_localName;
    QString m_remoteName;
    ScreenPosition m_position;

    QRectF m_localScreenRect;
    QRectF m_remoteScreenRect;
    QPointF m_remoteScreenPos;
    
    bool m_isDragging;
    QPointF m_dragOffset;
    bool m_isHoveringRemote;

    QPropertyAnimation* m_snapAnimation;
    
    // Visual constants
    static constexpr int SCREEN_WIDTH = 120;
    static constexpr int SCREEN_HEIGHT = 75;
    static constexpr int SCREEN_SPACING = 20;
    static constexpr int SCREEN_CORNER_RADIUS = 8;
};
