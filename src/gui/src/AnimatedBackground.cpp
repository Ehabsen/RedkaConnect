/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "AnimatedBackground.h"

#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>
#include <QLinearGradient>
#include <QRadialGradient>

AnimatedBackground::AnimatedBackground(QWidget* parent)
    : QWidget(parent)
    , m_style(Style::Particles)
    , m_animationTimer(new QTimer(this))
    , m_particleCount(50)
    , m_speed(1.0f)
    , m_time(0.0f)
    , m_primaryColor(QColor(15, 23, 42))      // Deep blue-black
    , m_secondaryColor(QColor(30, 41, 59))    // Slate
    , m_accentColor(QColor(56, 189, 248))     // Cyan accent
    , m_connectionDistance(120.0f)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        m_time += 0.016f * m_speed;  // ~60fps
        updateParticles();
        update();
    });
    
    m_elapsed.start();
}

AnimatedBackground::~AnimatedBackground() = default;

void AnimatedBackground::setStyle(Style style)
{
    m_style = style;
    if (style == Style::Particles) {
        initParticles();
    }
    update();
}

void AnimatedBackground::setColors(const QColor& primary, const QColor& secondary, const QColor& accent)
{
    m_primaryColor = primary;
    m_secondaryColor = secondary;
    m_accentColor = accent;
    update();
}

void AnimatedBackground::setParticleCount(int count)
{
    m_particleCount = count;
    initParticles();
}

void AnimatedBackground::setSpeed(float speed)
{
    m_speed = speed;
}

void AnimatedBackground::start()
{
    initParticles();
    m_animationTimer->start(16);  // ~60fps
}

void AnimatedBackground::stop()
{
    m_animationTimer->stop();
}

void AnimatedBackground::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    initParticles();
}

void AnimatedBackground::initParticles()
{
    m_particles.clear();
    m_particles.reserve(m_particleCount);
    
    auto* rng = QRandomGenerator::global();
    
    for (int i = 0; i < m_particleCount; ++i) {
        Particle p;
        p.position = QPointF(
            rng->bounded(static_cast<double>(width())),
            rng->bounded(static_cast<double>(height()))
        );
        p.velocity = QPointF(
            (rng->bounded(2.0) - 1.0) * 0.5,
            (rng->bounded(2.0) - 1.0) * 0.5
        );
        p.size = 2.0f + rng->bounded(3.0f);
        p.alpha = 0.3f + rng->bounded(0.5f);
        m_particles.append(p);
    }
}

void AnimatedBackground::updateParticles()
{
    for (auto& p : m_particles) {
        p.position += p.velocity * m_speed;
        
        // Wrap around edges
        if (p.position.x() < -10) p.position.setX(width() + 10);
        if (p.position.x() > width() + 10) p.position.setX(-10);
        if (p.position.y() < -10) p.position.setY(height() + 10);
        if (p.position.y() > height() + 10) p.position.setY(-10);
        
        // Subtle oscillation
        p.position.setY(p.position.y() + qSin(m_time + p.position.x() * 0.01) * 0.2);
    }
}

void AnimatedBackground::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Base gradient background
    QLinearGradient bgGradient(0, 0, width(), height());
    bgGradient.setColorAt(0, m_primaryColor);
    bgGradient.setColorAt(0.5, m_secondaryColor.darker(110));
    bgGradient.setColorAt(1, m_primaryColor.darker(120));
    painter.fillRect(rect(), bgGradient);
    
    // Add subtle noise/texture effect
    painter.setOpacity(0.03);
    for (int i = 0; i < 100; ++i) {
        int x = QRandomGenerator::global()->bounded(width());
        int y = QRandomGenerator::global()->bounded(height());
        painter.fillRect(x, y, 1, 1, Qt::white);
    }
    painter.setOpacity(1.0);
    
    switch (m_style) {
        case Style::Particles:
            drawConnections(painter);
            drawParticles(painter);
            break;
        case Style::Waves:
            drawWaves(painter);
            break;
        case Style::Gradient:
            drawGradient(painter);
            break;
    }
    
    // Vignette effect
    QRadialGradient vignette(width() / 2, height() / 2, qMax(width(), height()) * 0.7);
    vignette.setColorAt(0, QColor(0, 0, 0, 0));
    vignette.setColorAt(0.7, QColor(0, 0, 0, 0));
    vignette.setColorAt(1, QColor(0, 0, 0, 80));
    painter.fillRect(rect(), vignette);
}

