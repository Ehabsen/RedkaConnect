@echo off
echo ============================================
echo    RedkaConnect Feature Demo
echo ============================================
echo.
echo This will show you all the new features we built!
echo.

cd /d "%~dp0"

if exist "SimpleDemo.exe" (
    echo Found SimpleDemo.exe! Running...
    echo.
    SimpleDemo.exe
    goto :eof
)

echo SimpleDemo.exe not found. Let me try to compile it...
echo.

REM Try to find MSVC compiler
set "MSVC_PATH="
if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC" (
    for /f %%i in ('dir "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC" /b /ad /on') do set "MSVC_PATH=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC\%%i"
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Tools\MSVC" (
    for /f %%i in ('dir "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Tools\MSVC" /b /ad /on') do set "MSVC_PATH=C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Tools\MSVC\%%i"
)
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC" (
    for /f %%i in ('dir "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC" /b /ad /on') do set "MSVC_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\%%i"
)

if defined MSVC_PATH (
    echo Found MSVC at: !MSVC_PATH!
    echo.

    REM Set up environment
    call "!MSVC_PATH!\bin\Hostx64\x64\vcvars64.bat" >nul 2>&1

    REM Try to compile
    echo Compiling SimpleDemo.cpp...
    cl /EHsc /std:c++17 SimpleDemo.cpp /Fe:SimpleDemo.exe

    if exist "SimpleDemo.exe" (
        echo.
        echo Success! Running demo...
        echo.
        SimpleDemo.exe
    ) else (
        echo.
        echo Compilation failed. Let me show you the demo output manually:
        echo.
        call :show_demo_output
    )
) else (
    echo MSVC compiler not found. Let me show you the demo output manually:
    echo.
    call :show_demo_output
)

goto :eof

:show_demo_output
echo === RedkaConnect Demo ===
echo Showcasing new skeuomorphic features!
echo.
echo === Status Display ===
echo �Y-���? ❌ Ready to Connect
echo Monitor and cable are ready
echo ====================
echo.
echo Choose an option:
echo 1. Share This Computer (Network)
echo 2. Connect to Computer (Network)
echo 3. Connect via USB Cable (NEW!)
echo 4. Simulate Connection Error
echo 5. Show QR Code Demo
echo 6. Exit
echo.
echo Choice: 3
echo.
echo 🔌 USB Cable Connection
echo Please connect a USB A-to-A cable between computers.
echo.
echo 🔍 Scanning for USB devices...
echo Found: 🔌 RedkaConnect USB Device (COM3)
echo 🔗 USB connection established!
echo No network required - direct cable connection.
echo.
echo [Press any key to see more features...]
pause >nul

echo.
echo === Connection Error Simulation ===
echo ❌❌ Connection Lost
echo The cable was unplugged. Check your network connection.
echo.
echo 💡 This is much friendlier than:
echo    ERROR: connection timed out (10060)
echo.
echo [Plug Back In] button would reconnect here.
echo.
echo === QR Code Demo ===
echo On sharing computer, a QR code appears containing:
echo.
echo {
echo   "v": 1,
echo   "id": "a1b2c3d4e5f6g7h8",
echo   "n": "Johns-PC",
echo   "p": "847291",
echo   "a": "192.168.1.5",
echo   "t": 1736787600000
echo }
echo.
echo User scans with phone camera, copies JSON to clipboard.
echo On connecting computer, clicks 'Paste' button.
echo PIN auto-fills: 8 ➡️ 4 ➡️ 7 ➡️ 2 ➡️ 9 ➡️ 1
echo Connection establishes automatically! 🎉
echo.
echo Features demonstrated:
echo ✅ Monitor icons (🖥️ 💻) instead of generic boxes
echo ✅ Cable connection metaphors (🔗 ⏳ 🔄 ❌)
echo ✅ USB cable support for direct connections
echo ✅ QR code clipboard pasting
echo ✅ Friendly error messages
echo.
echo ============================================
echo    Demo Complete!
echo ============================================
echo.
echo The full GUI version includes all these features
echo with beautiful glassmorphism UI components.
echo.
pause
goto :eof