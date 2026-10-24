#include "WiFiManager.h"

#include <Preferences.h>
#include <ESPmDNS.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

WiFiManager::WiFiManager()
    : _ssid()
    , _password()
    , _setupSSID()
    , _setupPassword()
    , _mode(Mode::Disconnected)
    , _connectStartedAt(0)
    , _lastReconnectAttempt(0)
    , _connectAttempts(0)
    , _mdnsStarted(false)
    , _scanCount(0)
{
}

// ============================================================
// BEGIN
// ============================================================

bool WiFiManager::begin()
{
    Serial0.println();
    Serial0.println("============================================");
    Serial0.println("[WIFI] WiFiManager BEGIN");
    Serial0.println("============================================");

    WiFi.persistent(false);

    WiFi.setAutoReconnect(false);

    WiFi.mode(WIFI_STA);

    WiFi.setHostname(HOSTNAME);

    if (!loadCredentials())
    {
        Serial0.println("[WIFI] No saved credentials");
        Serial0.println("[WIFI] Starting setup AP");

        return startSetupMode();
    }

    Serial0.println("[WIFI] Saved credentials found");
    Serial0.print("[WIFI] SSID: ");
    Serial0.println(_ssid);

    _connectAttempts = 0;

    return beginStation();
}

// ============================================================
// UPDATE
// ============================================================

void WiFiManager::update()
{
    switch (_mode)
    {
        // ====================================================
        // CONNECTING
        // ====================================================

        case Mode::Connecting:
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                _mode = Mode::Connected;

                _connectAttempts = 0;

                Serial0.println();
                Serial0.println("[WIFI] Connected");

                Serial0.print("[WIFI] SSID: ");
                Serial0.println(WiFi.SSID());

                Serial0.print("[WIFI] IP: ");
                Serial0.println(WiFi.localIP());

                Serial0.print("[WIFI] RSSI: ");
                Serial0.println(WiFi.RSSI());

                startMDNS();

                break;
            }

            if (
                millis() - _connectStartedAt
                >= CONNECT_TIMEOUT_MS
            )
            {
                Serial0.println();
                Serial0.println(
                    "[WIFI] Connection timeout"
                );

                WiFi.disconnect(true, false);

                _connectAttempts++;

                if (
                    _connectAttempts
                    >= MAX_CONNECT_ATTEMPTS
                )
                {
                    Serial0.println(
                        "[WIFI] Connection failed"
                    );

                    Serial0.println(
                        "[WIFI] Starting setup AP"
                    );

                    startSetupMode();
                }
                else
                {
                    Serial0.print(
                        "[WIFI] Retry "
                    );

                    Serial0.print(
                        _connectAttempts
                    );

                    Serial0.print(
                        "/"
                    );

                    Serial0.println(
                        MAX_CONNECT_ATTEMPTS
                    );

                    beginStation();
                }
            }

            break;
        }

        // ====================================================
        // CONNECTED
        // ====================================================

        case Mode::Connected:
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial0.println(
                    "[WIFI] Connection lost"
                );

                stopMDNS();

                _mode = Mode::Disconnected;

                _lastReconnectAttempt = millis();
            }

            break;
        }

        // ====================================================
        // DISCONNECTED
        // ====================================================

        case Mode::Disconnected:
        {
            if (!hasCredentials())
            {
                startSetupMode();
                break;
            }

            if (
                millis() - _lastReconnectAttempt
                >= RECONNECT_INTERVAL_MS
            )
            {
                _lastReconnectAttempt = millis();

                Serial0.println(
                    "[WIFI] Trying reconnect..."
                );

                _connectAttempts = 0;

                beginStation();
            }

            break;
        }

        // ====================================================
        // SETUP
        // ====================================================

        case Mode::Setup:
        {
            // Nothing to do here.
            //
            // The ESP32 stays in AP mode until:
            //
            // POST /api/wifi/connect
            //
            // saves new credentials and restarts ESP32.

            break;
        }
    }
}

// ============================================================
// CONNECT
// ============================================================

bool WiFiManager::connect()
{
    if (!hasCredentials())
    {
        Serial0.println(
            "[WIFI] Cannot connect: no credentials"
        );

        return startSetupMode();
    }

    return beginStation();
}

// ============================================================
// CONNECT WITH NEW CREDENTIALS
// ============================================================

bool WiFiManager::connect(
    const String& ssid,
    const String& password
)
{
    if (ssid.isEmpty())
    {
        Serial0.println(
            "[WIFI] SSID is empty"
        );

        return false;
    }

    if (!saveCredentials(
        ssid,
        password
    ))
    {
        return false;
    }

    Serial0.println(
        "[WIFI] New credentials saved"
    );

    Serial0.print(
        "[WIFI] SSID: "
    );

    Serial0.println(
        _ssid
    );

    /*
     * Important:
     *
     * We do NOT immediately switch to STA here.
     *
     * The web handler will send the HTTP response
     * and then restart the ESP32.
     *
     * After reboot:
     *
     * WiFiManager::begin()
     *      ->
     * loadCredentials()
     *      ->
     * beginStation()
     */

    return true;
}

