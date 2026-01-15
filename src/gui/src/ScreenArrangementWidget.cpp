/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "ScreenArrangementWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QtMath>

ScreenArrangementWidget::ScreenArrangementWidget(QWidget* parent)
    : QWidget(parent)
    , m_localName("This PC")
    , m_remoteName("Remote")
    , m_position(ScreenPosition::Right)
    , m_isDragging(false)
    , m_isHoveringRemote(false)
    , m_snapAnimation(new QPropertyAnimation(this, "remoteScreenPos", this))
{
    setMinimumSize(400, 250);
    setMouseTracking(true);
    
    m_snapAnimation->setDuration(300);
    m_snapAnimation->setEasingCurve(QEasingCurve::OutBack);
    
    updateScreenRects();
    m_remoteScreenPos = positionToPoint(m_position);
}

ScreenArrangementWidget::~ScreenArrangementWidget() = default;

void ScreenArrangementWidget::setLocalScreenName(const QString& name)
{
    m_localName = name;
    update();
}

void ScreenArrangementWidget::setRemoteScreenName(const QString& name)
{
    m_remoteName = name;
    update();
}

void ScreenArrangementWidget::setScreenPosition(ScreenPosition position)
{
    if (m_position != position) {
        m_position = position;
        animateToPosition(position);
        emit positionChanged(position);
    }
}

void ScreenArrangementWidget::setRemoteScreenPos(const QPointF& pos)
{
    m_remoteScreenPos = pos;
    m_remoteScreenRect.moveCenter(pos);
    update();
}

void ScreenArrangementWidget::updateScreenRects()
{
    QPointF center(width() / 2.0, height() / 2.0);
    
    m_localScreenRect = QRectF(
        center.x() - SCREEN_WIDTH / 2.0,
        center.y() - SCREEN_HEIGHT / 2.0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    m_remoteScreenRect = QRectF(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    m_remoteScreenRect.moveCenter(m_remoteScreenPos);
}

void ScreenArrangementWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateScreenRects();
    m_remoteScreenPos = positionToPoint(m_position);
    m_remoteScreenRect.moveCenter(m_remoteScreenPos);
}

QPointF ScreenArrangementWidget::positionToPoint(ScreenPosition pos) const
{
    QPointF center(width() / 2.0, height() / 2.0);
    
    switch (pos) {
        case ScreenPosition::Left:
            return QPointF(center.x() - SCREEN_WIDTH - SCREEN_SPACING, center.y());
        case ScreenPosition::Right:
            return QPointF(center.x() + SCREEN_WIDTH + SCREEN_SPACING, center.y());
        case ScreenPosition::Top:
            return QPointF(center.x(), center.y() - SCREEN_HEIGHT - SCREEN_SPACING);
        case ScreenPosition::Bottom:
            return QPointF(center.x(), center.y() + SCREEN_HEIGHT + SCREEN_SPACING);
    }
    return center;
}

ScreenArrangementWidget::ScreenPosition ScreenArrangementWidget::positionFromPoint(const QPointF& point) const
{
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF diff = point - center;
    
    // Determine which quadrant/direction
    if (qAbs(diff.x()) > qAbs(diff.y())) {
        return diff.x() < 0 ? ScreenPosition::Left : ScreenPosition::Right;
    } else {
        return diff.y() < 0 ? ScreenPosition::Top : ScreenPosition::Bottom;
    }
}

void ScreenArrangementWidget::animateToPosition(ScreenPosition pos)
{
    m_snapAnimation->stop();
    m_snapAnimation->setStartValue(m_remoteScreenPos);
    m_snapAnimation->setEndValue(positionToPoint(pos));
    m_snapAnimation->start();
}

void ScreenArrangementWidget::snapToPosition()
{
    ScreenPosition newPos = positionFromPoint(m_remoteScreenPos);
    if (newPos != m_position) {
        m_position = newPos;
        emit positionChanged(m_position);
    }
    animateToPosition(m_position);
}

void ScreenArrangementWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw drop zones when dragging
    if (m_isDragging) {
        drawDropZones(painter);
    }
    
    // Draw connection line
    drawConnectionLine(painter);
    
    // Draw local screen (always in center)
    drawScreen(painter, m_localScreenRect, m_localName, true, false, false);
    
    // Draw remote screen
    QRectF remoteRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    remoteRect.moveCenter(m_remoteScreenPos);
    drawScreen(painter, remoteRect, m_remoteName, false, m_isHoveringRemote, m_isDragging);
}

