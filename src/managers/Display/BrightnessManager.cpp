#include "BrightnessManager.h"

#include "Config.h"

#include <math.h>

// ============================================================
// CONSTRUCTOR
// ============================================================

BrightnessManager::BrightnessManager()
    : _settings(nullptr),
      _sensors(nullptr),
      _clock(nullptr),
      _sink(nullptr),
      _mode(Mode::AUTO),
      _config{},
      _manualBrightness(::Config::DISPLAY_DEFAULT_BRIGHTNESS),
      _brightness(::Config::DISPLAY_DEFAULT_BRIGHTNESS),
      _sensorTarget(0),
      _profileTarget(0),
      _initialized(false),
      _hasAutoTarget(false),
      _lastUpdateMs(0)
{
    // --------------------------------------------------------
    // TIME PROFILES
    // --------------------------------------------------------

    _config.dayStartMinute     = 7 * 60;
    _config.nightStartMinute   = 22 * 60;
    _config.eveningDurationMin = 120;

    _config.day = {
        35, 100, 65
    };

    _config.evening = {
        15, 60, 30
    };

    _config.night = {
        5, 25, 10
    };

    // --------------------------------------------------------
    // FILTER CONFIGURATION
    // --------------------------------------------------------

    _config.offset         = 0;
    _config.hysteresisPct  = 3;
    _config.smoothingAlpha = 0.2f;

    // Время, за которое яркость приближается
    // к целевому значению.
    _config.smoothingTimeMs = 1500;

    // --------------------------------------------------------
    // LIGHT SENSOR CALIBRATION
    // --------------------------------------------------------

    _config.luxMin = 1.0f;
    _config.luxMax = 1000.0f;
}

// ============================================================
// BEGIN
// ============================================================

void BrightnessManager::begin(
    SettingsManager*        settings,
    SensorManager*          sensors,
    ClockSystem*            clock,
    IScreenBrightnessSink*  sink
)
{
    _settings = settings;
    _sensors  = sensors;
    _clock    = clock;
    _sink     = sink;

    // --------------------------------------------------------
    // LOAD MANUAL BRIGHTNESS
    // --------------------------------------------------------

    if (_settings != nullptr && _settings->initialized())
    {
        _manualBrightness = clampBrightness(
            _settings->get(Param::BRIGHTNESS)
        );
    }
    else
    {
        _manualBrightness = clampBrightness(
            _manualBrightness
        );
    }

    // --------------------------------------------------------
    // INITIAL STATE
    // --------------------------------------------------------

    _brightness = clampBrightness(
        _manualBrightness
    );

    _sensorTarget  = _brightness;
    _profileTarget = _brightness;

    _hasAutoTarget = false;
    _lastUpdateMs  = millis();

    _initialized = true;

    // Начальное состояние дисплея.
    applyBrightness(_brightness);
}

// ============================================================
// UPDATE
// ============================================================

void BrightnessManager::update()
{
    if (!_initialized)
        return;

    const uint32_t now = millis();

    // Безопасно работает и при переполнении millis().
    uint32_t deltaTimeMs = now - _lastUpdateMs;

    _lastUpdateMs = now;

    // Ограничиваем скачок времени после длительной
    // блокировки основного цикла.
    //
    // Это не позволяет сглаживанию мгновенно перескочить
    // к цели после длительной паузы.
    if (deltaTimeMs > 1000)
        deltaTimeMs = 1000;

    // --------------------------------------------------------
    // MANUAL MODE
    // --------------------------------------------------------

    if (_mode == Mode::MANUAL)
    {
        _hasAutoTarget = false;

        const int target = clampBrightness(
            _manualBrightness
        );

        if (_brightness != target)
        {
            _brightness = target;
            applyBrightness(_brightness);
        }

        return;
    }

    // --------------------------------------------------------
    // AUTO MODE
    // --------------------------------------------------------

    const int target = calculateAutoTarget();

    // При первом автоматическом расчёте не нужно плавно
    // переходить от ранее сохранённой ручной яркости.
    //
    // Это позволяет избежать временного применения
    // слишком яркого значения после запуска.
    if (!_hasAutoTarget)
    {
        _brightness = clampBrightness(target);

        _hasAutoTarget = true;

        applyBrightness(_brightness);

        return;
    }

    // --------------------------------------------------------
    // HYSTERESIS
    // --------------------------------------------------------

    if (!applyHysteresis(target, _brightness))
        return;

    // --------------------------------------------------------
    // TIME-BASED SMOOTHING
    // --------------------------------------------------------

    const int smoothed = applySmoothing(
        target,
        deltaTimeMs
    );

    const int nextBrightness = clampBrightness(
        smoothed
    );

    if (nextBrightness != _brightness)
    {
        _brightness = nextBrightness;

        applyBrightness(_brightness);
    }
}

