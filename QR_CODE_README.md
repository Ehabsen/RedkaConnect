# QR Code Pairing - RedkaConnect

## Overview

RedkaConnect now supports QR code pairing for easier setup between devices. The QR code contains all the necessary pairing information (device ID, PIN, IP address, expiry time) in a compact JSON format.

## How It Works

### For the Host (Sharing Computer)
1. Click "Share This Computer"
2. A large 6-digit PIN appears
3. A QR code is displayed containing the pairing data
4. The PIN expires after 5 minutes

### For the Client (Connecting Computer)
**Option 1: Manual PIN Entry (Always Works)**
1. Click "Connect to Another Computer"
2. Type the 6-digit PIN manually
3. Connection established

**Option 2: QR Code Scanning (Phone Required)**
1. Use a phone camera or QR scanner app to scan the QR code
2. Copy the decoded JSON text to clipboard
3. In RedkaConnect, click the 📱 "Paste" button
4. PIN auto-fills and connection establishes automatically

## QR Code Format

The QR code contains JSON data like this:

```json
{"v":1,"id":"a1b2c3d4e5f6g7h8","n":"My-PC","p":"847291","a":"192.168.1.5","t":1736787600000}
```

Where:
- `v`: Version (always 1)
- `id`: Unique device ID
- `n`: Device name
- `p`: 6-digit PIN
- `a`: IP address
- `t`: Expiry timestamp (milliseconds since epoch)

## Testing the QR Code Feature

A test application is included:

```bash
# Build the test app
cmake --build build --target qrcodetest

# Run the test
./build/gui/qrcodetest
```

The test app shows:
- A live QR code with generated pairing data
- The raw JSON data
- "Copy JSON" button to copy to clipboard
- "Test Paste" button to open the pairing dialog and test pasting

## Technical Details

### QR Code Generation
- Uses QR Code Model 2 with error correction level L
- Supports versions 2-5 (up to ~60 characters)
- Implemented from scratch in C++ (no external dependencies)

### Security
- PIN expires after 5 minutes
- QR code data includes expiry timestamp
- Only accepts valid JSON in the expected format
- Device ID prevents pairing with wrong devices

### Limitations
- No direct camera scanning (requires phone/external scanner)
- QR code pasting requires manual copy/paste step
- Works best with phone cameras for scanning

## Future Enhancements

1. **Direct Camera Scanning**: Integrate Qt Multimedia for webcam QR reading
2. **Mobile App**: Native mobile app for instant QR scanning
3. **NFC Support**: For devices with NFC hardware
4. **Bluetooth Pairing**: Alternative pairing method

## Building with QR Code Support

The QR code functionality is automatically included when building RedkaConnect. No additional dependencies are required beyond Qt.

```bash
# Standard build process
cmake -S . -B build
cmake --build build
```