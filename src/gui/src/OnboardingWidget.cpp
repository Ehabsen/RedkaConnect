/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "OnboardingWidget.h"
#include "AnimatedBackground.h"
#include "GlassPanel.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QHostInfo>
#include <QGraphicsDropShadowEffect>
#include <QtMath>
#include <QMouseEvent>

const OnboardingWidget::PageContent OnboardingWidget::pages[TOTAL_PAGES] = {
    {
        "⚡",
        "One Keyboard & Mouse,\nMultiple Computers",
        "RedkaConnect lets you seamlessly control\nmultiple computers with your existing\nkeyboard and mouse."
    },
    {
        "🔗",
        "Connect in Seconds",
        "1. Share from one computer\n2. Connect from another\n3. That's it! Start working across screens."
    },
    {
        "✨",
        "Let's Get Started",
        "Give this computer a name so you can\neasily identify it when connecting."
    }
};

OnboardingWidget::OnboardingWidget(QWidget* parent)
    : QWidget(parent)
    , m_currentPage(0)
    , m_animationProgress(0.0)
    , m_mouseX(0.5)
    , m_mouseY(0.5)
    , m_nameInput(nullptr)
{
    setMouseTracking(true);
    setupUi();
    setupAnimations();
}

OnboardingWidget::~OnboardingWidget() = default;

void OnboardingWidget::setupUi()
{
    // Main layout with animated background
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    
    // Container for content
    QWidget* container = new QWidget();
    rootLayout->addWidget(container);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(40, 32, 40, 32);
    mainLayout->setSpacing(0);

    // Skip button (top right)
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addStretch();
    m_skipButton = new QPushButton("Skip →");
    m_skipButton->setObjectName("skipButton");
    m_skipButton->setCursor(Qt::PointingHandCursor);
    connect(m_skipButton, &QPushButton::clicked, this, &OnboardingWidget::skipped);
    topLayout->addWidget(m_skipButton);
    mainLayout->addLayout(topLayout);

    mainLayout->addSpacing(20);

    // Illustration with glow
    m_illustrationLabel = new QLabel();
    m_illustrationLabel->setAlignment(Qt::AlignCenter);
    m_illustrationLabel->setObjectName("illustrationLabel");
    
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(60);
    glow->setColor(QColor(6, 182, 212, 150));
    glow->setOffset(0, 0);
    m_illustrationLabel->setGraphicsEffect(glow);
    mainLayout->addWidget(m_illustrationLabel);

    mainLayout->addSpacing(32);

    // Title
    m_titleLabel = new QLabel();
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("onboardingTitle");
    m_titleLabel->setWordWrap(true);
    mainLayout->addWidget(m_titleLabel);

    mainLayout->addSpacing(16);

    // Description
    m_descriptionLabel = new QLabel();
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setObjectName("onboardingDesc");
    m_descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(m_descriptionLabel);

    mainLayout->addSpacing(24);

    // Name input (hidden until last page)
    m_nameInput = new QLineEdit();
    m_nameInput->setObjectName("nameInput");
    m_nameInput->setPlaceholderText("Enter computer name...");
    m_nameInput->setText(QHostInfo::localHostName());
    m_nameInput->setAlignment(Qt::AlignCenter);
    m_nameInput->setMinimumHeight(56);
    m_nameInput->setMaximumWidth(320);
    m_nameInput->hide();
    
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addStretch();
    inputLayout->addWidget(m_nameInput);
    inputLayout->addStretch();
    mainLayout->addLayout(inputLayout);

    mainLayout->addStretch();

    // Dots indicator
    m_dotsContainer = new QWidget();
    QHBoxLayout* dotsLayout = new QHBoxLayout(m_dotsContainer);
    dotsLayout->setSpacing(10);
    dotsLayout->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < TOTAL_PAGES; ++i) {
        QLabel* dot = new QLabel();
        dot->setFixedSize(10, 10);
        dot->setObjectName(i == 0 ? "dotActive" : "dotInactive");
        dotsLayout->addWidget(dot);
    }
    
    QHBoxLayout* dotsContainerLayout = new QHBoxLayout();
    dotsContainerLayout->addStretch();
    dotsContainerLayout->addWidget(m_dotsContainer);
    dotsContainerLayout->addStretch();
    mainLayout->addLayout(dotsContainerLayout);

    mainLayout->addSpacing(24);

    // Next button
    m_nextButton = new QPushButton("Next →");
    m_nextButton->setObjectName("primaryButton");
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setMinimumHeight(56);
    connect(m_nextButton, &QPushButton::clicked, this, &OnboardingWidget::nextPage);
    mainLayout->addWidget(m_nextButton);

    updatePage();

    // Stylesheet - matches main app
    QString css = R"(
        OnboardingWidget {
            background: qlineargradient(x1:0, y1:0, x2:0.5, y2:1,
                stop:0 #080c15, stop:0.3 #0c1219, stop:0.7 #0f172a, stop:1 #080c15);
        }

        #skipButton {
            background: transparent;
            border: none;
            color: #64748b;
            font-size: 14px;
            font-weight: 500;
            padding: 8px 16px;
        }

        #skipButton:hover {
            color: #94a3b8;
        }

        #illustrationLabel {
            font-size: 80px;
            min-height: 120px;
        }

        #onboardingTitle {
            font-size: 32px;
            font-weight: 700;
            color: #f1f5f9;
            line-height: 1.3;
        }

        #onboardingDesc {
            font-size: 16px;
            color: #94a3b8;
            line-height: 1.7;
        }

        #nameInput {
            background: rgba(15, 23, 42, 0.6);
            border: 2px solid rgba(6, 182, 212, 0.3);
            border-radius: 14px;
            font-size: 18px;
            color: #e2e8f0;
            padding: 14px 24px;
        }

        #nameInput:focus {
            border-color: #06b6d4;
            background: rgba(15, 23, 42, 0.8);
        }

        #nameInput::placeholder {
            color: #475569;
        }

        #dotActive {
            background: #06b6d4;
            border-radius: 5px;
        }

        #dotInactive {
            background: rgba(100, 116, 139, 0.4);
            border-radius: 5px;
        }

        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #0891b2, stop:1 #06b6d4);
            border: none;
            border-radius: 14px;
            color: #0c1220;
            font-size: 17px;
            font-weight: 600;
            padding: 16px 32px;
        }

        #primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #06b6d4, stop:1 #22d3ee);
        }
        
        #primaryButton:focus {
            outline: 2px solid #22d3ee;
            outline-offset: 2px;
        }
    )";
    setStyleSheet(css);
}

