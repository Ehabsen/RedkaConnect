# Skeuomorphic RedkaConnect Interface

## Overview

RedkaConnect now features a highly skeuomorphic (real-world inspired) interface that uses familiar metaphors to make the software more intuitive and accessible.

## Visual Metaphors

### 🖥️ Monitor Icons (Instead of Generic Boxes)

**Before:**
- Generic squares/rectangles
- Technical "video-display.png" icons

**Now:**
- Realistic monitor emojis (🖥️ for servers, 💻 for clients)
- Cable connection indicators (🔗, ❌, ⏳, 🔄)

### 🔌 Unplugged Cable Error Messages

**Before:**
```
ERROR: connection timed out (10060)
Failed to bind to port 24800
Network interface error
```

**Now:**
```
🔌❌ Connection Lost
The cable was unplugged. Check your network connection.

[Plug Back In] Button
```

## Interface States

### 1. Ready State (Home Page)
```
🖥️ 🔗 Ready to Connect
Monitor and cable are ready
```

### 2. Sharing State
```
🖥️ ⏳ Sharing Computer
Waiting for someone to connect
```

### 3. Connecting State
```
🖥️ 🔄 Connecting
Plugging in the cable...
```

### 4. Connected State
```
🖥️ 🔗 Connected
Cable connected to "John's PC"
```

### 5. Error State
```
🔌❌ Connection Lost
The cable was unplugged. Check your network connection.

[Plug Back In] Button
```

## Technical Implementation

### Status Display Components
- `m_statusMonitorIcon` - Shows monitor emoji
- `m_statusCableIcon` - Shows cable/link status
- `m_statusTitleLabel` - Main status text
- `m_statusMessageLabel` - Descriptive message

### Error Page
- Large unplugged cable graphic (🔌❌)
- Red glow effect
- "Plug Back In" button (mapped to disconnect/reconnect)
- Friendly, non-technical language

### State Management
- `setState()` updates both visual and functional state
- Icons change based on connection status
- Error handling shows cable metaphor instead of error codes

## User Experience Benefits

### Familiarity
- Users understand "unplugged cable" instantly
- Monitor icons look like real computers
- Cable metaphors match physical networking

### Reduced Anxiety
- No intimidating technical error messages
- Clear visual feedback for all states
- Simple actions ("Plug back in") instead of complex fixes

### Accessibility
- Visual metaphors work across languages
- No need to understand technical terms
- Intuitive problem-solving

## Building with Skeuomorphic UI

The skeuomorphic features are automatically included in the main build:

```bash
cmake -S . -B build
cmake --build build
```

## Examples

### Normal Operation
- **Ready:** Monitor + green link = Ready to use
- **Sharing:** Monitor + hourglass = Waiting patiently
- **Connected:** Monitor + green link + name = Successfully connected

### Error Handling
- **Network Issue:** Unplugged cable graphic + "Plug back in" button
- **Connection Lost:** Same visual treatment
- **Discovery Failed:** Cable unplugged metaphor

### Device Discovery
- **Server devices:** 🖥️ with "Sharing" badge
- **Client devices:** 💻 with "Available" badge
- Both show as realistic computer monitors

This approach makes RedkaConnect feel more like familiar physical networking equipment rather than abstract software.