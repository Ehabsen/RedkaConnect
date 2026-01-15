# USB Cable Support - RedkaConnect

## Overview

RedkaConnect now supports **direct USB cable connections** as an alternative to network connections. This provides a fast, secure, and reliable connection method that doesn't require network configuration.

## How USB Connections Work

### Physical Connection
- Connect two computers with a **USB A-to-A cable** or appropriate USB adapters
- One computer runs in "Share" mode, the other in "Connect via USB" mode
- No network, WiFi, or internet required

### Technical Details
- Uses **USB Serial (CDC-ACM)** protocol
- Automatic device detection and pairing
- Built-in handshake and heartbeat for connection monitoring
- Fallback to manual device selection

## User Experience

### Home Screen Options
RedkaConnect now offers three connection methods:

1. **📤 Share This Computer** - Network sharing (original)
2. **📥 Connect to Computer** - Network client (original)
3. **🔌 Connect via USB Cable** - Direct USB connection (new)

### USB Connection Flow

#### On the Sharing Computer:
1. Click "Share This Computer" (network sharing mode)
2. Display pairing code/QR as usual

#### On the Connecting Computer:
1. Click "🔌 Connect via USB Cable"
2. Physically connect USB cable between computers
3. RedkaConnect automatically detects USB devices
4. Select the detected device and click "Connect"
5. Connection established instantly

### Visual Feedback

#### Status Display Updates
- **Ready:** 🖥️ 🔗 "Monitor and cable are ready"
- **USB Connecting:** 🖥️ 🔄 "Plugging in the USB cable..."
- **USB Connected:** 🖥️ 🔗 "Cable connected to USB device"

#### Error Handling
- **No USB Device:** "USB cable not detected"
- **Connection Failed:** "USB Connection Failed - Make sure the other computer is in share mode"
- **Cable Disconnected:** 🔌❌ "USB Cable Disconnected - Please reconnect the cable"

## Hardware Requirements

### Cables Needed
- **USB A to USB A**: For computers with USB A ports
- **USB A to USB C**: Mixed port types
- **USB C to USB C**: Modern computers

### Important Notes
- **NOT a standard USB cable!** Most USB cables are A-to-something and won't work for A-to-A connections
- USB cables for device-to-device connections are available but less common
- Some computers may require USB adapters or special cables

## Technical Implementation

### USBConnectionManager Class
```cpp
class USBConnectionManager : public QObject {
    // Device detection and management
    QList<USBDevice> availableDevices() const;
    void refreshDevices();

    // Connection management
    bool connectToDevice(const QString& portName);
    void disconnect();

    // Data communication
    bool sendData(const QByteArray& data);
    QByteArray readData();
};
```

### Protocol Details
- **Handshake:** "REDKA-USB-HANDSHAKE-REQUEST/RESPONSE"
- **Heartbeat:** "REDKA-USB-HEARTBEAT" (every 2 seconds)
- **Data Format:** Raw binary over serial connection
- **Baud Rate:** 115200 (configurable)

### Device Detection
- Scans for CDC-ACM (Communication Device Class - Abstract Control Model) devices
- Filters for devices with "Redka" in manufacturer/description
- Automatic refresh every second during discovery

## Advantages of USB Connections

### Speed & Reliability
- **Direct connection** - no network latency or interference
- **Consistent performance** - USB bandwidth is guaranteed
- **No network issues** - works offline, behind firewalls, on isolated networks

### Security
- **Physically secure** - requires physical access to both computers
- **No network exposure** - not visible to other network devices
- **Encrypted** - same SSL/TLS encryption as network connections

### Simplicity
- **No IP addresses** - no network configuration required
- **No port forwarding** - works through any firewall
- **Plug and play** - automatic detection and connection

## Limitations

### Hardware Requirements
- Compatible USB ports on both computers
- Appropriate USB cable (not all USB cables work for A-to-A)
- USB drivers must be available

### Cross-Platform Notes
- **Windows:** Usually works out-of-the-box with CDC-ACM drivers
- **Linux:** Requires CDC-ACM kernel module
- **macOS:** May require additional drivers for some USB configurations

### Range
- **Cable length limit** - USB cables have maximum length restrictions
- **No wireless** - requires physical proximity

## Building with USB Support

USB support is automatically included when building RedkaConnect:

```bash
cmake -S . -B build
cmake --build build
```

No additional dependencies required - uses Qt's QSerialPort class.

## Future Enhancements

1. **USB Device Detection Improvements**
   - Better device identification and naming
   - Support for custom USB device classes

2. **Alternative USB Protocols**
   - Raw USB endpoints (faster but more complex)
   - USB networking (RNDIS/NCM) for broader compatibility

3. **Cable Detection**
   - Hardware detection of cable connection
   - Visual feedback when cable is plugged/unplugged

4. **Multi-Device Support**
   - Connect multiple USB devices simultaneously
   - USB hubs and device chains

## Troubleshooting

### "No USB devices found"
- Check that USB cable is properly connected
- Try a different USB port
- Ensure the other computer is running RedkaConnect in share mode
- Check device manager (Windows) or dmesg (Linux) for USB errors

### "Connection failed"
- Verify the other computer is in "Share This Computer" mode
- Try clicking "Refresh USB Devices"
- Check USB cable compatibility
- Try different USB ports

### "Cable disconnected"
- USB cable was physically unplugged
- Reconnect the cable and click "Plug Back In"
- Check cable and USB ports for damage

USB connections provide a robust alternative to network connections, especially useful in environments where network setup is difficult or unwanted.