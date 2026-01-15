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
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QString>

class QLabel;
class QPushButton;

/**
 * @brief Beautiful animated onboarding widget
 * 
 * Shows an engaging first-time experience that explains
 * RedkaConnect's functionality with smooth animations.
 */
class OnboardingWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)

public:
    explicit OnboardingWidget(QWidget* parent = nullptr);
    ~OnboardingWidget() override;

    qreal animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(qreal progress);

signals:
    void finished(const QString& computerName);
    void skipped();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void nextPage();
    void previousPage();
    void onGetStarted();

private:
    void setupUi();
    void setupAnimations();
    void updatePage();
    void animatePageTransition(bool forward);

    // Pages
    int m_currentPage;
    static constexpr int TOTAL_PAGES = 3;

    // UI
    QLabel* m_illustrationLabel;
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QWidget* m_dotsContainer;
    QPushButton* m_nextButton;
    QPushButton* m_skipButton;
    QLineEdit* m_nameInput;

    // Animation
    qreal m_animationProgress;
    QPropertyAnimation* m_progressAnimation;
    QTimer* m_animationTimer;
    qreal m_mouseX;
    qreal m_mouseY;

    // Page content
    struct PageContent {
        QString emoji;
        QString title;
        QString description;
    };
    static const PageContent pages[TOTAL_PAGES];
};
