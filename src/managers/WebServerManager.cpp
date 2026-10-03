#include "WebServerManager.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>


// =============================================================
// CONSTRUCTOR
// =============================================================

WebServerManager::WebServerManager()
    : _server(80),
      _settings(nullptr),
      _sd(nullptr),
      _sound(nullptr),
      _clockSystem(nullptr),
      _alarmManager(nullptr),
      _alarm(),
      _timer(),
      _initialized(false)
{
}


// =============================================================
// BEGIN
// =============================================================

bool WebServerManager::begin(
    SettingsManager& settings,
    SDManager& sd,
    SoundManager& sound,
    ClockSystem& clockSystem,
    AlarmManager& alarmManager,
    const char* ssid,
    const char* password
)
{
    _settings = &settings;
    _sd = &sd;
    _sound = &sound;
    _clockSystem = &clockSystem;
    _alarmManager = &alarmManager;


    // =========================================================
    // WIFI
    // =========================================================

    Serial0.println("[WEB] Starting WiFi...");

    WiFi.mode(WIFI_STA);

    WiFi.disconnect(true, true);

    delay(300);

    WiFi.begin(
        ssid,
        password
    );


    const uint32_t startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000UL
    )
    {
        delay(250);

        Serial0.print(".");
    }


    Serial0.println();


    if (
        WiFi.status() != WL_CONNECTED
    )
    {
        Serial0.println(
            "[WEB] WiFi connection failed"
        );

        return false;
    }


    Serial0.print(
        "[WEB] WiFi connected: "
    );

    Serial0.println(
        WiFi.localIP()
    );


    // =========================================================
    // LITTLEFS
    // =========================================================

    Serial0.println(
        "[WEB] Mounting LittleFS..."
    );


    if (
        !LittleFS.begin(true)
    )
    {
        Serial0.println(
            "[WEB] LittleFS failed"
        );

        return false;
    }


    Serial0.println(
        "[WEB] LittleFS OK"
    );


    // =========================================================
    // ROUTES
    // =========================================================

    setupRoutes();


    // =========================================================
    // SERVER
    // =========================================================

    _server.begin();

    _initialized = true;


    Serial0.println(
        "[WEB] Web server started"
    );


    return true;
}


// =============================================================
// ROUTES
// =============================================================

void WebServerManager::setupRoutes()
{
    // =========================================================
    // ROOT
    // =========================================================

    _server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        }
    );


    // =========================================================
    // SETTINGS
    // =========================================================

    _server.on(
        "/api/param",
        HTTP_GET,
        [this]()
        {
            handleGetParam();
        }
    );


    _server.on(
        "/api/param",
        HTTP_POST,
        [this]()
        {
            handleSetParam();
        }
    );


    _server.on(
        "/api/params",
        HTTP_GET,
        [this]()
        {
            handleGetAllParams();
        }
    );


    _server.on(
        "/api/reset",
        HTTP_POST,
        [this]()
        {
            handleReset();
        }
    );


    // =========================================================
    // TIME
    // =========================================================

    _server.on(
        "/api/time",
        HTTP_GET,
        [this]()
        {
            handleTime();
        }
    );


    _server.on(
        "/api/time/sync",
        HTTP_POST,
        [this]()
        {
            handleTimeSync();
        }
    );


    // =========================================================
    // LEGACY ALARM
    // =========================================================

    _server.on(
        "/api/alarm",
        HTTP_GET,
        [this]()
        {
            handleGetAlarm();
        }
    );


    _server.on(
        "/api/alarm",
        HTTP_POST,
        [this]()
        {
            handleSetAlarm();
        }
    );


    // =========================================================
    // ALARMS
    // =========================================================

    _server.on(
        "/api/alarms",
        HTTP_GET,
        [this]()
        {
            handleGetAlarms();
        }
    );


    _server.on(
        "/api/alarms",
        HTTP_POST,
        [this]()
        {
            handleCreateAlarm();
        }
    );


    // =========================================================
    // TIMER
    // =========================================================

    _server.on(
        "/api/timer",
        HTTP_GET,
        [this]()
        {
            handleGetTimer();
        }
    );


    _server.on(
        "/api/timer",
        HTTP_POST,
        [this]()
        {
            handleSetTimer();
        }
    );


    // =========================================================
    // AUDIO
    // =========================================================

    _server.on(
        "/api/audio/play",
        HTTP_POST,
        [this]()
        {
            handleAudioPlay();
        }
    );


    _server.on(
        "/api/audio/pause",
        HTTP_POST,
        [this]()
        {
            handleAudioPause();
        }
    );


    _server.on(
        "/api/audio/resume",
        HTTP_POST,
        [this]()
        {
            handleAudioResume();
        }
    );


    _server.on(
        "/api/audio/stop",
        HTTP_POST,
        [this]()
        {
            handleAudioStop();
        }
    );


    _server.on(
        "/api/audio/status",
        HTTP_GET,
        [this]()
        {
            handleAudioStatus();
        }
    );


    // =========================================================
    // SD
    // =========================================================

    _server.on(
        "/api/sd",
        HTTP_GET,
        [this]()
        {
            handleSD();
        }
    );


    // =========================================================
    // SENSORS
    // =========================================================

    _server.on(
        "/api/sensors",
        HTTP_GET,
        [this]()
        {
            handleSensors();
        }
    );


    // =========================================================
    // NOT FOUND
    // =========================================================

    _server.onNotFound(
        [this]()
        {
            handleNotFound();
        }
    );
}


// =============================================================
// ROOT
// =============================================================

void WebServerManager::handleRoot()
{
    if (
        !LittleFS.exists(
            "/index.html"
        )
    )
    {
        _server.send(
            404,
            "text/plain",
            "index.html not found"
        );

        return;
    }


    File file =
        LittleFS.open(
            "/index.html",
            "r"
        );


    if (!file)
    {
        _server.send(
            500,
            "text/plain",
            "failed to open index.html"
        );

        return;
    }


    _server.streamFile(
        file,
        "text/html"
    );


    file.close();
}


// =============================================================
// NOT FOUND
// =============================================================

