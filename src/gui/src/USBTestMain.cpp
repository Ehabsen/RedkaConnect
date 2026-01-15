/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General License
 * found in the file LICENSE that should have accompanied this file.
 */

#include <QApplication>
#include "USBTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("USBTest");
    app.setApplicationVersion("1.0");

    USBTest window;
    window.show();

    return app.exec();
}