@echo off
echo ============================================
echo    RedkaConnect Demo - New Features
echo ============================================
echo.
echo This demo showcases the new skeuomorphic interface
echo with monitor icons, cable metaphors, QR code
echo clipboard pasting, and USB connection support.
echo.
echo Features demonstrated:
echo • Skeuomorphic monitor and cable icons
echo • Visual connection status feedback
echo • QR code pairing (clipboard-based)
echo • USB cable connection UI
echo • Unplugged cable error handling
echo.
echo The demo runs as a standalone Qt application
echo showing all the new UI components.
echo.
echo ============================================
echo.

if exist "build\gui\redkaconnectdemo.exe" (
    echo Found demo executable! Running...
    echo.
    "build\gui\redkaconnectdemo.exe"
) else (
    echo Demo executable not found.
    echo.
    echo To build the demo, you would need:
    echo • CMake (cmake.org)
    echo • Qt6 (qt.io) or Qt5
    echo • Visual Studio 2019/2022
    echo • OpenSSL
    echo.
    echo Then run: clean_build.ps1
    echo.
    echo For now, you can explore the source code:
    echo • src/gui/src/SimpleMainWindow.* - Main app
    echo • README_SKEUOMORPHIC.md - Interface docs
    echo • README_USB_SUPPORT.md - USB docs
    echo • QR_CODE_README.md - QR code docs
    echo.
    pause
)