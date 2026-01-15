/*
 * InputLeap -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2008 Volker Lanz (vl@fidra.de)
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "QInputLeapApplication.h"
#include "SimpleMainWindow.h"
#include "OnboardingWidget.h"
#include "AppConfig.h"

#include <QtCore>
#include <QtGui>
#include <QSettings>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    // High DPI support
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    
    QCoreApplication::setOrganizationName("Redka");
    QCoreApplication::setOrganizationDomain("redka.dev");
    QCoreApplication::setApplicationName("RedkaConnect");

    QInputLeapApplication app(argc, argv);
    
    // Set application style
    app.setStyle("Fusion");

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    app.setDesktopFileName(QStringLiteral("dev.redka.redkaconnect"));
#endif

    // Wait for system tray to be available (but don't block too long)
    int trayAttempts = 0;
    while (!QSystemTrayIcon::isSystemTrayAvailable() && trayAttempts < 3) {
        QThread::msleep(1000);
        trayAttempts++;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    QSettings settings;
    AppConfig appConfig(&settings);

    // Check if this is the first run
    bool firstRun = !settings.contains("onboardingComplete");
    
    if (firstRun) {
        // Show beautiful onboarding
        OnboardingWidget* onboarding = new OnboardingWidget();
        onboarding->setWindowTitle("Welcome to RedkaConnect");
        onboarding->setFixedSize(500, 600);
        onboarding->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        
        QObject::connect(onboarding, &OnboardingWidget::finished, &app, 
            [&settings, &appConfig, onboarding](const QString& computerName) {
                // Save settings
                settings.setValue("onboardingComplete", true);
                settings.setValue("computerName", computerName);
                appConfig.setScreenName(computerName);
                appConfig.saveSettings();
                
                // Close onboarding and show main window
                onboarding->close();
                onboarding->deleteLater();
                
                // Create and show main window
                SimpleMainWindow* mainWindow = new SimpleMainWindow(settings, appConfig);
                mainWindow->show();
            });
        
        QObject::connect(onboarding, &OnboardingWidget::skipped, &app,
            [&settings, &appConfig, onboarding]() {
                settings.setValue("onboardingComplete", true);
                appConfig.saveSettings();
                
                onboarding->close();
                onboarding->deleteLater();
                
                SimpleMainWindow* mainWindow = new SimpleMainWindow(settings, appConfig);
                mainWindow->show();
            });
        
        onboarding->show();
    } else {
        // Normal launch - show main window directly
        SimpleMainWindow* mainWindow = new SimpleMainWindow(settings, appConfig);
        mainWindow->show();
    }

    return app.exec();
}