// ============================================================
// MODE
// ============================================================

void BrightnessManager::setMode(Mode mode)
{
    if (_mode == mode)
        return;

    _mode = mode;

    _hasAutoTarget = false;
    _lastUpdateMs  = millis();

    // При переходе в MANUAL применяем ручное значение.
    if (_mode == Mode::MANUAL)
    {
        _brightness = clampBrightness(
            _manualBrightness
        );

        applyBrightness(_brightness);
    }

    // При переходе в AUTO целевое значение будет рассчитано
    // при следующем update().
}

// ============================================================
// MANUAL BRIGHTNESS
// ============================================================

void BrightnessManager::setManualBrightness(int value)
{
    value = clampBrightness(value);

    _manualBrightness = value;

    // Сохраняем пользовательское значение.
    //
    // В соответствии с исходным поведением проекта
    // applyBrightness() также сохраняет применённую
    // яркость в SettingsManager.
    if (_settings != nullptr && _settings->initialized())
    {
        _settings->set(
            Param::BRIGHTNESS,
            value
        );
    }

    if (_mode == Mode::MANUAL)
    {
        if (_brightness != value)
        {
            _brightness = value;

            applyBrightness(_brightness);
        }
    }
}

// ============================================================
// PROFILE NORMALIZATION
// ============================================================

BrightnessManager::Profile
BrightnessManager::normalizeProfile(
    const Profile& profile
) const
{
    Profile result = profile;

    result.min = clampBrightness(result.min);
    result.max = clampBrightness(result.max);

    if (result.min > result.max)
    {
        const int temporary = result.min;

        result.min = result.max;
        result.max = temporary;
    }

    result.base = constrain(
        result.base,
        result.min,
        result.max
    );

    return result;
}

// ============================================================
// PROFILE SELECTION
// ============================================================

BrightnessManager::Profile
BrightnessManager::selectProfile(
    uint16_t minuteOfDay
) const
{
    const Profile day = normalizeProfile(
        _config.day
    );

    const Profile evening = normalizeProfile(
        _config.evening
    );

    const Profile night = normalizeProfile(
        _config.night
    );

    const uint16_t dayStart = static_cast<uint16_t>(
        _config.dayStartMinute % 1440u
    );

    const uint16_t nightStart = static_cast<uint16_t>(
        _config.nightStartMinute % 1440u
    );

    const uint16_t duration = static_cast<uint16_t>(
        _config.eveningDurationMin > 1440
            ? 1440
            : _config.eveningDurationMin
    );

    const uint16_t eveningStart = static_cast<uint16_t>(
        (nightStart + 1440u - duration) % 1440u
    );

    minuteOfDay %= 1440u;

    // --------------------------------------------------------
    // STANDARD SCHEDULE
    // --------------------------------------------------------

    if (dayStart < nightStart)
    {
        if (minuteOfDay < dayStart ||
            minuteOfDay >= nightStart)
        {
            return night;
        }

        if (duration > 0 &&
            minuteOfDay >= eveningStart)
        {
            return evening;
        }

        return day;
    }

    // --------------------------------------------------------
    // WRAPPED DAY INTERVAL
    // --------------------------------------------------------
    //
    // Поддерживаем расписание, в котором интервал дня
    // пересекает полночь.
    //
    // Если расписание неоднозначно, предпочтение отдаётся
    // безопасному ночному профилю.
    // --------------------------------------------------------

    if (dayStart > nightStart)
    {
        const bool isDay =
            minuteOfDay >= dayStart ||
            minuteOfDay < nightStart;

        if (!isDay)
            return night;

        if (duration > 0 &&
            minuteOfDay < nightStart &&
            minuteOfDay >= eveningStart)
        {
            return evening;
        }

        return day;
    }

    // Одинаковые границы не образуют нормального
    // дневного интервала. Используем ночной профиль.
    return night;
}

