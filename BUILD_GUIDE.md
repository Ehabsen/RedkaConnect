# RedkaConnect Build Guide

## Prerequisites

### Required Software
1. **Visual Studio 2019/2022** with C++ build tools
2. **CMake 3.21+**
3. **Qt6 (6.4.2 recommended)** or **Qt5 (5.15.2)**
4. **OpenSSL 3.x**
5. **Administrator privileges** (for some installations)

### Current Installation Status
- ✅ **Visual Studio 2019**: Installed
- ✅ **CMake 3.27.9**: Installed
- ✅ **OpenSSL 3.6.0**: Installed
- ❌ **Qt6/Qt5**: Not installed (see below)

## Step 1: Install Qt

### Option A: Download Qt Installer (Recommended)
1. Go to https://www.qt.io/download
2. Download "Qt Online Installer for Windows"
3. Run the installer
4. During installation:
   - Select "Custom installation"
   - Choose Qt 6.4.2 → MSVC 2019 64-bit
   - Include Qt Network Auth and Qt Serial Port modules
   - Install to default location (C:\Qt)

### Option B: Use aqt (Command Line)
```powershell
# Install Python first if needed
winget install --id Python.Python.3.11

# Install aqt
pip install aqtinstall

# Install Qt 6.4.2
aqt install-qt windows desktop 6.4.2 win64_msvc2019_64 -O C:\Qt
```

## Step 2: Verify Installations

After installing Qt, verify all components:

```powershell
# Check CMake
cmake --version
# Should show: cmake version 3.27.x

# Check Qt (adjust path if needed)
dir "C:\Qt\6.4.2\msvc2019_64\bin\"
# Should contain: qt6-core.dll, qt6-widgets.dll, etc.

# Check OpenSSL
openssl version
# Should show: OpenSSL 3.x.x
```

## Step 3: Build RedkaConnect

### Automated Build (Recommended)
```powershell
# Open PowerShell as Administrator
# Navigate to project directory
cd C:\Users\exodo\Downloads\input-leap-master

# Run the automated build script
.\clean_build.ps1
```

### Manual Build Steps
```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.4.2\msvc2019_64" `
    -DQT_DEFAULT_MAJOR_VERSION=6 `
    -DCMAKE_INSTALL_PREFIX=input-leap-install

# Build
cmake --build . --parallel --config Release --target install
```

## Step 4: Run the Applications

After successful build:

```powershell
# Main RedkaConnect GUI
.\build\bin\Release\redkaconnect.exe

# Test applications
.\build\bin\Release\qrcodetest.exe    # QR code test
.\build\bin\Release\usbtest.exe       # USB test

# Demo application (no Qt required)
.\RedkaConnectDemo.exe               # Feature showcase
```

## Troubleshooting

### Common Issues

#### 1. "Qt not found"
```
Error: Could not find Qt
```
**Solution:**
- Verify Qt installation path
- Add Qt bin directory to PATH
- Use `-DCMAKE_PREFIX_PATH=C:\Qt\6.4.2\msvc2019_64`

#### 2. "MSVC not found"
```
Error: Generator Visual Studio 17 2022 could not find any instance of Visual Studio
```
**Solution:**
- Install Visual Studio with C++ build tools
- Use correct generator: `-G "Visual Studio 16 2019"` for VS 2019

#### 3. "OpenSSL not found"
```
Error: OpenSSL library not found
```
**Solution:**
- Install OpenSSL development package
- Add OpenSSL bin directory to PATH

#### 4. Permission errors
```
Error: Access denied
```
**Solution:**
- Run PowerShell/Command Prompt as Administrator
- Check file permissions on project directory

### Build Script Issues

If `clean_build.ps1` fails:
1. Check that all prerequisites are installed
2. Verify paths in the script
3. Try manual build steps above
4. Check the `deps\` directory for Bonjour SDK

## What Gets Built

### Main Applications
- **redkaconnect.exe** - Main GUI application with all new features
- **input-leaps.exe** - Server component
- **input-leapc.exe** - Client component
- **input-leapd.exe** - Daemon component

### Test Applications
- **qrcodetest.exe** - QR code generation and clipboard pasting test
- **usbtest.exe** - USB cable connection test
- **guiunittests.exe** - Unit tests

### Demo Application
- **RedkaConnectDemo.exe** - Standalone feature showcase (no full build required)

## Features in the Build

✅ **Skeuomorphic Interface**
- Monitor icons instead of generic boxes
- Cable metaphors for connection status
- Unplugged cable error messages

✅ **USB Cable Support**
- Direct USB serial connections
- CDC-ACM protocol support
- Automatic device detection

✅ **QR Code Enhancement**
- Real QR code generation
- Clipboard-based pasting
- PIN auto-fill from scanned codes

✅ **Enhanced Pairing**
- Visual PIN entry with auto-advance
- Friendly error messages
- Improved security

## Next Steps

After successful build:
1. Test the main application
2. Try USB cable connections
3. Test QR code pairing
4. Explore the demo application

Enjoy your new RedkaConnect with all the modern features! 🎉