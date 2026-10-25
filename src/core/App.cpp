#include "App.h"

#include <Arduino.h>

// ============================================================
// AUDIO SERVER CONFIGURATION
// ============================================================

// IP-адрес компьютера с Python WebSocket-сервером.
// ЗАМЕНИ на реальный адрес своего компьютера.
static constexpr char AUDIO_SERVER_HOST[] = "192.168.1.100";

// Порт WebSocket-сервера.
static constexpr uint16_t AUDIO_SERVER_PORT = 8765;

// WebSocket endpoint.
static constexpr char AUDIO_SERVER_PATH[] = "/audio";

// Идентификатор устройства.
static constexpr char AUDIO_DEVICE_ID[] = "smartclock-01";


// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _settings()
    , _spiManager()
    , _i2sManager()
    , _soundManager(
        _settings,
        _i2sManager
    )
    , _sensorManager()
    , _clockSystem(
        _settings
    )
    , _lighting(
        _settings
    )
    , _displaySystem(
        _settings,
        _clockSystem,
        _sensorManager,
        _spiManager
    )
    , _brightnessManager()
    , _alarmManager(
        _clockSystem
    )
    , _alarmController(
        _alarmManager
    )
    , _wifiManager()
    , _webServer(80)
    , _webWiFiManager(
        _wifiManager,
        _settings,
        _clockSystem
    )
    // , _audioWebSocket()
    , _initialized(false)
{
}


// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    if (_initialized)
    {
        Serial0.println(
            "[APP] Already initialized"
        );

        return true;
    }

    Serial0.println();
    Serial0.println(
        "============================================"
    );
    Serial0.println(
        "          SMART CLOCK STARTUP"
    );
    Serial0.println(
        "============================================"
    );
    Serial0.println();


    // --------------------------------------------------------
    // STEP 1: SETTINGS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 1/13: Settings"
    );

    if (!initSettings())
    {
        Serial0.println(
            "[APP][ERROR] Settings initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Settings initialized"
    );


    // --------------------------------------------------------
    // STEP 2: SPI
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 2/13: SPI"
    );

    if (!initSPI())
    {
        Serial0.println(
            "[APP][ERROR] SPI initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] SPI initialized"
    );


    // --------------------------------------------------------
    // STEP 3: I2S
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 3/13: I2S"
    );

    if (!initI2S())
    {
        Serial0.println(
            "[APP][ERROR] I2S initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] I2S initialized"
    );


    // --------------------------------------------------------
    // STEP 4: CLOCK
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 4/13: Clock"
    );

    if (!initClock())
    {
        Serial0.println(
            "[APP][ERROR] Clock initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Clock initialized"
    );


    // --------------------------------------------------------
    // STEP 5: SENSORS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 5/13: Sensors"
    );

    if (!initSensors())
    {
        Serial0.println(
            "[APP][ERROR] Sensor initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Sensors initialized"
    );


    // --------------------------------------------------------
    // STEP 6: LIGHTING
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 6/13: Lighting"
    );

    if (!initLighting())
    {
        Serial0.println(
            "[APP][ERROR] Lighting initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Lighting initialized"
    );


    // --------------------------------------------------------
    // STEP 7: DISPLAYS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 7/13: Displays"
    );

    if (!initDisplay())
    {
        Serial0.println(
            "[APP][ERROR] Display initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Displays initialized"
    );


    // --------------------------------------------------------
    // STEP 8: BRIGHTNESS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 8/13: Brightness"
    );

    if (!initBrightness())
    {
        Serial0.println(
            "[APP][ERROR] Brightness initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Brightness initialized"
    );


    // --------------------------------------------------------
    // STEP 9: SOUND
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 9/13: Sound"
    );

    if (!initSound())
    {
        Serial0.println(
            "[APP][ERROR] Sound initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Sound initialized"
    );


    // --------------------------------------------------------
    // STEP 10: ALARMS
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 10/13: Alarms"
    );

    if (!initAlarm())
    {
        Serial0.println(
            "[APP][ERROR] Alarm initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Alarms initialized"
    );


    // --------------------------------------------------------
    // STEP 11: WI-FI
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 11/13: Wi-Fi"
    );

    if (!initWiFi())
    {
        Serial0.println(
            "[APP][ERROR] Wi-Fi initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Wi-Fi initialized"
    );


    // --------------------------------------------------------
    // STEP 12: WEB INTERFACE
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 12/13: Web interface"
    );

    if (!initWebWiFi())
    {
        Serial0.println(
            "[APP][ERROR] Web interface initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] Web interface initialized"
    );


    // --------------------------------------------------------
    // STEP 13: AUDIO WEBSOCKET
    // --------------------------------------------------------

    Serial0.println(
        "[APP] Step 13/13: Audio WebSocket"
    );

    // if (!initAudioWebSocket())
    // {
    //     Serial0.println(
    //         "[APP][ERROR] Audio WebSocket initialization failed"
    //     );

    //     return false;
    // }

    Serial0.println(
        "[APP] Audio WebSocket initialized"
    );


    // --------------------------------------------------------
    // INITIALIZATION COMPLETE
    // --------------------------------------------------------

    _initialized = true;

    Serial0.println();
    Serial0.println(
        "============================================"
    );
    Serial0.println(
        "[APP] Initialization complete"
    );
    Serial0.println(
        "[APP] SMART CLOCK READY"
    );
    Serial0.println(
        "============================================"
    );
    Serial0.println();

    return true;
}


// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_initialized)
    {
        return;
    }


    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    _settings.update();


    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    _clockSystem.update();


    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    _sensorManager.update();


    // --------------------------------------------------------
    // BRIGHTNESS
    // --------------------------------------------------------

    _brightnessManager.update();


    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    _lighting.update();


    // --------------------------------------------------------
    // DISPLAYS
    // --------------------------------------------------------

    _displaySystem.update();


    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    _soundManager.update();


    // --------------------------------------------------------
    // ALARM MANAGER
    // --------------------------------------------------------

    _alarmManager.update();


    // --------------------------------------------------------
    // ALARM CONTROLLER
    // --------------------------------------------------------

    _alarmController.update();


    // --------------------------------------------------------
    // WI-FI
    // --------------------------------------------------------

    _wifiManager.update();


    // --------------------------------------------------------
    // HTTP SERVER
    // --------------------------------------------------------

    _webServer.handleClient();


    // --------------------------------------------------------
    // AUDIO WEBSOCKET
    // --------------------------------------------------------
    //
    // Обслуживаем соединение и получаем новые сообщения.
    // Метод должен вызываться регулярно.
    //
    // Автоматическое повторное подключение выполняет
    // библиотека WebSocketsClient.
    // --------------------------------------------------------

    // _audioWebSocket.update();
}


// ============================================================
// SETTINGS
// ============================================================

bool App::initSettings()
{
    Serial0.println(
        "[APP] Initializing settings..."
    );

    return _settings.begin();
}


// ============================================================
// SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println(
        "[APP] Initializing SPI..."
    );

    return _spiManager.begin();
}


// ============================================================
// I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println(
        "[APP] Initializing I2S..."
    );

    return _i2sManager.begin();
}


// ============================================================
// CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println(
        "[APP] Initializing clock..."
    );

    return _clockSystem.begin();
}


// ============================================================
// SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println(
        "[APP] Initializing sensors..."
    );

    return _sensorManager.begin();
}


// ============================================================
// LIGHTING
// ============================================================

bool App::initLighting()
{
    Serial0.println(
        "[APP] Initializing lighting..."
    );

    return _lighting.begin();
}


// ============================================================
// DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println(
        "[APP] Initializing displays..."
    );

    return _displaySystem.begin();
}


// ============================================================
// BRIGHTNESS
// ============================================================

bool App::initBrightness()
{
    Serial0.println(
        "[APP] Initializing BrightnessManager..."
    );

    _brightnessManager.begin(
        &_settings,
        &_sensorManager,
        &_clockSystem
    );

    if (!_brightnessManager.isInitialized())
    {
        Serial0.println(
            "[APP][ERROR] BrightnessManager initialization failed"
        );

        return false;
    }

    Serial0.println(
        "[APP] BrightnessManager initialized"
    );

    return true;
}


// ============================================================
// SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println(
        "[APP] Initializing sound..."
    );

    return _soundManager.begin();
}


// ============================================================
// ALARMS
// ============================================================

bool App::initAlarm()
{
    Serial0.println(
        "[APP] Initializing alarms..."
    );

    return _alarmManager.begin();
}


// ============================================================
// WI-FI
// ============================================================

bool App::initWiFi()
{
    Serial0.println(
        "[APP] Initializing Wi-Fi..."
    );

    return _wifiManager.begin();
}


// ============================================================
// WEB / WI-FI CONFIGURATION
// ============================================================

bool App::initWebWiFi()
{
    Serial0.println(
        "[APP] Configuring web interface..."
    );

    // --------------------------------------------------------
    // REGISTER WI-FI ROUTES
    // --------------------------------------------------------

    _webWiFiManager.setupRoutes(
        _webServer
    );


    // --------------------------------------------------------
    // START HTTP SERVER
    // --------------------------------------------------------

    _webServer.begin();

    Serial0.println(
        "[APP] HTTP server started"
    );

    Serial0.println(
        "[APP] HTTP port: 80"
    );

    return true;
}


// ============================================================
// AUDIO WEBSOCKET INITIALIZATION
// ============================================================

