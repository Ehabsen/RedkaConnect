/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "GlassPanel.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

GlassPanel::GlassPanel(QWidget* parent)
    : QFrame(parent)
    , m_glowIntensity(0.5)
    , m_tintColor(QColor(255, 255, 255, 8))
    , m_glowColor(QColor(56, 189, 248, 60))
    , m_blurRadius(20)
    , m_borderRadius(20)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

GlassPanel::~GlassPanel() = default;

void GlassPanel::setGlowIntensity(qreal intensity)
{
    m_glowIntensity = qBound(0.0, intensity, 1.0);
    update();
}

void GlassPanel::setTintColor(const QColor& color)
{
    m_tintColor = color;
    update();
}

void GlassPanel::setBlurRadius(int radius)
{
    m_blurRadius = radius;
    update();
}

void GlassPanel::setBorderRadius(int radius)
{
    m_borderRadius = radius;
    update();
}

void GlassPanel::setGlowColor(const QColor& color)
{
    m_glowColor = color;
    update();
}

void GlassPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = this->rect().adjusted(4, 4, -4, -4);
    
    // Outer glow effect
    if (m_glowIntensity > 0) {
        for (int i = 0; i < 6; ++i) {
            QColor glow = m_glowColor;
            glow.setAlpha(static_cast<int>((15 - i * 2) * m_glowIntensity));
            
            QPainterPath glowPath;
            glowPath.addRoundedRect(rect.adjusted(-i*2, -i*2, i*2, i*2), 
                                   m_borderRadius + i, m_borderRadius + i);
            painter.fillPath(glowPath, glow);
        }
    }
    
    // Main glass panel
    QPainterPath panelPath;
    panelPath.addRoundedRect(rect, m_borderRadius, m_borderRadius);
    
    // Glass base - dark frosted
    QColor glassBase(15, 20, 30, 180);
    painter.fillPath(panelPath, glassBase);
    
    // Frosted noise texture simulation
    painter.setClipPath(panelPath);
    painter.setOpacity(0.03);
    for (int i = 0; i < 200; ++i) {
        int x = rand() % width();
        int y = rand() % height();
        painter.fillRect(x, y, 2, 2, Qt::white);
    }
    painter.setOpacity(1.0);
    painter.setClipping(false);
    
    // Top highlight (glass reflection)
    QLinearGradient topHighlight(rect.topLeft(), QPointF(rect.left(), rect.top() + rect.height() * 0.5));
    topHighlight.setColorAt(0, QColor(255, 255, 255, 25));
    topHighlight.setColorAt(0.5, QColor(255, 255, 255, 5));
    topHighlight.setColorAt(1, QColor(255, 255, 255, 0));
    painter.fillPath(panelPath, topHighlight);
    
    // Inner tint
    painter.fillPath(panelPath, m_tintColor);
    
    // Border
    QLinearGradient borderGradient(rect.topLeft(), rect.bottomRight());
    borderGradient.setColorAt(0, QColor(255, 255, 255, 40));
    borderGradient.setColorAt(0.5, QColor(255, 255, 255, 15));
    borderGradient.setColorAt(1, QColor(255, 255, 255, 30));
    
    painter.setPen(QPen(borderGradient, 1.5));
    painter.drawRoundedRect(rect, m_borderRadius, m_borderRadius);
    
    // Inner border (subtle)
    QPen innerBorder(QColor(255, 255, 255, 8), 1);
    painter.setPen(innerBorder);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), m_borderRadius - 1, m_borderRadius - 1);
}
