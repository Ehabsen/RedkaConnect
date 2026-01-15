/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include <QApplication>
#include "QRCodeTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QRCodeTest");
    app.setApplicationVersion("1.0");

    QRCodeTest window;
    window.show();

    return app.exec();
}