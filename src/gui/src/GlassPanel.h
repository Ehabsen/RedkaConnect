/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QFrame>
#include <QColor>

/**
 * @brief Apple-style glassmorphism panel
 * 
 * Creates a frosted glass effect with:
 * - Semi-transparent background
 * - Subtle blur effect (simulated)
 * - Soft glow border
 * - Rounded corners
 */
class GlassPanel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal glowIntensity READ glowIntensity WRITE setGlowIntensity)
    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor)

public:
    explicit GlassPanel(QWidget* parent = nullptr);
    ~GlassPanel() override;

    qreal glowIntensity() const { return m_glowIntensity; }
    void setGlowIntensity(qreal intensity);

    QColor tintColor() const { return m_tintColor; }
    void setTintColor(const QColor& color);

    void setBlurRadius(int radius);
    void setBorderRadius(int radius);
    void setGlowColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal m_glowIntensity;
    QColor m_tintColor;
    QColor m_glowColor;
    int m_blurRadius;
    int m_borderRadius;
};