// ============================================================
// SENSOR READING
// ============================================================

bool BrightnessManager::readLux(float& lux) const
{
    lux = 0.0f;

    if (_sensors == nullptr)
        return false;

    const SensorData& data = _sensors->getData();

    if (!data.lightValid)
        return false;

    const float measuredLux = data.lightLux;

    // Отклоняем NaN и бесконечные значения.
    if (!isfinite(measuredLux))
        return false;

    // Отрицательная освещённость недопустима.
    if (measuredLux < 0.0f)
        return false;

    lux = measuredLux;

    return true;
}

// ============================================================
// LUX -> PERCENT
// ============================================================
//
// Нелинейное преобразование.
//
// При малой освещённости яркость растёт медленнее,
// чем при линейном преобразовании.
//
// После преобразования значение всё равно ограничивается
// профилем времени суток.
// ============================================================

int BrightnessManager::luxToPercent(float lux) const
{
    if (!isfinite(lux) || lux < 0.0f)
        return 0;

    const float luxMin = _config.luxMin;
    const float luxMax = _config.luxMax;

    if (!isfinite(luxMin) ||
        !isfinite(luxMax) ||
        luxMax <= luxMin)
    {
        return 0;
    }

    if (lux <= luxMin)
        return 0;

    if (lux >= luxMax)
        return 100;

    const float normalized = constrain(
        (lux - luxMin) / (luxMax - luxMin),
        0.0f,
        1.0f
    );

    // Нелинейная характеристика.
    //
    // Показатель 0.6f делает зависимость менее линейной.
    // Это начальная настройка, которую следует проверить
    // на реальных дисплеях.
    const float perceptual = powf(
        normalized,
        0.6f
    );

    const int result = static_cast<int>(
        perceptual * 100.0f + 0.5f
    );

    return constrain(result, 0, 100);
}

// ============================================================
// SAFE TARGET
// ============================================================

int BrightnessManager::calculateSafeTarget() const
{
    const Profile night = normalizeProfile(
        _config.night
    );

    return clampBrightness(night.base);
}

// ============================================================
// AUTOMATIC TARGET
// ============================================================

