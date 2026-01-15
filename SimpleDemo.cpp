/*
 * Simple RedkaConnect Demo - Shows key concepts without Qt dependencies
 * Demonstrates the skeuomorphic interface concepts and connection metaphors
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

class RedkaConnectDemo {
public:
    enum class ConnectionState {
        Disconnected,
        Waiting,
        Connecting,
        Connected
    };

    enum class ConnectionType {
        Network,
        USB
    };

    RedkaConnectDemo() : state(ConnectionState::Disconnected), connectionType(ConnectionType::Network) {}

    void run() {
        std::cout << "=== RedkaConnect Demo ===" << std::endl;
        std::cout << "Showcasing new skeuomorphic features!" << std::endl << std::endl;

        showMainMenu();
    }

private:
    void showMainMenu() {
        while (true) {
            updateStatusDisplay();

            std::cout << std::endl << "Choose an option:" << std::endl;
            std::cout << "1. Share This Computer (Network)" << std::endl;
            std::cout << "2. Connect to Computer (Network)" << std::endl;
            std::cout << "3. Connect via USB Cable (NEW!)" << std::endl;
            std::cout << "4. Simulate Connection Error" << std::endl;
            std::cout << "5. Show QR Code Demo" << std::endl;
            std::cout << "6. Exit" << std::endl;
            std::cout << std::endl << "Choice: ";

            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    simulateShare();
                    break;
                case 2:
                    simulateConnect();
                    break;
                case 3:
                    simulateUSBConnect();
                    break;
                case 4:
                    simulateError();
                    break;
                case 5:
                    showQRCodeDemo();
                    break;
                case 6:
                    std::cout << "Goodbye! Thanks for trying RedkaConnect!" << std::endl;
                    return;
                default:
                    std::cout << "Invalid choice. Try again." << std::endl;
            }
        }
    }

    void updateStatusDisplay() {
        std::cout << std::endl << "=== Status Display ===" << std::endl;

        // Skeuomorphic status with monitor and cable icons
        std::string monitorIcon = "🖥️";  // Monitor emoji
        std::string cableIcon;

        switch (state) {
            case ConnectionState::Disconnected:
                cableIcon = "❌";  // Broken cable
                std::cout << monitorIcon << " " << cableIcon << " Ready to Connect" << std::endl;
                std::cout << "Monitor and cable are ready" << std::endl;
                break;
            case ConnectionState::Waiting:
                cableIcon = "⏳";  // Hourglass for waiting
                std::cout << monitorIcon << " " << cableIcon << " Sharing Computer" << std::endl;
                std::cout << "Waiting for someone to connect" << std::endl;
                break;
            case ConnectionState::Connecting:
                cableIcon = "🔄";  // Spinning for connecting
                std::cout << monitorIcon << " " << cableIcon << " Connecting" << std::endl;
                std::cout << "Plugging in the cable..." << std::endl;
                break;
            case ConnectionState::Connected:
                cableIcon = "🔗";  // Connected cable
                std::cout << monitorIcon << " " << cableIcon << " Connected" << std::endl;
                std::cout << "Cable connected to remote computer" << std::endl;
                break;
        }

        if (connectionType == ConnectionType::USB) {
            std::cout << "Using: 🔌 USB Cable Connection" << std::endl;
        } else {
            std::cout << "Using: 🌐 Network Connection" << std::endl;
        }

        std::cout << "====================" << std::endl;
    }

    void simulateShare() {
        connectionType = ConnectionType::Network;
        state = ConnectionState::Waiting;

        std::cout << std::endl << "📤 Sharing your computer..." << std::endl;
        std::cout << "Your pairing code: 847-291" << std::endl;
        std::cout << "Share this code with the computer you want to connect." << std::endl;
        std::cout << std::endl;

        // Simulate waiting
        std::cout << "Waiting for connection";
        for (int i = 0; i < 3; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << ".";
        }
        std::cout << std::endl;

        std::cout << "✅ Connection established!" << std::endl;
        state = ConnectionState::Connected;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    void simulateConnect() {
        connectionType = ConnectionType::Network;

        std::cout << std::endl << "🔍 Looking for computers..." << std::endl;
        std::cout << "Found: John's PC (🖥️), Mary's Laptop (💻)" << std::endl;
        std::cout << std::endl;

        state = ConnectionState::Connecting;
        std::cout << "🔄 Connecting to John's PC..." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "✅ Connected successfully!" << std::endl;
        state = ConnectionState::Connected;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    void simulateUSBConnect() {
        connectionType = ConnectionType::USB;

        std::cout << std::endl << "🔌 USB Cable Connection" << std::endl;
        std::cout << "Please connect a USB A-to-A cable between computers." << std::endl;
        std::cout << std::endl;

        std::cout << "🔍 Scanning for USB devices..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Found: 🔌 RedkaConnect USB Device (COM3)" << std::endl;
        std::cout << "Attempting to connect..." << std::endl;

        state = ConnectionState::Connecting;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "🔗 USB connection established!" << std::endl;
        std::cout << "No network required - direct cable connection." << std::endl;
        state = ConnectionState::Connected;

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void simulateError() {
        std::cout << std::endl << "❌ Connection Error Simulation" << std::endl;
        std::cout << "🔌❌ Connection Lost" << std::endl;
        std::cout << "The cable was unplugged. Check your network connection." << std::endl;
        std::cout << std::endl;
        std::cout << "💡 This is much friendlier than:" << std::endl;
        std::cout << "   ERROR: connection timed out (10060)" << std::endl;
        std::cout << std::endl;

        state = ConnectionState::Disconnected;

        std::cout << "[Plug Back In] button would reconnect here." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    void showQRCodeDemo() {
        std::cout << std::endl << "📱 QR Code Pairing Demo" << std::endl;
        std::cout << "=====================================" << std::endl;
        std::cout << "On sharing computer, a QR code appears containing:" << std::endl;
        std::cout << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "  \"v\": 1," << std::endl;
        std::cout << "  \"id\": \"a1b2c3d4e5f6g7h8\"," << std::endl;
        std::cout << "  \"n\": \"Johns-PC\"," << std::endl;
        std::cout << "  \"p\": \"847291\"," << std::endl;
        std::cout << "  \"a\": \"192.168.1.5\"," << std::endl;
        std::cout << "  \"t\": 1736787600000" << std::endl;
        std::cout << "}" << std::endl;
        std::cout << std::endl;

        std::cout << "User scans with phone camera, copies JSON to clipboard." << std::endl;
        std::cout << "On connecting computer, clicks 'Paste' button." << std::endl;
        std::cout << "PIN auto-fills: 8 ▶️ 4 ▶️ 7 ▶️ 2 ▶️ 9 ▶️ 1" << std::endl;
        std::cout << "Connection establishes automatically! 🎉" << std::endl;
        std::cout << std::endl;

        std::cout << "Benefits:" << std::endl;
        std::cout << "✅ No typing 6-digit codes" << std::endl;
        std::cout << "✅ Works with any phone camera" << std::endl;
        std::cout << "✅ Secure (includes expiry timestamp)" << std::endl;
        std::cout << "✅ No webcam required on computers" << std::endl;
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    ConnectionState state;
    ConnectionType connectionType;
};

int main() {
    std::cout << "Welcome to RedkaConnect Demo!" << std::endl;
    std::cout << "This demonstrates the new skeuomorphic features we've added." << std::endl;
    std::cout << std::endl;

    RedkaConnectDemo demo;
    demo.run();

    return 0;
}