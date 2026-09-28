#include "App.h"

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// WIFI CREDENTIALS
// ============================================================

static constexpr const char* WIFI_SSID     = "tpl47";
static constexpr const char* WIFI_PASSWORD = "12713714";

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _ready(false),

      // ========================================================
      // SETTINGS (первым — от него зависят остальные)
      // ========================================================

      _settings(),

      // ========================================================
      // MANAGERS
      // ========================================================

      _spiManager(),
      _i2sManager(),
      _sdManager(),

      _soundManager(
          _sdManager,
          _settings,
          _i2sManager
      ),

      _sensorManager(),

      // ========================================================
      // CORE SYSTEMS
      // ========================================================

      _clockSystem(
          _settings
      ),

      _displaySystem(
          _settings,
          _clockSystem,
          _sensorManager
      ),

      // ========================================================
      // WEB SERVER
      // ========================================================

      _webServer()
{
}

// ============================================================
// BEGIN
// ============================================================

bool App::begin()
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("        SMART CLOCK STARTING");
    Serial0.println("========================================");

    _ready = false;

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    if (!initSettings())
    {
        Serial0.println("[APP] Settings initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // SPI
    // --------------------------------------------------------

    if (!initSPI())
    {
        Serial0.println("[APP] SPI initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // I2S
    // --------------------------------------------------------

    if (!initI2S())
    {
        Serial0.println("[APP] I2S initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    if (!initSD())
    {
        Serial0.println("[APP] SD initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // SOUND
    // --------------------------------------------------------

    if (!initSound())
    {
        Serial0.println("[APP] Sound initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    if (!initClock())
    {
        Serial0.println("[APP] Clock initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    if (!initSensors())
    {
        Serial0.println("[APP] Sensors initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // DISPLAY
    // --------------------------------------------------------

    if (!initDisplay())
    {
        Serial0.println("[APP] Display initialization failed");
        return false;
    }

    // --------------------------------------------------------
    // INPUT
    // --------------------------------------------------------

    // if (!initInput())
    // {
    //     Serial0.println("[APP] Input initialization failed");
    //     return false;
    // }

    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    if (!initWebServer())
    {
        Serial0.println("[APP] WARNING: web server failed (continuing)");
    }

    // --------------------------------------------------------
    // STARTUP SOUND
    //
    // Стрим System, короткий fade in/out 200 мс.
    // Не зависит от vol_media / vol_alarm.
    // --------------------------------------------------------

    Serial0.println("[APP] Playing startup sound...");

   SoundManager::PlayOptions opts;
opts.stream       = SoundManager::AudioStream::System;
opts.localPercent = 100;
opts.fadeInMs     = 200;
opts.fadeOutMs    = 200;
opts.curve        = SoundManager::FadeCurve::Linear;

if (!_soundManager.play(Constants::STARTUP_SOUND, opts))

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    _ready = true;

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("        SMART CLOCK READY");
    Serial0.println("========================================");

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void App::update()
{
    if (!_ready)
        return;

    // --------------------------------------------------------
    // WEB SERVER (принимает HTTP, меняет Settings)
    // --------------------------------------------------------

    _webServer.update();

    // --------------------------------------------------------
    // INPUT / SENSORS
    // --------------------------------------------------------

    // _inputManager.update();
    _sensorManager.update();

    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

    _clockSystem.update();

    // --------------------------------------------------------
    // DISPLAY
    //
    // Внутри:
    //   - pollBrightness() читает Settings и дёргает LVGL
    //   - screens.update() обновляет цифры/датчики
    //   - lvgl.update() гоняет lv_timer_handler
    // --------------------------------------------------------

    _displaySystem.update();

    // --------------------------------------------------------
    // SOUND
    //
    // Качает PCM в I2S, следит за fade in/out,
    // завершает трек, обрабатывает плавный стоп.
    // --------------------------------------------------------

    _soundManager.update();
}

// ============================================================
// READY
// ============================================================

bool App::isReady() const
{
    return _ready;
}

// ============================================================
// INIT SETTINGS
// ============================================================

bool App::initSettings()
{
    Serial0.println();
    Serial0.println("[APP] Initializing settings...");

    if (!_settings.begin())
    {
        Serial0.println("[APP] Settings FAILED");
        return false;
    }

    Serial0.println("[APP] Settings OK");
    return true;
}

// ============================================================
// INIT SPI
// ============================================================

bool App::initSPI()
{
    Serial0.println();
    Serial0.println("[APP] Initializing SPI...");

    if (!_spiManager.begin())
    {
        Serial0.println("[APP] SPI failed");
        return false;
    }

    Serial0.println("[APP] SPI OK");
    return true;
}

// ============================================================
// INIT I2S
// ============================================================

bool App::initI2S()
{
    Serial0.println();
    Serial0.println("[APP] Initializing I2S...");

    if (!_i2sManager.begin())
    {
        Serial0.println("[APP] I2S failed");
        return false;
    }

    Serial0.println("[APP] I2S OK");
    return true;
}

// ============================================================
// INIT SD
// ============================================================

bool App::initSD()
{
    Serial0.println();
    Serial0.println("[APP] Initializing SD...");

    if (!_sdManager.begin(PIN_SD_CS))
    {
        Serial0.println("[APP] SD failed");
        return false;
    }

    Serial0.println("[APP] SD OK");
    return true;
}

// ============================================================
// INIT SOUND
// ============================================================

bool App::initSound()
{
    Serial0.println();
    Serial0.println("[APP] Initializing sound...");

    if (!_soundManager.begin())
    {
        Serial0.println("[APP] Sound failed");
        return false;
    }

    Serial0.println("[APP] Sound OK");
    return true;
}

// ============================================================
// INIT CLOCK
// ============================================================

bool App::initClock()
{
    Serial0.println();
    Serial0.println("[APP] Initializing clock...");

    if (!_clockSystem.begin())
    {
        Serial0.println("[APP] Clock failed");
        return false;
    }

    Serial0.println("[APP] Clock OK");
    return true;
}

// ============================================================
// INIT SENSORS
// ============================================================

bool App::initSensors()
{
    Serial0.println();
    Serial0.println("[APP] Initializing sensors...");

    if (!_sensorManager.begin())
    {
        Serial0.println("[APP] Sensors failed");
        return false;
    }

    Serial0.println("[APP] Sensors OK");
    return true;
}

// ============================================================
// INIT DISPLAY
// ============================================================

bool App::initDisplay()
{
    Serial0.println();
    Serial0.println("[APP] Initializing display...");

    if (!_displaySystem.begin())
    {
        Serial0.println("[APP] Display failed");
        return false;
    }

    Serial0.println("[APP] Display OK");
    return true;
}

// ============================================================
// INIT INPUT
// ============================================================

// bool App::initInput()
// {
//     Serial0.println();
//     Serial0.println("[APP] Initializing input...");

//     if (!_inputManager.begin())
//     {
//         Serial0.println("[APP] Input failed");
//         return false;
//     }

//     Serial0.println("[APP] Input OK");
//     return true;
// }

// ============================================================
// INIT WEB SERVER
// ============================================================

bool App::initWebServer()
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("[APP] Initializing web server...");
    Serial0.println("========================================");

    if (!_webServer.begin(
            _settings,
            _sdManager,
            _soundManager,
            WIFI_SSID,
            WIFI_PASSWORD))
    {
        Serial0.println("[APP] Web server FAILED");
        return false;
    }

    Serial0.print("[APP] Web server OK — http://");
    Serial0.println(_webServer.getIP());

    return true;
}