void WebServerManager::handleNotFound()
{
    const String uri =
        _server.uri();


    // =========================================================
    // /api/alarms/{id}
    // =========================================================

    if (
        uri.startsWith(
            "/api/alarms/"
        )
    )
    {
        String id;


        if (
            !getAlarmIdFromUri(
                id
            )
        )
        {
            sendError(
                400,
                "invalid alarm id"
            );

            return;
        }


        switch (
            _server.method()
        )
        {
            case HTTP_GET:

                handleGetAlarmById();

                return;


            case HTTP_PUT:

                handleUpdateAlarmById();

                return;


            case HTTP_DELETE:

                handleDeleteAlarmById();

                return;


            default:

                sendError(
                    405,
                    "method not allowed"
                );

                return;
        }
    }


    _server.send(
        404,
        "application/json",
        "{\"ok\":false,\"error\":\"not found\"}"
    );
}


// =============================================================
// PARAMETER NAME → ID
// =============================================================

static bool paramFromName(
    const char* name,
    SettingsManager::Id& id
)
{
    if (!name)
        return false;


    if (strcmp(name, "display_brightness") == 0)
    {
        id = SettingsManager::Id::DISPLAY_BRIGHTNESS;
        return true;
    }


    if (strcmp(name, "matrix_enabled") == 0)
    {
        id = SettingsManager::Id::MATRIX_ENABLED;
        return true;
    }


    if (strcmp(name, "matrix_brightness") == 0)
    {
        id = SettingsManager::Id::MATRIX_BRIGHTNESS;
        return true;
    }


    if (strcmp(name, "matrix_effect") == 0)
    {
        id = SettingsManager::Id::MATRIX_EFFECT;
        return true;
    }


    if (strcmp(name, "matrix_speed") == 0)
    {
        id = SettingsManager::Id::MATRIX_SPEED;
        return true;
    }


    if (strcmp(name, "cob_enabled") == 0)
    {
        id = SettingsManager::Id::COB_ENABLED;
        return true;
    }


    if (strcmp(name, "cob_brightness") == 0)
    {
        id = SettingsManager::Id::COB_BRIGHTNESS;
        return true;
    }


    if (strcmp(name, "cob_effect") == 0)
    {
        id = SettingsManager::Id::COB_EFFECT;
        return true;
    }


    if (strcmp(name, "cob_speed") == 0)
    {
        id = SettingsManager::Id::COB_SPEED;
        return true;
    }


    if (strcmp(name, "mic_enabled") == 0)
    {
        id = SettingsManager::Id::MIC_ENABLED;
        return true;
    }


    if (strcmp(name, "volume_media") == 0)
    {
        id = SettingsManager::Id::VOLUME_MEDIA;
        return true;
    }


    if (strcmp(name, "volume_alarm") == 0)
    {
        id = SettingsManager::Id::VOLUME_ALARM;
        return true;
    }


    if (strcmp(name, "volume_system") == 0)
    {
        id = SettingsManager::Id::VOLUME_SYSTEM;
        return true;
    }


    if (strcmp(name, "utc_offset") == 0)
    {
        id = SettingsManager::Id::UTC_OFFSET;
        return true;
    }


    return false;
}


// =============================================================
// ID → NAME
// =============================================================

static const char* paramName(
    SettingsManager::Id id
)
{
    switch (id)
    {
        case SettingsManager::Id::DISPLAY_BRIGHTNESS:
            return "display_brightness";

        case SettingsManager::Id::MATRIX_ENABLED:
            return "matrix_enabled";

        case SettingsManager::Id::MATRIX_BRIGHTNESS:
            return "matrix_brightness";

        case SettingsManager::Id::MATRIX_EFFECT:
            return "matrix_effect";

        case SettingsManager::Id::MATRIX_SPEED:
            return "matrix_speed";

        case SettingsManager::Id::COB_ENABLED:
            return "cob_enabled";

        case SettingsManager::Id::COB_BRIGHTNESS:
            return "cob_brightness";

        case SettingsManager::Id::COB_EFFECT:
            return "cob_effect";

        case SettingsManager::Id::COB_SPEED:
            return "cob_speed";

        case SettingsManager::Id::MIC_ENABLED:
            return "mic_enabled";

        case SettingsManager::Id::VOLUME_MEDIA:
            return "volume_media";

        case SettingsManager::Id::VOLUME_ALARM:
            return "volume_alarm";

        case SettingsManager::Id::VOLUME_SYSTEM:
            return "volume_system";

        case SettingsManager::Id::UTC_OFFSET:
            return "utc_offset";

        default:
            return "";
    }
}


// =============================================================
// GET PARAM VALUE
// =============================================================

static int getParamValue(
    const SettingsManager& settings,
    SettingsManager::Id id
)
{
    switch (id)
    {
        case SettingsManager::Id::DISPLAY_BRIGHTNESS:
            return settings.getDisplayBrightness();

        case SettingsManager::Id::MATRIX_ENABLED:
            return settings.isMatrixEnabled();

        case SettingsManager::Id::MATRIX_BRIGHTNESS:
            return settings.getMatrixBrightness();

        case SettingsManager::Id::MATRIX_EFFECT:
            return settings.getMatrixEffect();

        case SettingsManager::Id::MATRIX_SPEED:
            return settings.getMatrixSpeed();

        case SettingsManager::Id::COB_ENABLED:
            return settings.isCobEnabled();

        case SettingsManager::Id::COB_BRIGHTNESS:
            return settings.getCobBrightness();

        case SettingsManager::Id::COB_EFFECT:
            return settings.getCobEffect();

        case SettingsManager::Id::COB_SPEED:
            return settings.getCobSpeed();

        case SettingsManager::Id::MIC_ENABLED:
            return settings.isMicEnabled();

        case SettingsManager::Id::VOLUME_MEDIA:
            return settings.getMediaVolume();

        case SettingsManager::Id::VOLUME_ALARM:
            return settings.getAlarmVolume();

        case SettingsManager::Id::VOLUME_SYSTEM:
            return settings.getSystemVolume();

        case SettingsManager::Id::UTC_OFFSET:
            return settings.getUtcOffset();

        default:
            return 0;
    }
}


// =============================================================
// SET PARAM VALUE
// =============================================================