// bool App::initAudioWebSocket()
// {
//     Serial0.println(
//         "[APP] Initializing audio WebSocket..."
//     );

//     Serial0.printf(
//         "[APP] Audio server: %s:%u%s\n",
//         AUDIO_SERVER_HOST,
//         AUDIO_SERVER_PORT,
//         AUDIO_SERVER_PATH
//     );

//     // --------------------------------------------------------
//     // REGISTER EVENT CALLBACK
//     // --------------------------------------------------------

//     _audioWebSocket.setEventCallback(
//         &App::handleAudioWebSocketEvent
//     );


//     // --------------------------------------------------------
//     // START WEBSOCKET CLIENT
//     // --------------------------------------------------------

//     const bool result = _audioWebSocket.begin(
//         AUDIO_SERVER_HOST,
//         AUDIO_SERVER_PORT,
//         AUDIO_SERVER_PATH,
//         AUDIO_DEVICE_ID
//     );

//     if (!result)
//     {
//         Serial0.println(
//             "[APP][ERROR] Failed to initialize audio WebSocket"
//         );

//         return false;
//     }

//     // begin() инициализирует клиента.
//     // Само TCP/WebSocket-соединение устанавливается
//     // асинхронно во время вызовов update().

//     Serial0.println(
//         "[APP] Audio WebSocket client started"
//     );

//     return true;
// }


// ============================================================
// AUDIO WEBSOCKET EVENT HANDLER
// ============================================================

// void App::handleAudioWebSocketEvent(
//     WebSocketAudioClient::Event event,
//     const uint8_t* payload,
//     size_t length
// )
// {
//     switch (event)
//     {
//         // ----------------------------------------------------
//         // CONNECTED
//         // ----------------------------------------------------

//         case WebSocketAudioClient::Event::CONNECTED:
//         {
//             Serial0.println(
//                 "[APP][AUDIO] Connected to Python server"
//             );

//             break;
//         }


//         // ----------------------------------------------------
//         // DISCONNECTED
//         // ----------------------------------------------------

//         case WebSocketAudioClient::Event::DISCONNECTED:
//         {
//             Serial0.println(
//                 "[APP][AUDIO] Disconnected from Python server"
//             );

//             break;
//         }


//         // ----------------------------------------------------
//         // TEXT RECEIVED
//         // ----------------------------------------------------

//         case WebSocketAudioClient::Event::TEXT_RECEIVED:
//         {
//             Serial0.printf(
//                 "[APP][AUDIO] Server text message (%u bytes)\n",
//                 static_cast<unsigned int>(length)
//             );

//             if (payload != nullptr && length > 0)
//             {
//                 Serial0.printf(
//                     "[APP][AUDIO] Message: %.*s\n",
//                     static_cast<int>(length),
//                     reinterpret_cast<const char*>(payload)
//                 );
//             }

//             // Здесь можно обрабатывать JSON-команды сервера:
//             // start_recording, stop_recording, play_audio и т.д.
//             //
//             // Конкретный формат команд должен совпадать
//             // с реализацией Python-сервера.

//             break;
//         }


//         // ----------------------------------------------------
//         // PCM RECEIVED
//         // ----------------------------------------------------

//         case WebSocketAudioClient::Event::PCM_RECEIVED:
//         {
//             // Сервер прислал бинарный аудиофрагмент.
//             //
//             // payload:
//             //     PCM-данные.
//             //
//             // length:
//             //     размер полученного фрагмента в байтах.
//             //
//             // ВАЖНО:
//             // Здесь данные пока не воспроизводятся.
//             // Для воспроизведения нужно передавать их
//             // в очередь PCM-проигрывателя, который затем
//             // пишет данные в I2S.
//             //
//             // Не выполняем блокирующее воспроизведение
//             // непосредственно внутри callback.

//             if (payload == nullptr || length == 0)
//             {
//                 break;
//             }

//             // Диагностическое сообщение на каждый 50-й
//             // принятый PCM-фрагмент, чтобы не засорять Serial.

//             static uint32_t receivedChunks = 0;

//             ++receivedChunks;

//             if ((receivedChunks % 50) == 0)
//             {
//                 Serial0.printf(
//                     "[APP][AUDIO] Received %lu PCM chunks; last=%u bytes\n",
//                     static_cast<unsigned long>(receivedChunks),
//                     static_cast<unsigned int>(length)
//                 );
//             }

//             break;
//         }


//         // ----------------------------------------------------
//         // ERROR
//         // ----------------------------------------------------

//         case WebSocketAudioClient::Event::ERROR:
//         {
//             Serial0.println(
//                 "[APP][AUDIO][ERROR] WebSocket error"
//             );

//             break;
//         }
//     }
// }