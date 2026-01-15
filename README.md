# RedkaConnect

<!-- add badges: flatpak, snap, GitHub Actions -->

**Seamless keyboard and mouse sharing for Windows computers**

### What is RedkaConnect?

RedkaConnect is modern software that lets you control multiple Windows computers with a single keyboard and mouse. Just like a KVM switch, but in software - move your mouse to the edge of the screen to switch between computers.

**Key Features:**
- 🖥️ **Network sharing** - Control computers across your local network
- 🔌 **USB cable support** - Direct cable connection (no network required)
- 📱 **QR code pairing** - Easy setup with phone camera
- 📋 **Clipboard sharing** - Copy and paste between computers
- 🪟 **Windows optimized** - Built specifically for Windows 10/11

### Why RedkaConnect?

**Simple & Reliable**
- Built for Windows users who want hassle-free computer control
- No complex network setup required
- Works out-of-the-box with modern Windows computers

**Flexible Connections**
- **Network mode**: Share across your local network (WiFi or Ethernet)
- **USB mode**: Direct cable connection for maximum security and speed
- **QR pairing**: Scan codes with your phone for instant setup

**Modern Interface**
- Clean, intuitive design with familiar computer/cable metaphors
- Visual status indicators show connection state at a glance
- Error messages you can actually understand

## Quick Start

### Option 1: Network Connection (Recommended)

1. **On the main computer** (the one with your keyboard/mouse):
   - Launch RedkaConnect
   - Click **"📤 Share This Computer"**
   - Note the 6-digit PIN code displayed

2. **On the second computer**:
   - Launch RedkaConnect
   - Click **"📥 Connect to Computer"**
   - Enter the PIN from step 1
   - Click Connect

3. **Done!** Move your mouse to the edge of the screen to switch computers

### Option 2: USB Cable Connection

1. **Get a USB cable**:
   - USB A-to-A cable (for older computers)
   - USB A-to-C or C-to-C adapters (for modern computers)
   - *Note: Not all USB cables work for computer-to-computer connections*

2. **On the main computer**:
   - Launch RedkaConnect
   - Click **"📤 Share This Computer"**

3. **On the second computer**:
   - Launch RedkaConnect
   - Click **"🔌 Connect via USB Cable"**
   - Connect the USB cable between computers
   - Select the detected USB device and click Connect

4. **Done!** Instant connection with no network setup required

### Visual Status Guide

- 🖥️ 🔗 **Ready** - Everything is set up correctly
- 🖥️ ⏳ **Sharing** - Waiting for someone to connect
- 🖥️ 🔄 **Connecting** - Establishing connection
- 🖥️ 🔗 **Connected** - Successfully linked!
- 🔌❌ **Cable Error** - Check your connection and try "Plug Back In"

## System Requirements

- **Windows 10 or 11** (64-bit only)
- **USB ports** for cable connections (optional)
- **Network connection** for network sharing (optional)

## Troubleshooting

### Network Connection Issues
- Make sure both computers are on the same network
- Check Windows Firewall settings
- Try temporarily disabling antivirus software
- Ensure the PIN hasn't expired (valid for 5 minutes)

### USB Connection Issues
- Use a proper USB A-to-A cable (not just any USB cable)
- Try different USB ports on both computers
- Make sure the sharing computer is running first
- Check Device Manager for USB errors

### General Problems
- 🔌❌ **"Connection Lost"** → Click "Plug Back In" or restart both computers
- 🖥️ 🔄 **Stuck connecting** → Close and restart RedkaConnect on both computers
- Scroll Lock prevents mouse switching → Turn off Scroll Lock

## Support

Found a bug or need help? [Create an issue](https://github.com/Ehabsen/RedkaConnect/issues) on GitHub.

Please include:
- Windows version
- Connection method (Network/USB)
- Steps to reproduce the problem
- What you expected to happen vs. what actually happened

## Downloads

Get the latest Windows installer from [Releases](https://github.com/Ehabsen/RedkaConnect/releases)


## FAQ

**Q: What's the difference between Network and USB connections?**

> **Network**: Works over WiFi/Ethernet, requires both computers on same network, easier for multiple computers
> **USB**: Direct cable connection, ultra-fast and secure, no network setup needed, works offline

**Q: What USB cable do I need?**

> For computer-to-computer connections, you need a special USB cable. Regular charging cables usually won't work. Look for "USB data transfer cable" or "USB A to A cable" on Amazon/eBay.

**Q: Can I connect more than 2 computers?**

> Yes! One computer runs in "Share" mode, and multiple others can connect via network. USB connections are currently limited to 2 computers.

**Q: Is my connection secure?**

> Both network and USB connections are encrypted. USB connections are physically secure (someone needs physical access to both computers).

**Q: Why does the connection drop sometimes?**

> Network: Check WiFi signal, try Ethernet cable, or use USB mode
> USB: Try different USB ports, ensure cable is properly seated, check for USB power management issues



> A: Edit your configuration to include the server's ip address manually with
>
>```
>(...)
>
>section: options
>    serverhostname=<AAA.BBB.CCC.DDD>
>```

**Q: Does it work with different screen resolutions?**

> Yes! RedkaConnect automatically handles different screen sizes and resolutions seamlessly.