static bool setParamValue(
    SettingsManager& settings,
    SettingsManager::Id id,
    int value
)
{
    switch (id)
    {
        case SettingsManager::Id::DISPLAY_BRIGHTNESS:

            return settings.setDisplayBrightness(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::MATRIX_ENABLED:

            return settings.setMatrixEnabled(
                value != 0
            );


        case SettingsManager::Id::MATRIX_BRIGHTNESS:

            return settings.setMatrixBrightness(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::MATRIX_EFFECT:

            return settings.setMatrixEffect(
                static_cast<uint8_t>(
                    constrain(value, 0, 255)
                )
            );


        case SettingsManager::Id::MATRIX_SPEED:

            return settings.setMatrixSpeed(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::COB_ENABLED:

            return settings.setCobEnabled(
                value != 0
            );


        case SettingsManager::Id::COB_BRIGHTNESS:

            return settings.setCobBrightness(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::COB_EFFECT:

            return settings.setCobEffect(
                static_cast<uint8_t>(
                    constrain(value, 0, 255)
                )
            );


        case SettingsManager::Id::COB_SPEED:

            return settings.setCobSpeed(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::MIC_ENABLED:

            return settings.setMicEnabled(
                value != 0
            );


        case SettingsManager::Id::VOLUME_MEDIA:

            return settings.setMediaVolume(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::VOLUME_ALARM:

            return settings.setAlarmVolume(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::VOLUME_SYSTEM:

            return settings.setSystemVolume(
                static_cast<uint8_t>(
                    constrain(value, 0, 100)
                )
            );


        case SettingsManager::Id::UTC_OFFSET:

            return settings.setUtcOffset(
                static_cast<int8_t>(
                    constrain(value, -12, 14)
                )
            );


        default:
            return false;
    }
}


// =============================================================
// GET PARAM
// =============================================================

void WebServerManager::handleGetParam()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }


    String name =
        _server.arg("name");


    if (name.isEmpty())
    {
        name =
            _server.arg("param");
    }


    if (name.isEmpty())
    {
        sendError(
            400,
            "missing name"
        );

        return;
    }


    SettingsManager::Id id;


    if (
        !paramFromName(
            name.c_str(),
            id
        )
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }


    JsonDocument doc;


    doc["ok"] = true;

    doc["param"] =
        paramName(id);

    doc["value"] =
        getParamValue(
            *_settings,
            id
        );


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// SET PARAM
// =============================================================

void WebServerManager::handleSetParam()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }


    JsonDocument doc;


    if (!parseJson(doc))
        return;


    const char* name =
        doc["name"];


    if (!name)
    {
        name =
            doc["param"];
    }


    if (!name)
    {
        sendError(
            400,
            "missing name"
        );

        return;
    }


    // =========================================================
    // NTP
    // =========================================================

    if (
        strcmp(
            name,
            "ntp_sync"
        ) == 0
    )
    {
        if (
            _clockSystem &&
            _clockSystem->syncFromNTP()
        )
        {
            JsonDocument response;

            response["ok"] = true;
            response["action"] = "ntp_sync";
            response["success"] = true;


            String body;

            serializeJson(
                response,
                body
            );


            sendJson(
                200,
                body
            );
        }
        else
        {
            sendError(
                503,
                "NTP synchronization failed"
            );
        }


        return;
    }


    if (
        doc["value"].isNull()
    )
    {
        sendError(
            400,
            "missing value"
        );

        return;
    }


    SettingsManager::Id id;


    if (
        !paramFromName(
            name,
            id
        )
    )
    {
        sendError(
            400,
            "unknown param"
        );

        return;
    }


    const int oldValue =
        getParamValue(
            *_settings,
            id
        );


    const int value =
        doc["value"].as<int>();


    if (
        !setParamValue(
            *_settings,
            id,
            value
        )
    )
    {
        sendError(
            400,
            "invalid parameter value"
        );

        return;
    }


    const int actualValue =
        getParamValue(
            *_settings,
            id
        );


    JsonDocument response;


    response["ok"] =
        true;

    response["param"] =
        paramName(id);

    response["value"] =
        actualValue;

    response["changed"] =
        actualValue != oldValue;


    String body;

    serializeJson(
        response,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// GET ALL PARAMS
// =============================================================

void WebServerManager::handleGetAllParams()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }


    JsonDocument doc;


    for (
        uint8_t i = 0;

        i <
        static_cast<uint8_t>(
            SettingsManager::Id::COUNT
        );

        ++i
    )
    {
        const SettingsManager::Id id =
            static_cast<SettingsManager::Id>(
                i
            );


        const char* name =
            paramName(id);


        if (
            !name ||
            !name[0]
        )
        {
            continue;
        }


        doc[name] =
            getParamValue(
                *_settings,
                id
            );
    }


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// RESET
// =============================================================

void WebServerManager::handleReset()
{
    if (!_settings)
    {
        sendError(
            500,
            "settings not initialized"
        );

        return;
    }


    _settings->reset();

    sendOk();
}


// =============================================================
// TIME
// =============================================================

void WebServerManager::handleTime()
{
    if (!_clockSystem)
    {
        sendError(
            503,
            "clock not ready"
        );

        return;
    }


    JsonDocument doc;


    doc["year"] =
        _clockSystem->year();

    doc["month"] =
        _clockSystem->month();

    doc["day"] =
        _clockSystem->day();

    doc["hour"] =
        _clockSystem->hour();

    doc["minute"] =
        _clockSystem->minute();

    doc["second"] =
        _clockSystem->second();


    if (_settings)
    {
        doc["utc_offset"] =
            _settings->getUtcOffset();
    }
    else
    {
        doc["utc_offset"] = 0;
    }


    doc["valid"] =
        _clockSystem->isTimeValid();


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// TIME SYNC
// =============================================================

void WebServerManager::handleTimeSync()
{
    if (!_clockSystem)
    {
        sendError(
            503,
            "clock not ready"
        );

        return;
    }


    if (
        !_clockSystem->syncFromNTP()
    )
    {
        sendError(
            503,
            "NTP synchronization failed"
        );

        return;
    }


    JsonDocument doc;


    doc["ok"] = true;

    doc["synchronized"] =
        true;


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// LEGACY GET ALARM
// =============================================================

void WebServerManager::handleGetAlarm()
{
    JsonDocument doc;


    doc["enabled"] =
        _alarm.enabled;


    char timeBuffer[6];


    snprintf(
        timeBuffer,
        sizeof(timeBuffer),
        "%02u:%02u",
        _alarm.hour,
        _alarm.minute
    );


    doc["time"] =
        timeBuffer;


    JsonArray repeat =
        doc["repeat"]
            .to<JsonArray>();


    for (
        uint8_t i = 0;
        i < 7;
        ++i
    )
    {
        if (_alarm.repeat[i])
        {
            repeat.add(i);
        }
    }


    doc["sound"] =
        _alarm.sound;


    doc["volume"] =
        _alarm.volume;


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// LEGACY SET ALARM
// =============================================================

void WebServerManager::handleSetAlarm()
{
    JsonDocument doc;


    if (!parseJson(doc))
        return;


    if (
        !doc["enabled"].isNull()
    )
    {
        _alarm.enabled =
            doc["enabled"].as<bool>();
    }


    const char* time =
        doc["time"];


    if (time)
    {
        int hour = 0;

        int minute = 0;


        if (
            sscanf(
                time,
                "%d:%d",
                &hour,
                &minute
            ) != 2
        )
        {
            sendError(
                400,
                "invalid time"
            );

            return;
        }


        if (
            hour < 0 ||
            hour > 23 ||
            minute < 0 ||
            minute > 59
        )
        {
            sendError(
                400,
                "invalid time"
            );

            return;
        }


        _alarm.hour =
            static_cast<uint8_t>(
                hour
            );


        _alarm.minute =
            static_cast<uint8_t>(
                minute
            );
    }


    if (
        doc["repeat"].is<JsonArray>()
    )
    {
        for (
            uint8_t i = 0;
            i < 7;
            ++i
        )
        {
            _alarm.repeat[i] =
                false;
        }


        JsonArray repeat =
            doc["repeat"]
                .as<JsonArray>();


        for (
            JsonVariant item :
            repeat
        )
        {
            const int day =
                item.as<int>();


            if (
                day >= 0 &&
                day <= 6
            )
            {
                _alarm.repeat[day] =
                    true;
            }
        }
    }


    if (
        !doc["sound"].isNull()
    )
    {
        _alarm.sound =
            static_cast<uint8_t>(
                constrain(
                    doc["sound"].as<int>(),
                    0,
                    255
                )
            );
    }


    if (
        !doc["volume"].isNull()
    )
    {
        _alarm.volume =
            static_cast<uint8_t>(
                constrain(
                    doc["volume"].as<int>(),
                    0,
                    100
                )
            );
    }


    JsonDocument response;


    response["ok"] =
        true;

    response["enabled"] =
        _alarm.enabled;

    response["hour"] =
        _alarm.hour;

    response["minute"] =
        _alarm.minute;


    String body;

    serializeJson(
        response,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// GET ALL ALARMS
// =============================================================

void WebServerManager::handleGetAlarms()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "alarm manager not ready"
        );

        return;
    }


    JsonDocument doc;


    JsonArray alarms =
        doc["alarms"]
            .to<JsonArray>();


    const uint8_t count =
        _alarmManager->count();


    for (
        uint8_t i = 0;
        i < count;
        ++i
    )
    {
        const AlarmScheduleData* schedule =
            _alarmManager->getSchedule(i);


        if (!schedule)
            continue;


        AlarmData alarm;


        if (
            _alarmManager->loadAlarm(
                schedule->id,
                alarm
            )
        )
        {
            JsonObject item =
                alarms.add<JsonObject>();


            JsonDocument alarmDoc;


            serializeAlarm(
                alarmDoc,
                alarm
            );


            item.set(
                alarmDoc.as<JsonObject>()
            );
        }
        else
        {
            JsonObject item =
                alarms.add<JsonObject>();


            serializeAlarmSchedule(
                *reinterpret_cast<JsonDocument*>(
                    &item
                ),
                *schedule
            );
        }
    }


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// CREATE ALARM
// =============================================================

void WebServerManager::handleCreateAlarm()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "alarm manager not ready"
        );

        return;
    }


    JsonDocument doc;


    if (!parseJson(doc))
        return;


    AlarmData alarm;


    if (
        !parseAlarmJson(
            doc,
            alarm
        )
    )
    {
        return;
    }


    if (
        !_alarmManager->create(
            alarm
        )
    )
    {
        sendError(
            400,
            "failed to create alarm"
        );

        return;
    }


    JsonDocument response;


    response["ok"] =
        true;


    JsonObject alarmObject =
        response["alarm"]
            .to<JsonObject>();


    // =========================================================
    // ArduinoJson 7:
    //
    // response["alarm"].to<JsonDocument>()
    // НЕПРАВИЛЬНО.
    //
    // Используем временный документ и копируем объект.
    // =========================================================

    JsonDocument alarmDoc;


    serializeAlarm(
        alarmDoc,
        alarm
    );


    alarmObject.set(
        alarmDoc.as<JsonObject>()
    );


    String body;

    serializeJson(
        response,
        body
    );


    sendJson(
        201,
        body
    );
}


// =============================================================
// GET ALARM BY ID
// =============================================================

void WebServerManager::handleGetAlarmById()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "alarm manager not ready"
        );

        return;
    }


    String id;


    if (
        !getAlarmIdFromUri(
            id
        )
    )
    {
        sendError(
            400,
            "invalid alarm id"
        );

        return;
    }


    AlarmData alarm;


    if (
        !_alarmManager->loadAlarm(
            id.c_str(),
            alarm
        )
    )
    {
        sendError(
            404,
            "alarm not found"
        );

        return;
    }


    JsonDocument doc;


    serializeAlarm(
        doc,
        alarm
    );


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// UPDATE ALARM
// =============================================================

void WebServerManager::handleUpdateAlarmById()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "alarm manager not ready"
        );

        return;
    }


    String id;


    if (
        !getAlarmIdFromUri(
            id
        )
    )
    {
        sendError(
            400,
            "invalid alarm id"
        );

        return;
    }


    JsonDocument doc;


    if (!parseJson(doc))
        return;


    AlarmData alarm;


    if (
        !_alarmManager->loadAlarm(
            id.c_str(),
            alarm
        )
    )
    {
        sendError(
            404,
            "alarm not found"
        );

        return;
    }


    // =========================================================
    // Parse supplied values.
    //
    // The ID comes from the URL.
    // =========================================================

    if (
        !doc["name"].isNull()
    )
    {
        const char* name =
            doc["name"];


        if (name)
        {
            strncpy(
                alarm.name,
                name,
                AlarmLimits::MAX_NAME_LENGTH - 1
            );


            alarm.name[
                AlarmLimits::MAX_NAME_LENGTH - 1
            ] = '\0';
        }
    }


    if (
        !doc["enabled"].isNull()
    )
    {
        alarm.enabled =
            doc["enabled"].as<bool>();
    }


    if (
        !doc["time"].isNull()
    )
    {
        JsonObject time =
            doc["time"]
                .as<JsonObject>();


        if (!time.isNull())
        {
            if (
                !time["hour"].isNull()
            )
            {
                alarm.time.hour =
                    time["hour"].as<uint8_t>();
            }


            if (
                !time["minute"].isNull()
            )
            {
                alarm.time.minute =
                    time["minute"].as<uint8_t>();
            }


            if (
                !time["second"].isNull()
            )
            {
                alarm.time.second =
                    time["second"].as<uint8_t>();
            }
        }
    }


    if (
        !doc["repeatMask"].isNull()
    )
    {
        alarm.repeatMask =
            doc["repeatMask"].as<uint8_t>();
    }


    if (
        !doc["phases"].isNull()
    )
    {
        JsonDocument parsed =
            doc;


        AlarmData replacement;


        if (
            !parseAlarmJson(
                parsed,
                replacement
            )
        )
        {
            return;
        }


        // Preserve ID.
        strncpy(
            replacement.id,
            alarm.id,
            AlarmLimits::MAX_ID_LENGTH - 1
        );


        replacement.id[
            AlarmLimits::MAX_ID_LENGTH - 1
        ] = '\0';


        alarm =
            replacement;
    }


    if (
        !_alarmManager->updateAlarm(
            alarm
        )
    )
    {
        sendError(
            400,
            "failed to update alarm"
        );

        return;
    }


    JsonDocument response;


    response["ok"] =
        true;


    JsonObject result =
        response["alarm"]
            .to<JsonObject>();


    JsonDocument alarmDoc;


    serializeAlarm(
        alarmDoc,
        alarm
    );


    result.set(
        alarmDoc.as<JsonObject>()
    );


    String body;

    serializeJson(
        response,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// DELETE ALARM
// =============================================================

void WebServerManager::handleDeleteAlarmById()
{
    if (!_alarmManager)
    {
        sendError(
            503,
            "alarm manager not ready"
        );

        return;
    }


    String id;


    if (
        !getAlarmIdFromUri(
            id
        )
    )
    {
        sendError(
            400,
            "invalid alarm id"
        );

        return;
    }


    if (
        !_alarmManager->remove(
            id.c_str()
        )
    )
    {
        sendError(
            404,
            "alarm not found"
        );

        return;
    }


    JsonDocument doc;


    doc["ok"] =
        true;

    doc["deleted"] =
        id;


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// PARSE ALARM JSON
// =============================================================

bool WebServerManager::parseAlarmJson(
    JsonDocument& doc,
    AlarmData& alarm
)
{
    // =========================================================
    // ID
    // =========================================================

    const char* id =
        doc["id"];


    if (!id || !id[0])
    {
        sendError(
            400,
            "missing alarm id"
        );

        return false;
    }


    if (
        strlen(id) >=
        AlarmLimits::MAX_ID_LENGTH
    )
    {
        sendError(
            400,
            "alarm id too long"
        );

        return false;
    }


    memset(
        &alarm,
        0,
        sizeof(AlarmData)
    );


    strncpy(
        alarm.id,
        id,
        AlarmLimits::MAX_ID_LENGTH - 1
    );


    alarm.id[
        AlarmLimits::MAX_ID_LENGTH - 1
    ] = '\0';


    // =========================================================
    // NAME
    // =========================================================

    const char* name =
        doc["name"] |
        "";


    strncpy(
        alarm.name,
        name,
        AlarmLimits::MAX_NAME_LENGTH - 1
    );


    alarm.name[
        AlarmLimits::MAX_NAME_LENGTH - 1
    ] = '\0';


    // =========================================================
    // ENABLED
    // =========================================================

    alarm.enabled =
        doc["enabled"] |
        false;


    // =========================================================
    // TIME
    // =========================================================

    JsonObject time =
        doc["time"]
            .as<JsonObject>();


    if (
        time.isNull()
    )
    {
        sendError(
            400,
            "missing alarm time"
        );

        return false;
    }


    const int hour =
        time["hour"] |
        0;


    const int minute =
        time["minute"] |
        0;


    const int second =
        time["second"] |
        0;


    if (
        hour < 0 ||
        hour > 23 ||
        minute < 0 ||
        minute > 59 ||
        second < 0 ||
        second > 59
    )
    {
        sendError(
            400,
            "invalid alarm time"
        );

        return false;
    }


    alarm.time.hour =
        static_cast<uint8_t>(
            hour
        );


    alarm.time.minute =
        static_cast<uint8_t>(
            minute
        );


    alarm.time.second =
        static_cast<uint8_t>(
            second
        );


    // =========================================================
    // REPEAT MASK
    // =========================================================

    alarm.repeatMask =
        doc["repeatMask"] |
        0;


    // =========================================================
    // PHASES
    // =========================================================

    JsonArray phases =
        doc["phases"]
            .as<JsonArray>();


    if (
        phases.isNull()
    )
    {
        alarm.phaseCount =
            0;

        return true;
    }


    if (
        phases.size() >
        AlarmLimits::MAX_PHASES
    )
    {
        sendError(
            400,
            "too many alarm phases"
        );

        return false;
    }


    alarm.phaseCount =
        static_cast<uint8_t>(
            phases.size()
        );


    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        JsonObject phaseJson =
            phases[i]
                .as<JsonObject>();


        AlarmPhase& phase =
            alarm.phases[i];


        // =====================================================
        // GENERAL
        // =====================================================

        phase.startOffsetMs =
            phaseJson["startOffsetMs"] |
            static_cast<int64_t>(0);


        phase.durationMs =
            phaseJson["phaseDurationMs"] |
            static_cast<uint32_t>(0);


        const char* condition =
            phaseJson["condition"] |
            "always";


        strncpy(
            phase.condition,
            condition,
            AlarmLimits::MAX_CONDITION_LENGTH - 1
        );


        phase.condition[
            AlarmLimits::MAX_CONDITION_LENGTH - 1
        ] = '\0';


        // =====================================================
        // MATRIX
        // =====================================================

        JsonObject matrix =
            phaseJson["matrix"]
                .as<JsonObject>();


        if (
            !matrix.isNull()
        )
        {
            phase.matrix.enabled =
                matrix["enabled"] |
                false;


            const char* effectId =
                matrix["effectId"] |
                "";


            strncpy(
                phase.matrix.effectId,
                effectId,
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            );


            phase.matrix.effectId[
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            ] = '\0';


            phase.matrix.start =
                matrix["from"] |
                0;


            phase.matrix.end =
                matrix["to"] |
                0;


            phase.matrix.speedMs =
                matrix["periodMs"] |
                0;


            phase.matrix.durationMs =
                matrix["transitionMs"] |
                0;


            phase.matrix.loop =
                matrix["loop"] |
                false;


            phase.matrix.maxDurationMs =
                matrix["maxDurationMs"] |
                0;
        }


        // =====================================================
        // AUDIO
        // =====================================================

        JsonObject audio =
            phaseJson["audio"]
                .as<JsonObject>();


        if (
            !audio.isNull()
        )
        {
            phase.audio.enabled =
                audio["enabled"] |
                false;


            const char* effectId =
                audio["effectId"] |
                "";


            strncpy(
                phase.audio.effectId,
                effectId,
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            );


            phase.audio.effectId[
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            ] = '\0';


            phase.audio.start =
                audio["from"] |
                0;


            phase.audio.end =
                audio["to"] |
                0;


            phase.audio.speedMs =
                audio["periodMs"] |
                0;


            phase.audio.durationMs =
                audio["transitionMs"] |
                0;


            phase.audio.loop =
                audio["loop"] |
                false;


            phase.audio.maxDurationMs =
                audio["maxDurationMs"] |
                0;
        }


        // =====================================================
        // COB
        // =====================================================

        JsonObject cob =
            phaseJson["cob"]
                .as<JsonObject>();


        if (
            !cob.isNull()
        )
        {
            phase.cob.enabled =
                cob["enabled"] |
                false;


            const char* effectId =
                cob["effectId"] |
                "";


            strncpy(
                phase.cob.effectId,
                effectId,
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            );


            phase.cob.effectId[
                AlarmLimits::MAX_EFFECT_ID_LENGTH - 1
            ] = '\0';


            phase.cob.start =
                cob["from"] |
                0;


            phase.cob.end =
                cob["to"] |
                0;


            phase.cob.speedMs =
                cob["periodMs"] |
                0;


            phase.cob.durationMs =
                cob["transitionMs"] |
                0;


            phase.cob.loop =
                cob["loop"] |
                false;


            phase.cob.maxDurationMs =
                cob["maxDurationMs"] |
                0;
        }
    }


    return true;
}


// =============================================================
// SERIALIZE ALARM
// =============================================================

void WebServerManager::serializeAlarm(
    JsonDocument& doc,
    const AlarmData& alarm
)
{
    doc["schemaVersion"] =
        1;


    doc["id"] =
        alarm.id;


    doc["name"] =
        alarm.name;


    doc["enabled"] =
        alarm.enabled;


    // =========================================================
    // TIME
    // =========================================================

    JsonObject time =
        doc["time"]
            .to<JsonObject>();


    time["hour"] =
        alarm.time.hour;


    time["minute"] =
        alarm.time.minute;


    time["second"] =
        alarm.time.second;


    // =========================================================
    // REPEAT
    // =========================================================

    doc["repeatMask"] =
        alarm.repeatMask;


    // =========================================================
    // PHASES
    // =========================================================

    JsonArray phases =
        doc["phases"]
            .to<JsonArray>();


    for (
        uint8_t i = 0;
        i < alarm.phaseCount;
        ++i
    )
    {
        const AlarmPhase& phase =
            alarm.phases[i];


        JsonObject phaseJson =
            phases.add<JsonObject>();


        phaseJson["startOffsetMs"] =
            phase.startOffsetMs;


        phaseJson["phaseDurationMs"] =
            phase.durationMs;


        phaseJson["condition"] =
            phase.condition;


        // =====================================================
        // MATRIX
        // =====================================================

        JsonObject matrix =
            phaseJson["matrix"]
                .to<JsonObject>();


        matrix["enabled"] =
            phase.matrix.enabled;


        matrix["effectId"] =
            phase.matrix.effectId;


        matrix["from"] =
            phase.matrix.start;


        matrix["to"] =
            phase.matrix.end;


        matrix["periodMs"] =
            phase.matrix.speedMs;


        matrix["transitionMs"] =
            phase.matrix.durationMs;


        matrix["loop"] =
            phase.matrix.loop;


        if (
            phase.matrix.maxDurationMs > 0
        )
        {
            matrix["maxDurationMs"] =
                phase.matrix.maxDurationMs;
        }


        // =====================================================
        // AUDIO
        // =====================================================

        JsonObject audio =
            phaseJson["audio"]
                .to<JsonObject>();


        audio["enabled"] =
            phase.audio.enabled;


        audio["effectId"] =
            phase.audio.effectId;


        audio["from"] =
            phase.audio.start;


        audio["to"] =
            phase.audio.end;


        audio["periodMs"] =
            phase.audio.speedMs;


        audio["transitionMs"] =
            phase.audio.durationMs;


        audio["loop"] =
            phase.audio.loop;


        if (
            phase.audio.maxDurationMs > 0
        )
        {
            audio["maxDurationMs"] =
                phase.audio.maxDurationMs;
        }


        // =====================================================
        // COB
        // =====================================================

        JsonObject cob =
            phaseJson["cob"]
                .to<JsonObject>();


        cob["enabled"] =
            phase.cob.enabled;


        cob["effectId"] =
            phase.cob.effectId;


        cob["from"] =
            phase.cob.start;


        cob["to"] =
            phase.cob.end;


        cob["periodMs"] =
            phase.cob.speedMs;


        cob["transitionMs"] =
            phase.cob.durationMs;


        cob["loop"] =
            phase.cob.loop;


        if (
            phase.cob.maxDurationMs > 0
        )
        {
            cob["maxDurationMs"] =
                phase.cob.maxDurationMs;
        }
    }
}


// =============================================================
// SERIALIZE SCHEDULE
// =============================================================

void WebServerManager::serializeAlarmSchedule(
    JsonDocument& doc,
    const AlarmScheduleData& schedule
)
{
    doc["id"] =
        schedule.id;


    doc["enabled"] =
        schedule.enabled;


    JsonObject time =
        doc["time"]
            .to<JsonObject>();


    time["hour"] =
        schedule.hour;


    time["minute"] =
        schedule.minute;


    time["second"] =
        schedule.second;


    doc["repeatMask"] =
        schedule.repeatMask;
}


// =============================================================
// ALARM ID FROM URI
// =============================================================

bool WebServerManager::getAlarmIdFromUri(
    String& id
)
{
    const String prefix =
        "/api/alarms/";


    const String uri =
        _server.uri();


    if (
        !uri.startsWith(
            prefix
        )
    )
    {
        return false;
    }


    id =
        uri.substring(
            prefix.length()
        );


    while (
        id.endsWith("/")
    )
    {
        id.remove(
            id.length() - 1
        );
    }


    if (
        id.isEmpty()
    )
    {
        return false;
    }


    if (
        id.indexOf("/") >= 0
    )
    {
        return false;
    }


    return true;
}


// =============================================================
// TIMER GET
// =============================================================

void WebServerManager::handleGetTimer()
{
    updateTimer();


    JsonDocument doc;


    switch (
        _timer.status
    )
    {
        case TimerStatus::READY:
            doc["status"] = "ready";
            break;

        case TimerStatus::RUNNING:
            doc["status"] = "running";
            break;

        case TimerStatus::PAUSED:
            doc["status"] = "paused";
            break;

        case TimerStatus::FINISHED:
            doc["status"] = "finished";
            break;
    }


    doc["remaining"] =
        _timer.remainingSeconds;


    doc["duration"] =
        _timer.durationSeconds;


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// TIMER SET
// =============================================================

void WebServerManager::handleSetTimer()
{
    JsonDocument doc;


    if (!parseJson(doc))
        return;


    const char* action =
        doc["action"];


    if (!action)
    {
        sendError(
            400,
            "missing action"
        );

        return;
    }


    // =========================================================
    // START
    // =========================================================

    if (
        strcmp(
            action,
            "start"
        ) == 0
    )
    {
        const uint32_t hours =
            doc["hours"] | 0;


        const uint32_t minutes =
            doc["minutes"] | 0;


        const uint32_t seconds =
            doc["seconds"] | 0;


        if (
            minutes > 59 ||
            seconds > 59
        )
        {
            sendError(
                400,
                "invalid timer value"
            );

            return;
        }


        _timer.durationSeconds =
            hours * 3600UL +
            minutes * 60UL +
            seconds;


        if (
            _timer.durationSeconds == 0
        )
        {
            sendError(
                400,
                "timer duration is zero"
            );

            return;
        }


        _timer.remainingSeconds =
            _timer.durationSeconds;


        _timer.lastUpdate =
            millis();


        _timer.status =
            TimerStatus::RUNNING;


        sendOk();

        return;
    }


    // =========================================================
    // PAUSE
    // =========================================================

    if (
        strcmp(
            action,
            "pause"
        ) == 0
    )
    {
        updateTimer();


        if (
            _timer.status !=
            TimerStatus::RUNNING
        )
        {
            sendError(
                409,
                "timer is not running"
            );

            return;
        }


        _timer.status =
            TimerStatus::PAUSED;


        sendOk();

        return;
    }


    // =========================================================
    // RESUME
    // =========================================================

    if (
        strcmp(
            action,
            "resume"
        ) == 0
    )
    {
        if (
            _timer.remainingSeconds == 0
        )
        {
            sendError(
                409,
                "timer is finished"
            );

            return;
        }


        _timer.lastUpdate =
            millis();


        _timer.status =
            TimerStatus::RUNNING;


        sendOk();

        return;
    }


    // =========================================================
    // STOP
    // =========================================================

    if (
        strcmp(
            action,
            "stop"
        ) == 0
    )
    {
        _timer.status =
            TimerStatus::READY;


        _timer.durationSeconds =
            0;


        _timer.remainingSeconds =
            0;


        _timer.lastUpdate =
            millis();


        sendOk();

        return;
    }


    sendError(
        400,
        "unknown timer action"
    );
}


// =============================================================
// TIMER UPDATE
// =============================================================

void WebServerManager::updateTimer()
{
    if (
        _timer.status !=
        TimerStatus::RUNNING
    )
    {
        return;
    }


    const uint32_t now =
        millis();


    const uint32_t elapsed =
        now - _timer.lastUpdate;


    if (
        elapsed < 1000
    )
    {
        return;
    }


    const uint32_t seconds =
        elapsed / 1000;


    _timer.lastUpdate +=
        seconds * 1000;


    if (
        seconds >=
        _timer.remainingSeconds
    )
    {
        _timer.remainingSeconds =
            0;


        _timer.status =
            TimerStatus::FINISHED;


        return;
    }


    _timer.remainingSeconds -=
        seconds;
}


// =============================================================
// AUDIO PLAY
// =============================================================

void WebServerManager::handleAudioPlay()
{
    if (!_sound)
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }


    JsonDocument doc;


    if (!parseJson(doc))
        return;


    const char* path =
        doc["path"];


    if (
        !path ||
        !path[0]
    )
    {
        sendError(
            400,
            "missing path"
        );

        return;
    }


    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }


    if (
        !_sd->card().exists(
            path
        )
    )
    {
        sendError(
            404,
            "file not found"
        );

        return;
    }


    SoundManager::PlayOptions options;


    const char* stream =
        doc["stream"] |
        "media";


    if (
        strcmp(
            stream,
            "alarm"
        ) == 0
    )
    {
        options.stream =
            SoundManager::AudioStream::Alarm;
    }
    else if (
        strcmp(
            stream,
            "system"
        ) == 0
    )
    {
        options.stream =
            SoundManager::AudioStream::System;
    }
    else
    {
        options.stream =
            SoundManager::AudioStream::Media;
    }


    int volume =
        doc["volume"] |
        100;


    volume =
        constrain(
            volume,
            0,
            100
        );


    options.localPercent =
        static_cast<uint8_t>(
            volume
        );


    options.fadeInMs =
        doc["fade_in"] |
        0;


    options.fadeOutMs =
        doc["fade_out"] |
        0;


    const char* curve =
        doc["curve"] |
        "linear";


    if (
        strcmp(
            curve,
            "exp"
        ) == 0
    )
    {
        options.curve =
            SoundManager::FadeCurve::Exponential;
    }
    else if (
        strcmp(
            curve,
            "log"
        ) == 0
    )
    {
        options.curve =
            SoundManager::FadeCurve::Logarithmic;
    }
    else
    {
        options.curve =
            SoundManager::FadeCurve::Linear;
    }


    if (
        !_sound->play(
            path,
            options
        )
    )
    {
        sendError(
            500,
            "play failed"
        );

        return;
    }


    sendOk();
}


// =============================================================
// AUDIO PAUSE
// =============================================================

void WebServerManager::handleAudioPause()
{
    if (!_sound)
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }


    if (
        !_sound->pause()
    )
    {
        sendError(
            500,
            "pause failed"
        );

        return;
    }


    sendOk();
}


