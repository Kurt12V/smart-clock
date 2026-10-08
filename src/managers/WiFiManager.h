#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager
{
public:

    enum class Mode
    {
        Disconnected,
        Connecting,
        Connected,
        Setup
    };

public:

    WiFiManager();

    // =========================================================
    // LIFECYCLE
    // =========================================================

    bool begin();
    void update();

    // =========================================================
    // CONNECTION
    // =========================================================

    bool connect();

    bool connect(
        const String& ssid,
        const String& password
    );

    void disconnect();

    // =========================================================
    // SETUP AP
    // =========================================================

    bool startSetupMode();
    void stopSetupMode();

    bool isSetupMode() const;

    // =========================================================
    // CREDENTIALS
    // =========================================================

    bool hasCredentials() const;

    bool saveCredentials(
        const String& ssid,
        const String& password
    );

    void clearCredentials();

    String getSSID() const;

    // =========================================================
    // STATE
    // =========================================================

    Mode getMode() const;

    bool isConnected() const;

    String getIP() const;

    String getAPIP() const;

    int getRSSI() const;

    // =========================================================
    // HOSTNAME
    // =========================================================

    const char* getHostname() const;

    // =========================================================
    // SETUP NETWORK
    // =========================================================

    String getSetupSSID() const;

    String getSetupPassword() const;

    // =========================================================
    // SCAN
    // =========================================================

    int scanNetworks();

    int getScanCount() const;

    String getScanSSID(
        int index
    ) const;

    int getScanRSSI(
        int index
    ) const;

    bool getScanEncrypted(
        int index
    ) const;

private:

    // =========================================================
    // NVS
    // =========================================================

    bool loadCredentials();

    // =========================================================
    // STA
    // =========================================================

    bool beginStation();

    // =========================================================
    // AP
    // =========================================================

    bool beginAccessPoint();

    // =========================================================
    // MDNS
    // =========================================================

    void startMDNS();

    void stopMDNS();

    // =========================================================
    // SETUP CREDENTIALS
    // =========================================================

    String generateSetupSSID() const;

    String generateSetupPassword() const;

private:

    static constexpr const char* HOSTNAME =
        "smartclock";

    static constexpr const char* NVS_NAMESPACE =
        "wifi";

    static constexpr const char* NVS_KEY_SSID =
        "ssid";

    static constexpr const char* NVS_KEY_PASSWORD =
        "pass";

    static constexpr uint32_t CONNECT_TIMEOUT_MS =
        15000;

    static constexpr uint32_t RECONNECT_INTERVAL_MS =
        30000;

    static constexpr uint8_t MAX_CONNECT_ATTEMPTS =
        3;

private:

    String _ssid;
    String _password;

    String _setupSSID;
    String _setupPassword;

    Mode _mode;

    uint32_t _connectStartedAt;
    uint32_t _lastReconnectAttempt;

    uint8_t _connectAttempts;

    bool _mdnsStarted;

    int _scanCount;
};