// ============================================================
// DISCONNECT
// ============================================================

void WiFiManager::disconnect()
{
    stopMDNS();

    WiFi.disconnect(true, false);

    _mode = Mode::Disconnected;

    Serial0.println(
        "[WIFI] Disconnected"
    );
}

// ============================================================
// SETUP MODE
// ============================================================

bool WiFiManager::startSetupMode()
{
    stopMDNS();

    WiFi.disconnect(true, false);

    delay(100);

    WiFi.mode(WIFI_AP);

    WiFi.setHostname(HOSTNAME);

    return beginAccessPoint();
}

// ============================================================
// BEGIN ACCESS POINT
// ============================================================

bool WiFiManager::beginAccessPoint()
{
    _setupSSID =
        generateSetupSSID();

    _setupPassword =
        generateSetupPassword();

    Serial0.println();
    Serial0.println(
        "============================================"
    );

    Serial0.println(
        "[WIFI] SETUP MODE"
    );

    Serial0.println(
        "============================================"
    );

    Serial0.print(
        "[WIFI] SSID: "
    );

    Serial0.println(
        _setupSSID
    );

    Serial0.print(
        "[WIFI] Password: "
    );

    Serial0.println(
        _setupPassword
    );

    IPAddress localIP(
        192,
        168,
        4,
        1
    );

    IPAddress gateway(
        192,
        168,
        4,
        1
    );

    IPAddress subnet(
        255,
        255,
        255,
        0
    );

    if (
        !WiFi.softAPConfig(
            localIP,
            gateway,
            subnet
        )
    )
    {
        Serial0.println(
            "[WIFI] softAPConfig failed"
        );

        return false;
    }

    if (
        !WiFi.softAP(
            _setupSSID.c_str(),
            _setupPassword.c_str()
        )
    )
    {
        Serial0.println(
            "[WIFI] softAP failed"
        );

        return false;
    }

    _mode = Mode::Setup;

    Serial0.println(
        "[WIFI] Setup AP started"
    );

    Serial0.print(
        "[WIFI] Setup IP: "
    );

    Serial0.println(
        WiFi.softAPIP()
    );

    return true;
}

// ============================================================
// STOP SETUP MODE
// ============================================================

void WiFiManager::stopSetupMode()
{
    if (_mode != Mode::Setup)
    {
        return;
    }

    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_STA);

    _mode = Mode::Disconnected;

    Serial0.println(
        "[WIFI] Setup AP stopped"
    );
}

// ============================================================
// IS SETUP MODE
// ============================================================

bool WiFiManager::isSetupMode() const
{
    return _mode == Mode::Setup;
}

// ============================================================
// HAS CREDENTIALS
// ============================================================

bool WiFiManager::hasCredentials() const
{
    return !_ssid.isEmpty();
}

// ============================================================
// SAVE CREDENTIALS
// ============================================================

bool WiFiManager::saveCredentials(
    const String& ssid,
    const String& password
)
{
    if (ssid.isEmpty())
    {
        return false;
    }

    Preferences preferences;

    if (
        !preferences.begin(
            NVS_NAMESPACE,
            false
        )
    )
    {
        Serial0.println(
            "[WIFI] Preferences begin failed"
        );

        return false;
    }

    bool ok = true;

    if (
        preferences.putString(
            NVS_KEY_SSID,
            ssid
        ) == 0
    )
    {
        ok = false;
    }

    if (
        preferences.putString(
            NVS_KEY_PASSWORD,
            password
        ) == 0
        &&
        !password.isEmpty()
    )
    {
        ok = false;
    }

    preferences.end();

    if (!ok)
    {
        Serial0.println(
            "[WIFI] Failed to save credentials"
        );

        return false;
    }

    _ssid = ssid;
    _password = password;

    return true;
}

// ============================================================
// LOAD CREDENTIALS
// ============================================================

bool WiFiManager::loadCredentials()
{
    Preferences preferences;

    if (
        !preferences.begin(
            NVS_NAMESPACE,
            true
        )
    )
    {
        Serial0.println(
            "[WIFI] Preferences open failed"
        );

        return false;
    }

    _ssid =
        preferences.getString(
            NVS_KEY_SSID,
            ""
        );

    _password =
        preferences.getString(
            NVS_KEY_PASSWORD,
            ""
        );

    preferences.end();

    return !_ssid.isEmpty();
}

// ============================================================
// CLEAR CREDENTIALS
// ============================================================

void WiFiManager::clearCredentials()
{
    Preferences preferences;

    if (
        preferences.begin(
            NVS_NAMESPACE,
            false
        )
    )
    {
        preferences.clear();

        preferences.end();
    }

    _ssid = "";
    _password = "";

    Serial0.println(
        "[WIFI] Credentials cleared"
    );
}

// ============================================================
// GET SSID
// ============================================================

String WiFiManager::getSSID() const
{
    return _ssid;
}

// ============================================================
// BEGIN STATION
// ============================================================

