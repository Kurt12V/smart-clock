#include "App.h"

#include "Config.h"
#include "Pins.h"
#include "Constants.h"
#include "Version.h"

// ============================================================
// WIFI CREDENTIALS
// ============================================================

static constexpr const char* WIFI_SSID     = "tpl45";
static constexpr const char* WIFI_PASSWORD = "12713714";

// ============================================================
// PWM CHANNELS
// ============================================================

namespace
{
    // Канал 0 занят подсветкой дисплея (LVGLManager).
    // COB: каналы 1..4.
    constexpr uint8_t COB_CH1 = 1;
    constexpr uint8_t COB_CH2 = 2;
    constexpr uint8_t COB_CH3 = 3;
    constexpr uint8_t COB_CH4 = 4;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

App::App()
    : _ready(false),

      // ========================================================
      // SETTINGS
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

      _inputManager(),
      _sensorManager(),

      // ========================================================
      // COB LED
      // ========================================================

      _cob1(
          PIN_COB1,
          COB_CH1,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob2(
          PIN_COB2,
          COB_CH2,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob3(
          PIN_COB3,
          COB_CH3,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cob4(
          PIN_COB4,
          COB_CH4,
          Config::COB_PWM_FREQUENCY,
          Config::COB_PWM_RESOLUTION
      ),

      _cobManager(
          _cob1,
          _cob2,
          _cob3,
          _cob4
      ),

      _cobEffects(
          _cobManager
      ),

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

    if (!initSettings())
    {
        Serial0.println("[APP] Settings initialization failed");
        return false;
    }

    if (!initSPI())
    {
        Serial0.println("[APP] SPI initialization failed");
        return false;
    }

    if (!initI2S())
    {
        Serial0.println("[APP] I2S initialization failed");
        return false;
    }

    if (!initSD())
    {
        Serial0.println("[APP] SD initialization failed");
        return false;
    }

    if (!initSound())
    {
        Serial0.println("[APP] Sound initialization failed");
        return false;
    }

    if (!initClock())
    {
        Serial0.println("[APP] Clock initialization failed");
        return false;
    }

    if (!initSensors())
    {
        Serial0.println("[APP] Sensors initialization failed");
        return false;
    }

    if (!initCob())
    {
        Serial0.println("[APP] COB initialization failed");
        return false;
    }

    if (!initDisplay())
    {
        Serial0.println("[APP] Display initialization failed");
        return false;
    }

    if (!initInput())
    {
        Serial0.println("[APP] Input initialization failed");
        return false;
    }

    if (!initWebServer())
    {
        Serial0.println("[APP] WARNING: web server failed (continuing)");
    }

    // --------------------------------------------------------
    // STARTUP SOUND
    // --------------------------------------------------------

    Serial0.println("[APP] Playing startup sound...");

    {
        SoundManager::PlayOptions opts;

        opts.stream       = SoundManager::AudioStream::System;
        opts.localPercent = 100;
        opts.fadeInMs     = 200;
        opts.fadeOutMs    = 200;
        opts.curve        = SoundManager::FadeCurve::Linear;

        if (!_soundManager.play(Constants::STARTUP_SOUND, opts))
        {
            Serial0.println("[APP] WARNING: startup sound failed");
        }
    }

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

    _webServer.update();

    _inputManager.update();
    _sensorManager.update();

    _clockSystem.update();

    updateCob();

    _displaySystem.update();

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
// INIT COB
// ============================================================

bool App::initCob()
{
    Serial0.println();
    Serial0.println("[APP] Initializing COB...");

    _cobManager.begin();

    // Начальные яркости (0..3) из Settings
    for (uint8_t i = 0; i < 4; ++i)
    {
        Param p = static_cast<Param>(
            static_cast<uint8_t>(Param::COB_BRIGHTNESS_1) + i
        );

        _cobEffects.setBrightness(
            i,
            static_cast<uint8_t>(_settings.get(p))
        );
    }

    _cobEffects.setEffect(
        static_cast<CobEffectType>(
            _settings.get(Param::COB_EFFECT)
        )
    );

    _cobEffects.setSpeed(
        static_cast<uint8_t>(_settings.get(Param::COB_SPEED))
    );

    _cobEffects.setEnabled(
        _settings.get(Param:: COB_ENABLED) != 0
    );

    _cobEffects.begin();

    Serial0.println("[APP] COB OK");
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

bool App::initInput()
{
    Serial0.println();
    Serial0.println("[APP] Initializing input...");

    if (!_inputManager.begin())
    {
        Serial0.println("[APP] Input failed");
        return false;
    }

    Serial0.println("[APP] Input OK");
    return true;
}

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

// ============================================================
// UPDATE COB
//
// Читает Settings каждый цикл, применяет изменения в
// CobEffects. Изменения из веба подхватываются ≤10 мс.
// ============================================================

void App::updateCob()
{
    // Яркости 0..3
    for (uint8_t i = 0; i < 4; ++i)
    {
        Param p = static_cast<Param>(
            static_cast<uint8_t>(Param::COB_BRIGHTNESS_1) + i
        );

        uint8_t want = static_cast<uint8_t>(_settings.get(p));

        if (_cobEffects.brightness(i) != want)
            _cobEffects.setBrightness(i, want);
    }

    // Эффект
    CobEffectType eff = static_cast<CobEffectType>(
        _settings.get(Param::COB_EFFECT)
    );

    if (_cobEffects.effect() != eff)
        _cobEffects.setEffect(eff);

    // Скорость
    uint8_t spd = static_cast<uint8_t>(
        _settings.get(Param::COB_SPEED)
    );

    if (_cobEffects.speed() != spd)
        _cobEffects.setSpeed(spd);

    // Вкл/выкл
    bool on = _settings.get(Param::COB_ENABLED) != 0;

    if (_cobEffects.isEnabled() != on)
        _cobEffects.setEnabled(on);

    // Тик эффекта
    _cobEffects.update();

    // Тик CobLed (обрабатывает fadeTo, если где-то используется)
    _cobManager.update();
}