int BrightnessManager::calculateAutoTarget()
{
    uint16_t minuteOfDay = 0;

    bool timeValid = false;

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    if (_clock != nullptr && _clock->isTimeValid())
    {
        const uint8_t hour = _clock->hour();
        const uint8_t minute = _clock->minute();

        if (hour < 24 && minute < 60)
        {
            minuteOfDay = static_cast<uint16_t>(
                static_cast<uint16_t>(hour) * 60u +
                static_cast<uint16_t>(minute)
            );

            timeValid = true;
        }
    }

    // --------------------------------------------------------
    // INVALID TIME
    // --------------------------------------------------------
    //
    // При неизвестном времени не используем дневной профиль.
    // Выбираем безопасную яркость на основе ночного профиля.
    // --------------------------------------------------------

    if (!timeValid)
    {
        const int safeTarget = calculateSafeTarget();

        _sensorTarget  = safeTarget;
        _profileTarget = safeTarget;

        return safeTarget;
    }

    // --------------------------------------------------------
    // PROFILE
    // --------------------------------------------------------

    const Profile profile = selectProfile(
        minuteOfDay
    );

    // --------------------------------------------------------
    // SENSOR
    // --------------------------------------------------------

    float lux = 0.0f;

    const bool sensorValid = readLux(lux);

    int sensorValue = profile.base;

    if (sensorValid)
    {
        sensorValue = luxToPercent(lux);
    }

    _sensorTarget = sensorValue;

    // --------------------------------------------------------
    // USER OFFSET
    // --------------------------------------------------------

    // Ограничиваем поправку допустимым диапазоном.
    const int offset = constrain(
        _config.offset,
        -100,
        100
    );

    const int adjustedTarget = constrain(
        sensorValue + offset,
        0,
        100
    );

    // --------------------------------------------------------
    // PROFILE LIMITS
    // --------------------------------------------------------

    // Профиль ограничивает итоговое значение ПОСЛЕ
    // преобразования датчика и пользовательской поправки.
    const int target = constrain(
        adjustedTarget,
        profile.min,
        profile.max
    );

    _profileTarget = target;

    return clampBrightness(target);
}

// ============================================================
// BRIGHTNESS CLAMP
// ============================================================

int BrightnessManager::clampBrightness(int value) const
{
    int minimum = ::Config::DISPLAY_MIN_BRIGHTNESS;
    int maximum = ::Config::DISPLAY_MAX_BRIGHTNESS;

    minimum = constrain(minimum, 0, 100);
    maximum = constrain(maximum, 0, 100);

    if (minimum > maximum)
    {
        const int temporary = minimum;

        minimum = maximum;
        maximum = temporary;
    }

    return constrain(
        value,
        minimum,
        maximum
    );
}

// ============================================================
// HYSTERESIS
// ============================================================

bool BrightnessManager::applyHysteresis(
    int candidate,
    int current
) const
{
    const int zone = constrain(
        _config.hysteresisPct,
        0,
        100
    );

    const int difference = candidate - current;

    return difference > zone ||
           difference < -zone;
}

// ============================================================
// TIME-BASED SMOOTHING
// ============================================================
//
// Сглаживание зависит от фактически прошедшего времени,
// а не от количества вызовов update().
//
// smoothingTimeMs определяет характерную скорость реакции.
// ============================================================

int BrightnessManager::applySmoothing(
    int candidate,
    uint32_t deltaTimeMs
)
{
    candidate = clampBrightness(candidate);

    if (deltaTimeMs == 0)
        return _brightness;

    const uint32_t smoothingTimeMs =
        _config.smoothingTimeMs;

    if (smoothingTimeMs == 0)
        return candidate;

    // alpha = 1 - exp(-dt / tau)
    //
    // Чем больше dt, тем сильнее влияние нового значения.
    const float ratio =
        static_cast<float>(deltaTimeMs) /
        static_cast<float>(smoothingTimeMs);

    const float alpha = 1.0f - expf(-ratio);

    const float smoothed =
        static_cast<float>(_brightness) +
        alpha *
        (
            static_cast<float>(candidate) -
            static_cast<float>(_brightness)
        );

    const int result = static_cast<int>(
        smoothed + 0.5f
    );

    return clampBrightness(result);
}

// ============================================================
// APPLY BRIGHTNESS
// ============================================================

void BrightnessManager::applyBrightness(int value)
{
    // Сначала синхронизируем значение с реальными
    // ограничениями драйвера.
    value = clampBrightness(value);

    // Защита от рассогласования состояния.
    _brightness = value;

    // Управляем только подсветкой экранов.
    if (_sink != nullptr)
    {
        _sink->setScreenBrightness(value);
    }

    // Сохраняем значение согласно исходному поведению.
    //
    // Важно: здесь сохранена исходная логика, которая
    // допускает запись автоматической яркости в BRIGHTNESS.
    if (_settings != nullptr && _settings->initialized())
    {
        _settings->set(
            Param::BRIGHTNESS,
            value
        );
    }
}