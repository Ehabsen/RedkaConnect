/*
 * RedkaConnect Demo -- Showcase of new UI features
 */

#include <QApplication>
#include "RedkaConnectDemo.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("RedkaConnectDemo");
    app.setApplicationVersion("1.0");
    app.setApplicationDisplayName("RedkaConnect Demo");

    RedkaConnectDemo window;
    window.show();

    return app.exec();
}