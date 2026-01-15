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
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>
#include <QColor>

/**
 * @brief Animated background widget inspired by Vanta.js
 * 
 * Creates beautiful animated effects like:
 * - Floating particles with connections
 * - Subtle wave animations
 * - Gradient color shifts
 * 
 * This is rendered natively in Qt for performance.
 */
class AnimatedBackground : public QWidget
{
    Q_OBJECT

public:
    enum class Style {
        Particles,    // Connected dots like Vanta.js NET
        Waves,        // Flowing waves like Vanta.js WAVES
        Gradient      // Animated gradient shift
    };

    explicit AnimatedBackground(QWidget* parent = nullptr);
    ~AnimatedBackground() override;

    void setStyle(Style style);
    void setColors(const QColor& primary, const QColor& secondary, const QColor& accent);
    void setParticleCount(int count);
    void setSpeed(float speed);
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Particle {
        QPointF position;
        QPointF velocity;
        float size;
        float alpha;
    };

    void initParticles();
    void updateParticles();
    void drawParticles(QPainter& painter);
    void drawWaves(QPainter& painter);
    void drawGradient(QPainter& painter);
    void drawConnections(QPainter& painter);

    Style m_style;
    QTimer* m_animationTimer;
    QElapsedTimer m_elapsed;
    
    QVector<Particle> m_particles;
    int m_particleCount;
    float m_speed;
    float m_time;
    
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_accentColor;
    
    float m_connectionDistance;
};