void AnimatedBackground::drawParticles(QPainter& painter)
{
    for (const auto& p : m_particles) {
        QColor particleColor = m_accentColor;
        particleColor.setAlphaF(p.alpha * 0.8);
        
        // Glow effect
        QRadialGradient glow(p.position, p.size * 3);
        glow.setColorAt(0, QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 40));
        glow.setColorAt(1, QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 0));
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p.position, p.size * 3, p.size * 3);
        
        // Core particle
        painter.setBrush(particleColor);
        painter.drawEllipse(p.position, p.size, p.size);
    }
}

void AnimatedBackground::drawConnections(QPainter& painter)
{
    for (int i = 0; i < m_particles.size(); ++i) {
        for (int j = i + 1; j < m_particles.size(); ++j) {
            QPointF diff = m_particles[i].position - m_particles[j].position;
            float distance = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
            
            if (distance < m_connectionDistance) {
                float alpha = (1.0f - distance / m_connectionDistance) * 0.3f;
                QColor lineColor = m_accentColor;
                lineColor.setAlphaF(alpha);
                
                painter.setPen(QPen(lineColor, 1.0));
                painter.drawLine(m_particles[i].position, m_particles[j].position);
            }
        }
    }
}

void AnimatedBackground::drawWaves(QPainter& painter)
{
    int waveCount = 4;
    
    for (int w = 0; w < waveCount; ++w) {
        QPainterPath path;
        float phase = m_time * (0.5f + w * 0.2f);
        float amplitude = 30 + w * 15;
        float yOffset = height() * (0.4f + w * 0.15f);
        
        path.moveTo(0, yOffset);
        
        for (int x = 0; x <= width(); x += 5) {
            float y = yOffset + 
                qSin(x * 0.01f + phase) * amplitude +
                qSin(x * 0.02f + phase * 1.5f) * (amplitude * 0.5f);
            path.lineTo(x, y);
        }
        
        path.lineTo(width(), height());
        path.lineTo(0, height());
        path.closeSubpath();
        
        QColor waveColor = m_accentColor;
        waveColor.setAlpha(20 + w * 10);
        painter.fillPath(path, waveColor);
    }
}

void AnimatedBackground::drawGradient(QPainter& painter)
{
    // Animated gradient orbs
    float t = m_time * 0.5f;
    
    QPointF center1(
        width() * (0.3f + 0.2f * qSin(t)),
        height() * (0.4f + 0.2f * qCos(t * 0.7f))
    );
    
    QPointF center2(
        width() * (0.7f + 0.2f * qCos(t * 0.8f)),
        height() * (0.6f + 0.2f * qSin(t * 0.6f))
    );
    
    QRadialGradient orb1(center1, width() * 0.5f);
    orb1.setColorAt(0, QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 60));
    orb1.setColorAt(1, QColor(m_accentColor.red(), m_accentColor.green(), m_accentColor.blue(), 0));
    painter.fillRect(rect(), orb1);
    
    QColor secondary = m_accentColor;
    secondary.setHsv((m_accentColor.hue() + 60) % 360, m_accentColor.saturation(), m_accentColor.value());
    
    QRadialGradient orb2(center2, width() * 0.4f);
    orb2.setColorAt(0, QColor(secondary.red(), secondary.green(), secondary.blue(), 40));
    orb2.setColorAt(1, QColor(secondary.red(), secondary.green(), secondary.blue(), 0));
    painter.fillRect(rect(), orb2);
}