void ScreenArrangementWidget::drawScreen(QPainter& painter, const QRectF& rect, 
                                          const QString& name, bool isLocal,
                                          bool isHovered, bool isDragging)
{
    // Screen bezel
    QPainterPath bezelPath;
    bezelPath.addRoundedRect(rect, SCREEN_CORNER_RADIUS, SCREEN_CORNER_RADIUS);
    
    // Colors
    QColor bezelColor = isLocal ? QColor("#3d4f6f") : QColor("#2d5a4a");
    QColor screenColor = isLocal ? QColor("#1a2332") : QColor("#1a332a");
    QColor glowColor = isLocal ? QColor("#6c8ebf") : QColor("#5fb88f");
    
    if (isHovered || isDragging) {
        bezelColor = bezelColor.lighter(130);
        glowColor = glowColor.lighter(120);
    }
    
    // Draw glow effect
    if (isHovered || isDragging) {
        painter.save();
        for (int i = 15; i > 0; i -= 3) {
            QColor glow = glowColor;
            glow.setAlpha(30 - i * 2);
            painter.setPen(QPen(glow, i));
            painter.drawRoundedRect(rect.adjusted(-i/2, -i/2, i/2, i/2), 
                                   SCREEN_CORNER_RADIUS + i/2, SCREEN_CORNER_RADIUS + i/2);
        }
        painter.restore();
    }
    
    // Draw bezel
    painter.fillPath(bezelPath, bezelColor);
    
    // Draw screen area (inner rectangle)
    QRectF screenRect = rect.adjusted(6, 6, -6, -12);
    QPainterPath screenPath;
    screenPath.addRoundedRect(screenRect, 4, 4);
    painter.fillPath(screenPath, screenColor);
    
    // Draw screen reflection
    QLinearGradient reflection(screenRect.topLeft(), screenRect.bottomLeft());
    reflection.setColorAt(0, QColor(255, 255, 255, 15));
    reflection.setColorAt(0.5, QColor(255, 255, 255, 5));
    reflection.setColorAt(1, QColor(0, 0, 0, 20));
    painter.fillPath(screenPath, reflection);
    
    // Draw stand
    QRectF standRect(rect.center().x() - 15, rect.bottom() - 8, 30, 6);
    painter.fillRect(standRect, bezelColor.darker(110));
    
    // Draw name
    painter.setPen(QColor(255, 255, 255, 200));
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setWeight(QFont::Medium);
    painter.setFont(font);
    painter.drawText(screenRect, Qt::AlignCenter, name);
    
    // Draw "YOU" badge for local screen
    if (isLocal) {
        QRectF badgeRect(rect.right() - 28, rect.top() - 8, 32, 16);
        QPainterPath badgePath;
        badgePath.addRoundedRect(badgeRect, 8, 8);
        painter.fillPath(badgePath, QColor("#6c8ebf"));
        
        painter.setPen(Qt::white);
        font.setPixelSize(9);
        font.setWeight(QFont::Bold);
        painter.setFont(font);
        painter.drawText(badgeRect, Qt::AlignCenter, "YOU");
    }
}