void OnboardingWidget::setupAnimations()
{
    m_progressAnimation = new QPropertyAnimation(this, "animationProgress", this);
    m_progressAnimation->setDuration(800);
    m_progressAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_animationTimer->start(50);
}

void OnboardingWidget::setAnimationProgress(qreal progress)
{
    m_animationProgress = progress;
    update();
}

void OnboardingWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    m_progressAnimation->setStartValue(0.0);
    m_progressAnimation->setEndValue(1.0);
    m_progressAnimation->start();
}

void OnboardingWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient bg(0, 0, width(), height());
    bg.setColorAt(0, QColor(8, 12, 21));
    bg.setColorAt(0.4, QColor(12, 18, 25));
    bg.setColorAt(0.7, QColor(15, 23, 42));
    bg.setColorAt(1, QColor(8, 12, 21));
    painter.fillRect(rect(), bg);

    // Animated orbs
    qreal time = (QDateTime::currentMSecsSinceEpoch() % 20000) / 20000.0;
    
    // Cyan orb
    QRadialGradient cyanOrb(
        width() * (0.3 + 0.15 * qSin(time * 2 * M_PI)),
        height() * (0.35 + 0.15 * qCos(time * 2 * M_PI)),
        200
    );
    cyanOrb.setColorAt(0, QColor(6, 182, 212, 35));
    cyanOrb.setColorAt(1, QColor(6, 182, 212, 0));
    painter.fillRect(rect(), cyanOrb);

    // Purple orb
    QRadialGradient purpleOrb(
        width() * (0.7 + 0.12 * qCos(time * 1.5 * M_PI)),
        height() * (0.65 + 0.12 * qSin(time * 1.5 * M_PI)),
        180
    );
    purpleOrb.setColorAt(0, QColor(139, 92, 246, 25));
    purpleOrb.setColorAt(1, QColor(139, 92, 246, 0));
    painter.fillRect(rect(), purpleOrb);

    // Subtle grid
    painter.setPen(QPen(QColor(255, 255, 255, 6), 1));
    int gridSize = 50;
    for (int x = 0; x < width(); x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }
}

void OnboardingWidget::updatePage()
{
    const auto& page = pages[m_currentPage];
    
    m_illustrationLabel->setText(page.emoji);
    m_titleLabel->setText(page.title);
    m_descriptionLabel->setText(page.description);

    m_nameInput->setVisible(m_currentPage == TOTAL_PAGES - 1);
    m_skipButton->setVisible(m_currentPage < TOTAL_PAGES - 1);

    if (m_currentPage == TOTAL_PAGES - 1) {
        m_nextButton->setText("Get Started →");
    } else {
        m_nextButton->setText("Next →");
    }

    // Update dots
    QList<QLabel*> dots = m_dotsContainer->findChildren<QLabel*>();
    for (int i = 0; i < dots.size(); ++i) {
        dots[i]->setObjectName(i == m_currentPage ? "dotActive" : "dotInactive");
        dots[i]->setStyleSheet(i == m_currentPage 
            ? "background: #06b6d4; border-radius: 5px;"
            : "background: rgba(100, 116, 139, 0.4); border-radius: 5px;");
    }
}

void OnboardingWidget::nextPage()
{
    if (m_currentPage < TOTAL_PAGES - 1) {
        m_currentPage++;
        animatePageTransition(true);
        updatePage();
    } else {
        onGetStarted();
    }
}

void OnboardingWidget::previousPage()
{
    if (m_currentPage > 0) {
        m_currentPage--;
        animatePageTransition(false);
        updatePage();
    }
}

void OnboardingWidget::onGetStarted()
{
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) {
        name = QHostInfo::localHostName();
    }
    emit finished(name);
}

void OnboardingWidget::animatePageTransition(bool forward)
{
    Q_UNUSED(forward)
}