bool WiFiManager::beginStation()
{
    if (_ssid.isEmpty())
    {
        return startSetupMode();
    }

    stopMDNS();

    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_STA);

    WiFi.setHostname(HOSTNAME);

    WiFi.disconnect(true, false);

    delay(100);

    Serial0.println();
    Serial0.println(
        "[WIFI] Connecting..."
    );

    Serial0.print(
        "[WIFI] SSID: "
    );

    Serial0.println(
        _ssid
    );

    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    _connectStartedAt =
        millis();

    _mode =
        Mode::Connecting;

    return true;
}

// ============================================================
// GET MODE
// ============================================================

WiFiManager::Mode
WiFiManager::getMode() const
{
    return _mode;
}

// ============================================================
// IS CONNECTED
// ============================================================

bool WiFiManager::isConnected() const
{
    return
        _mode == Mode::Connected &&
        WiFi.status() == WL_CONNECTED;
}

// ============================================================
// GET IP
// ============================================================

String WiFiManager::getIP() const
{
    if (!isConnected())
    {
        return "";
    }

    return WiFi.localIP().toString();
}

// ============================================================
// GET AP IP
// ============================================================

String WiFiManager::getAPIP() const
{
    if (!isSetupMode())
    {
        return "";
    }

    return WiFi.softAPIP().toString();
}

// ============================================================
// GET RSSI
// ============================================================

int WiFiManager::getRSSI() const
{
    if (!isConnected())
    {
        return 0;
    }

    return WiFi.RSSI();
}

// ============================================================
// HOSTNAME
// ============================================================

const char*
WiFiManager::getHostname() const
{
    return HOSTNAME;
}

// ============================================================
// SETUP SSID
// ============================================================

String WiFiManager::getSetupSSID() const
{
    return _setupSSID;
}

// ============================================================
// SETUP PASSWORD
// ============================================================

String WiFiManager::getSetupPassword() const
{
    return _setupPassword;
}

// ============================================================
// GENERATE SETUP SSID
// ============================================================

String WiFiManager::generateSetupSSID() const
{
    uint64_t mac =
        ESP.getEfuseMac();

    uint16_t suffix =
        static_cast<uint16_t>(
            mac & 0xFFFF
        );

    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "SmartClock-%04X",
        suffix
    );

    return String(buffer);
}

// ============================================================
// GENERATE SETUP PASSWORD
// ============================================================

String WiFiManager::generateSetupPassword() const
{
    uint64_t mac =
        ESP.getEfuseMac();

    uint32_t suffix =
        static_cast<uint32_t>(
            mac & 0xFFFFFF
        );

    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "SC%06X",
        suffix
    );

    return String(buffer);
}

// ============================================================
// MDNS
// ============================================================

void WiFiManager::startMDNS()
{
    if (_mdnsStarted)
    {
        return;
    }

    if (
        MDNS.begin(HOSTNAME)
    )
    {
        MDNS.addService(
            "http",
            "tcp",
            80
        );

        _mdnsStarted = true;

        Serial0.println(
            "[WIFI] mDNS started"
        );

        Serial0.println(
            "[WIFI] http://smartclock.local"
        );
    }
    else
    {
        Serial0.println(
            "[WIFI] mDNS start failed"
        );
    }
}

// ============================================================
// STOP MDNS
// ============================================================

void WiFiManager::stopMDNS()
{
    if (!_mdnsStarted)
    {
        return;
    }

    MDNS.end();

    _mdnsStarted = false;

    Serial0.println(
        "[WIFI] mDNS stopped"
    );
}

// ============================================================
// SCAN
// ============================================================

int WiFiManager::scanNetworks()
{
    if (isSetupMode())
    {
        WiFi.mode(WIFI_AP_STA);
    }
    else
    {
        WiFi.mode(WIFI_STA);
    }

    Serial0.println(
        "[WIFI] Scanning networks..."
    );

    _scanCount =
        WiFi.scanNetworks(
            false,
            true
        );

    if (_scanCount < 0)
    {
        _scanCount = 0;

        Serial0.println(
            "[WIFI] Scan failed"
        );

        return 0;
    }

    Serial0.print(
        "[WIFI] Networks found: "
    );

    Serial0.println(
        _scanCount
    );

    return _scanCount;
}

// ============================================================
// GET SCAN COUNT
// ============================================================

int WiFiManager::getScanCount() const
{
    return _scanCount;
}

// ============================================================
// GET SCAN SSID
// ============================================================

String WiFiManager::getScanSSID(
    int index
) const
{
    if (
        index < 0 ||
        index >= _scanCount
    )
    {
        return "";
    }

    return WiFi.SSID(index);
}

// ============================================================
// GET SCAN RSSI
// ============================================================

int WiFiManager::getScanRSSI(
    int index
) const
{
    if (
        index < 0 ||
        index >= _scanCount
    )
    {
        return 0;
    }

    return WiFi.RSSI(index);
}

// ============================================================
// GET SCAN ENCRYPTION
// ============================================================

bool WiFiManager::getScanEncrypted(
    int index
) const
{
    if (
        index < 0 ||
        index >= _scanCount
    )
    {
        return false;
    }

    return WiFi.encryptionType(index)
        != WIFI_AUTH_OPEN;
}