// =============================================================
// AUDIO RESUME
// =============================================================

void WebServerManager::handleAudioResume()
{
    if (!_sound)
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }


    if (
        !_sound->resume()
    )
    {
        sendError(
            500,
            "resume failed"
        );

        return;
    }


    sendOk();
}


// =============================================================
// AUDIO STOP
// =============================================================

void WebServerManager::handleAudioStop()
{
    if (!_sound)
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }


    uint32_t fadeOut =
        0;


    if (
        _server.hasArg(
            "plain"
        )
    )
    {
        JsonDocument doc;


        DeserializationError error =
            deserializeJson(
                doc,
                _server.arg(
                    "plain"
                )
            );


        if (!error)
        {
            fadeOut =
                doc["fade_out"] |
                0;
        }
    }


    if (fadeOut > 0)
    {
        _sound->stop(
            fadeOut
        );
    }
    else
    {
        _sound->stop();
    }


    sendOk();
}


// =============================================================
// AUDIO STATUS
// =============================================================

void WebServerManager::handleAudioStatus()
{
    if (!_sound)
    {
        sendError(
            503,
            "sound not ready"
        );

        return;
    }


    JsonDocument doc;


    doc["state"] =
        _sound->getStateString();


    doc["path"] =
        _sound->getCurrentPath();


    doc["stream"] =
        static_cast<uint8_t>(
            _sound->getCurrentStream()
        );


    doc["position"] =
        _sound->getPositionMs();


    doc["duration"] =
        _sound->getDurationMs();


    doc["volume"] =
        _sound->getEffectiveVolume();


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// SD
// =============================================================

void WebServerManager::handleSD()
{
    if (
        !_sd ||
        !_sd->isReady()
    )
    {
        sendError(
            503,
            "SD not available"
        );

        return;
    }


    static constexpr size_t MAX_FILES =
        300;


    static constexpr uint8_t MAX_DEPTH =
        3;


    static SDFileEntry entries[
        MAX_FILES
    ];


    const size_t count =
        _sd->listFiles(
            entries,
            MAX_FILES,
            MAX_DEPTH,
            "/"
        );


    JsonDocument doc;


    JsonArray files =
        doc["files"]
            .to<JsonArray>();


    for (
        size_t i = 0;
        i < count;
        ++i
    )
    {
        JsonObject item =
            files.add<JsonObject>();


        item["path"] =
            entries[i].path;


        item["size"] =
            entries[i].size;


        item["type"] =
            entries[i].isDir
                ? "dir"
                : "file";
    }


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// SENSORS
// =============================================================

void WebServerManager::handleSensors()
{
    JsonDocument doc;


    doc["temperature"] =
        0;


    doc["humidity"] =
        0;


    doc["light"] =
        0;


    doc["distance"] =
        0;


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        200,
        body
    );
}


// =============================================================
// PARSE JSON
// =============================================================

bool WebServerManager::parseJson(
    JsonDocument& doc
)
{
    if (
        !_server.hasArg(
            "plain"
        )
    )
    {
        sendError(
            400,
            "body missing"
        );

        return false;
    }


    DeserializationError error =
        deserializeJson(
            doc,
            _server.arg(
                "plain"
            )
        );


    if (error)
    {
        sendError(
            400,
            "invalid JSON"
        );

        return false;
    }


    return true;
}


// =============================================================
// SEND JSON
// =============================================================

void WebServerManager::sendJson(
    int code,
    const String& body
)
{
    _server.send(
        code,
        "application/json",
        body
    );
}


// =============================================================
// SEND OK
// =============================================================

void WebServerManager::sendOk()
{
    sendJson(
        200,
        "{\"ok\":true}"
    );
}


// =============================================================
// SEND ERROR
// =============================================================

void WebServerManager::sendError(
    int code,
    const char* message
)
{
    JsonDocument doc;


    doc["ok"] =
        false;


    doc["error"] =
        message
            ? message
            : "unknown error";


    String body;

    serializeJson(
        doc,
        body
    );


    sendJson(
        code,
        body
    );
}


// =============================================================
// UPDATE
// =============================================================

void WebServerManager::update()
{
    if (!_initialized)
        return;


    _server.handleClient();


    // Timer принадлежит WebServerManager.
    updateTimer();


    // AlarmManager здесь НЕ обновляем.
    //
    // App::update() уже делает:
    //
    //     _alarmManager.update();
    //
    // Это предотвращает двойной вызов.
}


// =============================================================
// RUNNING
// =============================================================

bool WebServerManager::isRunning() const
{
    return _initialized;
}


// =============================================================
// CONNECTED
// =============================================================

bool WebServerManager::isConnected() const
{
    return
        WiFi.status() ==
        WL_CONNECTED;
}


// =============================================================
// IP
// =============================================================

String WebServerManager::getIP() const
{
    if (
        WiFi.status() !=
        WL_CONNECTED
    )
    {
        return "0.0.0.0";
    }


    return WiFi.localIP()
        .toString();
}