void ScreenArrangementWidget::drawConnectionLine(QPainter& painter)
{
    QPointF localCenter = m_localScreenRect.center();
    QPointF remoteCenter = m_remoteScreenPos;
    
    // Calculate edge points
    QPointF localEdge, remoteEdge;
    
    QPointF diff = remoteCenter - localCenter;
    if (qAbs(diff.x()) > qAbs(diff.y())) {
        // Horizontal connection
        if (diff.x() > 0) {
            localEdge = QPointF(m_localScreenRect.right(), localCenter.y());
            remoteEdge = QPointF(remoteCenter.x() - SCREEN_WIDTH/2, remoteCenter.y());
        } else {
            localEdge = QPointF(m_localScreenRect.left(), localCenter.y());
            remoteEdge = QPointF(remoteCenter.x() + SCREEN_WIDTH/2, remoteCenter.y());
        }
    } else {
        // Vertical connection
        if (diff.y() > 0) {
            localEdge = QPointF(localCenter.x(), m_localScreenRect.bottom());
            remoteEdge = QPointF(remoteCenter.x(), remoteCenter.y() - SCREEN_HEIGHT/2);
        } else {
            localEdge = QPointF(localCenter.x(), m_localScreenRect.top());
            remoteEdge = QPointF(remoteCenter.x(), remoteCenter.y() + SCREEN_HEIGHT/2);
        }
    }
    
    // Draw dashed line
    QPen linePen(QColor("#5fb88f"), 2, Qt::DashLine);
    linePen.setDashPattern({4, 4});
    painter.setPen(linePen);
    painter.drawLine(localEdge, remoteEdge);
    
    // Draw arrow at remote end
    QPointF arrowDir = (remoteEdge - localEdge);
    double len = qSqrt(arrowDir.x() * arrowDir.x() + arrowDir.y() * arrowDir.y());
    if (len > 0) {
        arrowDir /= len;
        QPointF perpendicular(-arrowDir.y(), arrowDir.x());
        
        QPointF arrowTip = remoteEdge;
        QPointF arrowLeft = arrowTip - arrowDir * 10 + perpendicular * 5;
        QPointF arrowRight = arrowTip - arrowDir * 10 - perpendicular * 5;
        
        QPainterPath arrowPath;
        arrowPath.moveTo(arrowTip);
        arrowPath.lineTo(arrowLeft);
        arrowPath.lineTo(arrowRight);
        arrowPath.closeSubpath();
        
        painter.fillPath(arrowPath, QColor("#5fb88f"));
    }
}

void ScreenArrangementWidget::drawDropZones(QPainter& painter)
{
    QPointF center(width() / 2.0, height() / 2.0);
    
    struct Zone {
        ScreenPosition pos;
        QRectF rect;
    };
    
    std::vector<Zone> zones = {
        {ScreenPosition::Left, QRectF(0, center.y() - 60, center.x() - SCREEN_WIDTH/2 - 10, 120)},
        {ScreenPosition::Right, QRectF(center.x() + SCREEN_WIDTH/2 + 10, center.y() - 60, 
                                       width() - center.x() - SCREEN_WIDTH/2 - 10, 120)},
        {ScreenPosition::Top, QRectF(center.x() - 80, 0, 160, center.y() - SCREEN_HEIGHT/2 - 10)},
        {ScreenPosition::Bottom, QRectF(center.x() - 80, center.y() + SCREEN_HEIGHT/2 + 10, 
                                        160, height() - center.y() - SCREEN_HEIGHT/2 - 10)}
    };
    
    ScreenPosition hoveredPos = positionFromPoint(m_remoteScreenPos);
    
    for (const auto& zone : zones) {
        bool isHovered = (zone.pos == hoveredPos);
        
        QColor zoneColor = isHovered ? QColor(95, 184, 143, 40) : QColor(255, 255, 255, 10);
        QColor borderColor = isHovered ? QColor(95, 184, 143, 100) : QColor(255, 255, 255, 30);
        
        QPainterPath zonePath;
        zonePath.addRoundedRect(zone.rect, 12, 12);
        painter.fillPath(zonePath, zoneColor);
        
        painter.setPen(QPen(borderColor, 2, Qt::DashLine));
        painter.drawRoundedRect(zone.rect, 12, 12);
    }
}

void ScreenArrangementWidget::mousePressEvent(QMouseEvent* event)
{
    QRectF remoteRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    remoteRect.moveCenter(m_remoteScreenPos);
    
    if (remoteRect.contains(event->pos())) {
        m_isDragging = true;
        m_dragOffset = event->pos() - m_remoteScreenPos;
        setCursor(Qt::ClosedHandCursor);
        update();
    }
}

void ScreenArrangementWidget::mouseMoveEvent(QMouseEvent* event)
{
    QRectF remoteRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    remoteRect.moveCenter(m_remoteScreenPos);
    
    bool wasHovering = m_isHoveringRemote;
    m_isHoveringRemote = remoteRect.contains(event->pos());
    
    if (m_isHoveringRemote && !m_isDragging) {
        setCursor(Qt::OpenHandCursor);
    } else if (!m_isDragging) {
        setCursor(Qt::ArrowCursor);
    }
    
    if (m_isDragging) {
        m_remoteScreenPos = event->pos() - m_dragOffset;
        update();
    } else if (wasHovering != m_isHoveringRemote) {
        update();
    }
}

void ScreenArrangementWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    
    if (m_isDragging) {
        m_isDragging = false;
        setCursor(m_isHoveringRemote ? Qt::OpenHandCursor : Qt::ArrowCursor);
        snapToPosition();
    }
}
