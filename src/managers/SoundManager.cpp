#include "SoundManager.h"

#include <cstring>
#include <cmath>

// ============================================================
// CONSTRUCTOR
// ============================================================

SoundManager::SoundManager(
    SDManager&       sdManager,
    SettingsManager& settings,
    I2SManager&      i2sManager
)
    : _sdManager(sdManager),
      _settings(settings),
      _i2sManager(i2sManager),

      _initialized(false),
      _state(State::STOPPED),

      _positionBytes(0),
      _durationMs(0),

      _currentStream(AudioStream::Media),
      _localPercent(100),

      _fadeInEnabled(false),
      _fadeOutEnabled(false),
      _fadeInMs(0),
      _fadeOutMs(0),
      _fadeStartMs(0),
      _fadeCurve(FadeCurve::Linear),

      _stopFadeActive(false),
      _stopFadeStartMs(0),
      _stopFadeDurationMs(0),

      _lastStatusMs(0)
{
    memset(&_wav, 0, sizeof(_wav));
}

// ============================================================
// BEGIN / END
// ============================================================

bool SoundManager::begin()
{
    Serial0.println();
    Serial0.println("[SOUND] Starting...");

    if (!_sdManager.isReady())
    {
        Serial0.println("[SOUND] ERROR: SD not ready");
        return false;
    }

    if (!_i2sManager.isInitialized())
    {
        Serial0.println("[SOUND] ERROR: I2S not initialized");
        return false;
    }

    _initialized = true;

    Serial0.print("[SOUND] vol_media  = ");
    Serial0.println(
        _settings.get(Param::VOLUME_MEDIA)
    );

    Serial0.print("[SOUND] vol_alarm  = ");
    Serial0.println(
        _settings.get(Param::VOLUME_ALARM)
    );

    Serial0.print("[SOUND] vol_system = ");
    Serial0.println(
        _settings.get(Param::VOLUME_SYSTEM)
    );

    Serial0.println("[SOUND] Ready");

    return true;
}

void SoundManager::end()
{
    stop();

    _initialized = false;
}

// ============================================================
// UPDATE
// ============================================================

void SoundManager::update()
{
    if (!_initialized)
        return;

    if (_state != State::PLAYING &&
        _state != State::FADING_OUT)
        return;

    if (!_file)
    {
        finishPlayback();
        return;
    }

    // --------------------------------------------------------
    // Проверка окончания плавного стопа
    // --------------------------------------------------------

    if (_stopFadeActive)
    {
        uint32_t elapsed =
            millis() - _stopFadeStartMs;

        if (elapsed >= _stopFadeDurationMs)
        {
            finishPlayback();
            return;
        }
    }

    readAndPlayChunk();
}

// ============================================================
// PLAY
// ============================================================

bool SoundManager::play(const char* path)
{
    PlayOptions opts;

    return play(path, opts);
}

// ============================================================
// PLAY WITH OPTIONS
// ============================================================

bool SoundManager::play(
    const char* path,
    const PlayOptions& opts
)
{
    Serial0.println();
    Serial0.println("[SOUND] play()");

    if (!_initialized)
    {
        Serial0.println(
            "[SOUND] ERROR: not initialized"
        );

        return false;
    }

    if (!path)
    {
        Serial0.println(
            "[SOUND] ERROR: path NULL"
        );

        return false;
    }

    // --------------------------------------------------------
    // Останавливаем предыдущий звук
    // --------------------------------------------------------

    stop();

    // --------------------------------------------------------
    // Сохраняем параметры воспроизведения
    // --------------------------------------------------------

    _currentPath   = path;

    _currentStream = opts.stream;

    _localPercent =
        constrain(
            opts.localPercent,
            0,
            100
        );

    _fadeInEnabled =
        opts.fadeInMs > 0;

    _fadeOutEnabled =
        opts.fadeOutMs > 0;

    _fadeInMs =
        opts.fadeInMs;

    _fadeOutMs =
        opts.fadeOutMs;

    _fadeCurve =
        opts.curve;

    _fadeStartMs =
        millis();

    _stopFadeActive =
        false;

    // --------------------------------------------------------
    // Открываем WAV
    // --------------------------------------------------------

    if (!openWav(path))
        return false;

    if (!validateWav())
    {
        _file.close();
        return false;
    }

    Serial0.printf(
        "[SOUND] %s  ch=%u  %lu Hz  %lu ms  stream=%u\n",
        path,
        _wav.channels,
        static_cast<unsigned long>(
            _wav.sampleRate
        ),
        static_cast<unsigned long>(
            _durationMs
        ),
        static_cast<uint8_t>(
            _currentStream
        )
    );

    // --------------------------------------------------------
    // Настраиваем I2S
    // --------------------------------------------------------

    if (!_i2sManager.beginSpeaker(
            _wav.sampleRate))
    {
        _file.close();
        return false;
    }

    // --------------------------------------------------------
    // Переходим к WAV data chunk
    // --------------------------------------------------------

    if (!_file.seek(
            _wav.dataOffset))
    {
        _file.close();
        return false;
    }

    _positionBytes = 0;

    // --------------------------------------------------------
    // Запускаем динамик
    // --------------------------------------------------------

    if (!_i2sManager.startSpeaker())
    {
        _file.close();
        return false;
    }

    _state =
        State::PLAYING;

    _lastStatusMs =
        millis();

    return true;
}

// ============================================================
// STOP
// ============================================================

void SoundManager::stop()
{
    if (_state == State::STOPPED)
    {
        if (_file)
            _file.close();

        _currentPath = "";

        return;
    }

    _i2sManager.stopSpeaker();

    _i2sManager.clearSpeaker();

    if (_file)
        _file.close();

    _state =
        State::STOPPED;

    _positionBytes =
        0;

    _currentPath =
        "";

    _stopFadeActive =
        false;

    Serial0.println(
        "[SOUND] stopped"
    );
}

// ============================================================
// STOP WITH FADE
// ============================================================

void SoundManager::stop(
    uint32_t fadeOutMs
)
{
    if (_state != State::PLAYING ||
        fadeOutMs == 0)
    {
        stop();
        return;
    }

    _stopFadeActive =
        true;

    _stopFadeStartMs =
        millis();

    _stopFadeDurationMs =
        fadeOutMs;

    _state =
        State::FADING_OUT;

    Serial0.printf(
        "[SOUND] fade out %lu ms\n",
        static_cast<unsigned long>(
            fadeOutMs
        )
    );
}

// ============================================================
// PAUSE
// ============================================================

bool SoundManager::pause()
{
    if (_state != State::PLAYING)
        return false;

    if (!_i2sManager.stopSpeaker())
        return false;

    _state =
        State::PAUSED;

    return true;
}

// ============================================================
// RESUME
// ============================================================

bool SoundManager::resume()
{
    if (_state != State::PAUSED)
        return false;

    if (!_i2sManager.startSpeaker())
        return false;

    _state =
        State::PLAYING;

    return true;
}

// ============================================================
// OPEN WAV
// ============================================================

bool SoundManager::openWav(
    const char* path
)
{
    _file =
        _sdManager
            .card()
            .fs()
            .open(
                path,
                FILE_READ
            );

    if (!_file)
        return false;

    if (!parseWav(_file))
    {
        _file.close();
        return false;
    }

    return true;
}

// ============================================================
// PARSE WAV
// ============================================================

bool SoundManager::parseWav(
    File& file
)
{
    uint8_t header[12];

    if (file.read(
            header,
            sizeof(header)
        ) != sizeof(header))
    {
        return false;
    }

    if (memcmp(
            header,
            "RIFF",
            4
        ) != 0)
    {
        return false;
    }

    if (memcmp(
            header + 8,
            "WAVE",
            4
        ) != 0)
    {
        return false;
    }

    bool foundFmt =
        false;

    bool foundData =
        false;

    while (file.available())
    {
        uint8_t chunkHeader[8];

        if (file.read(
                chunkHeader,
                sizeof(chunkHeader)
            ) != sizeof(chunkHeader))
        {
            break;
        }

        char chunkId[5];

        memcpy(
            chunkId,
            chunkHeader,
            4
        );

        chunkId[4] =
            '\0';

        uint32_t chunkSize =
            static_cast<uint32_t>(
                chunkHeader[4]
            ) |
            (
                static_cast<uint32_t>(
                    chunkHeader[5]
                ) << 8
            ) |
            (
                static_cast<uint32_t>(
                    chunkHeader[6]
                ) << 16
            ) |
            (
                static_cast<uint32_t>(
                    chunkHeader[7]
                ) << 24
            );

        uint32_t chunkDataOffset =
            static_cast<uint32_t>(
                file.position()
            );

        // ----------------------------------------------------
        // FMT
        // ----------------------------------------------------

        if (memcmp(
                chunkId,
                "fmt ",
                4
            ) == 0)
        {
            if (chunkSize < 16)
                return false;

            uint8_t fmt[16];

            if (file.read(
                    fmt,
                    16
                ) != 16)
            {
                return false;
            }

            _wav.audioFormat =
                fmt[0] |
                (fmt[1] << 8);

            _wav.channels =
                fmt[2] |
                (fmt[3] << 8);

            _wav.sampleRate =
                static_cast<uint32_t>(
                    fmt[4]
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[5]
                    ) << 8
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[6]
                    ) << 16
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[7]
                    ) << 24
                );

            _wav.byteRate =
                static_cast<uint32_t>(
                    fmt[8]
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[9]
                    ) << 8
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[10]
                    ) << 16
                ) |
                (
                    static_cast<uint32_t>(
                        fmt[11]
                    ) << 24
                );

            _wav.blockAlign =
                fmt[12] |
                (fmt[13] << 8);

            _wav.bitsPerSample =
                fmt[14] |
                (fmt[15] << 8);

            file.seek(
                chunkDataOffset +
                chunkSize
            );

            foundFmt =
                true;
        }

        // ----------------------------------------------------
        // DATA
        // ----------------------------------------------------

        else if (memcmp(
                    chunkId,
                    "data",
                    4
                ) == 0)
        {
            _wav.dataOffset =
                chunkDataOffset;

            _wav.dataSize =
                chunkSize;

            foundData =
                true;

            break;
        }

        // ----------------------------------------------------
        // OTHER CHUNK
        // ----------------------------------------------------

        else
        {
            uint32_t next =
                chunkDataOffset +
                chunkSize;

            // WAV chunks are word aligned
            if (chunkSize & 1)
                next++;

            file.seek(next);
        }
    }

    if (!foundFmt ||
        !foundData)
    {
        return false;
    }

    if (_wav.byteRate == 0)
        return false;

    _durationMs =
        static_cast<uint32_t>(
            (
                static_cast<uint64_t>(
                    _wav.dataSize
                ) * 1000ULL
            ) /
            _wav.byteRate
        );

    return true;
}

// ============================================================
// VALIDATE WAV
// ============================================================

bool SoundManager::validateWav() const
{
    // PCM
    if (_wav.audioFormat != 1)
        return false;

    // Mono or stereo
    if (_wav.channels != 1 &&
        _wav.channels != 2)
    {
        return false;
    }

    if (_wav.sampleRate == 0)
        return false;

    // Only 16-bit WAV
    if (_wav.bitsPerSample != 16)
        return false;

    if (_wav.blockAlign == 0)
        return false;

    if (_wav.dataSize == 0)
        return false;

    return true;
}

// ============================================================
// READ + PLAY CHUNK
// ============================================================

bool SoundManager::readAndPlayChunk()
{
    if (!_file)
        return false;

    // --------------------------------------------------------
    // Проверяем конец файла
    // --------------------------------------------------------

    if (_positionBytes >= _wav.dataSize)
    {
        finishPlayback();
        return true;
    }

    uint32_t remaining =
        _wav.dataSize -
        _positionBytes;

    size_t bytesToRead =
        min(
            static_cast<size_t>(
                remaining
            ),
            BUFFER_BYTES
        );

    size_t bytesRead =
        _file.read(
            _inputBuffer,
            bytesToRead
        );

    if (bytesRead == 0)
    {
        finishPlayback();
        return true;
    }

    const uint32_t sourceBytesRead =
        static_cast<uint32_t>(
            bytesRead
        );

    size_t outputSamples =
        0;

    // --------------------------------------------------------
    // MONO
    // --------------------------------------------------------

    if (_wav.channels == 1)
    {
        size_t sampleCount =
            bytesRead / 2;

        memcpy(
            _outputBuffer,
            _inputBuffer,
            sampleCount *
            sizeof(int16_t)
        );

        outputSamples =
            sampleCount;
    }

    // --------------------------------------------------------
    // STEREO -> MONO
    // --------------------------------------------------------

    else
    {
        const int16_t* stereo =
            reinterpret_cast<
                const int16_t*
            >(_inputBuffer);

        size_t frames =
            (bytesRead / 2) / 2;

        for (size_t i = 0;
             i < frames;
             ++i)
        {
            int32_t left =
                stereo[i * 2];

            int32_t right =
                stereo[i * 2 + 1];

            int32_t mono =
                (left + right) / 2;

            mono =
                constrain(
                    mono,
                    -32768,
                    32767
                );

            _outputBuffer[i] =
                static_cast<int16_t>(
                    mono
                );
        }

        outputSamples =
            frames;
    }

    // --------------------------------------------------------
    // VOLUME
    // --------------------------------------------------------

    uint8_t volume =
        getEffectiveVolume();

    applyVolume(
        _outputBuffer,
        outputSamples,
        volume
    );

    // --------------------------------------------------------
    // WRITE TO I2S
    // --------------------------------------------------------

    size_t outputBytes =
        outputSamples *
        sizeof(int16_t);

    size_t written =
        0;

    if (!_i2sManager.writeSpeaker(
            reinterpret_cast<
                const uint8_t*
            >(_outputBuffer),
            outputBytes,
            written,
            100
        ))
    {
        finishPlayback();
        return false;
    }

    _positionBytes +=
        sourceBytesRead;

    return true;
}

// ============================================================
// FINISH PLAYBACK
// ============================================================

void SoundManager::finishPlayback()
{
    _i2sManager.stopSpeaker();

    _i2sManager.clearSpeaker();

    if (_file)
        _file.close();

    _state =
        State::STOPPED;

    _positionBytes =
        _wav.dataSize;

    _currentPath =
        "";

    _stopFadeActive =
        false;
}

// ============================================================
// APPLY VOLUME
// ============================================================

void SoundManager::applyVolume(
    int16_t* samples,
    size_t sampleCount,
    uint8_t volume
)
{
    if (!samples)
        return;

    if (volume >= 100)
        return;

    if (volume == 0)
    {
        memset(
            samples,
            0,
            sampleCount *
            sizeof(int16_t)
        );

        return;
    }

    float multiplier =
        volume / 100.0f;

    for (size_t i = 0;
         i < sampleCount;
         ++i)
    {
        int32_t value =
            static_cast<int32_t>(
                samples[i] *
                multiplier
            );

        value =
            constrain(
                value,
                -32768,
                32767
            );

        samples[i] =
            static_cast<int16_t>(
                value
            );
    }
}

// ============================================================
// STREAM VOLUME
// ============================================================

uint8_t SoundManager::streamVolume(
    AudioStream s
) const
{
    switch (s)
    {
        case AudioStream::Alarm:

            return static_cast<uint8_t>(
                _settings.get(
                    Param::VOLUME_ALARM
                )
            );

        case AudioStream::System:

            return static_cast<uint8_t>(
                _settings.get(
                    Param::VOLUME_SYSTEM
                )
            );

        case AudioStream::Media:
        default:

            return static_cast<uint8_t>(
                _settings.get(
                    Param::VOLUME_MEDIA
                )
            );
    }
}

// ============================================================
// SET STREAM VOLUME
// ============================================================

void SoundManager::setStreamVolume(
    AudioStream s,
    uint8_t v
)
{
    v =
        constrain(
            v,
            0,
            100
        );

    Param p =
        Param::VOLUME_MEDIA;

    switch (s)
    {
        case AudioStream::Alarm:

            p =
                Param::VOLUME_ALARM;

            break;

        case AudioStream::System:

            p =
                Param::VOLUME_SYSTEM;

            break;

        case AudioStream::Media:
        default:

            p =
                Param::VOLUME_MEDIA;

            break;
    }

    // ========================================================
    // ВАЖНО
    // ========================================================
    //
    // Здесь НЕТ:
    //
    //     _settings.save(p);
    //
    // Значение изменяется только в RAM.
    //
    // SettingsManager сам сохранит его после
    // SAVE_DELAY_MS (700 мс) без новых изменений.
    //
    // Это предотвращает медленные записи Preferences
    // при движении ползунка громкости.
    //
    // ========================================================

    _settings.set(
        p,
        v
    );
}

// ============================================================
// LOCAL PERCENT
// ============================================================

uint8_t SoundManager::getLocalPercent() const
{
    return _localPercent;
}

// ============================================================
// BASE VOLUME
// ============================================================

uint8_t SoundManager::calculateBaseVolume() const
{
    uint8_t streamVol =
        streamVolume(
            _currentStream
        );

    return static_cast<uint8_t>(
        (
            static_cast<uint16_t>(
                streamVol
            ) *
            static_cast<uint16_t>(
                _localPercent
            )
        ) / 100
    );
}

// ============================================================
// EFFECTIVE VOLUME
// ============================================================

uint8_t SoundManager::getEffectiveVolume() const
{
    uint8_t base =
        calculateBaseVolume();

    float fade =
        calculateFadeMultiplier();

    float result =
        base * fade;

    result =
        constrain(
            result,
            0.0f,
            100.0f
        );

    return static_cast<uint8_t>(
        result
    );
}

// ============================================================
// FADE MULTIPLIER
// ============================================================

float SoundManager::calculateFadeMultiplier() const
{
    if (_state != State::PLAYING &&
        _state != State::FADING_OUT)
    {
        return 1.0f;
    }

    uint32_t now =
        millis();

    // --------------------------------------------------------
    // EXPLICIT FADE OUT
    // --------------------------------------------------------

    if (_stopFadeActive &&
        _stopFadeDurationMs > 0)
    {
        uint32_t elapsed =
            now -
            _stopFadeStartMs;

        float value =
            1.0f -
            (
                static_cast<float>(
                    elapsed
                ) /
                static_cast<float>(
                    _stopFadeDurationMs
                )
            );

        value =
            constrain(
                value,
                0.0f,
                1.0f
            );

        return applyFadeCurve(
            value
        );
    }

    // --------------------------------------------------------
    // FADE IN
    // --------------------------------------------------------

    if (_fadeInEnabled &&
        _fadeInMs > 0)
    {
        uint32_t elapsed =
            now -
            _fadeStartMs;

        if (elapsed < _fadeInMs)
        {
            float value =
                static_cast<float>(
                    elapsed
                ) /
                static_cast<float>(
                    _fadeInMs
                );

            return applyFadeCurve(
                value
            );
        }
    }

    // --------------------------------------------------------
    // NATURAL FADE OUT
    // --------------------------------------------------------

    if (_fadeOutEnabled &&
        _fadeOutMs > 0 &&
        _durationMs > 0)
    {
        uint32_t position =
            getPositionMs();

        uint32_t fadeStart =
            _durationMs > _fadeOutMs
                ? _durationMs - _fadeOutMs
                : 0;

        if (position >= fadeStart)
        {
            uint32_t elapsed =
                position -
                fadeStart;

            float value =
                1.0f -
                (
                    static_cast<float>(
                        elapsed
                    ) /
                    static_cast<float>(
                        _fadeOutMs
                    )
                );

            value =
                constrain(
                    value,
                    0.0f,
                    1.0f
                );

            return applyFadeCurve(
                value
            );
        }
    }

    return 1.0f;
}

// ============================================================
// FADE CURVE
// ============================================================

float SoundManager::applyFadeCurve(
    float value
) const
{
    value =
        constrain(
            value,
            0.0f,
            1.0f
        );

    switch (_fadeCurve)
    {
        case FadeCurve::Linear:
            return value;

        case FadeCurve::Exponential:
            return value * value;

        case FadeCurve::Logarithmic:
            return sqrtf(value);

        default:
            return value;
    }
}

// ============================================================
// POSITION
// ============================================================

uint32_t SoundManager::getPositionMs() const
{
    if (_wav.byteRate == 0)
        return 0;

    return static_cast<uint32_t>(
        (
            static_cast<uint64_t>(
                _positionBytes
            ) * 1000ULL
        ) /
        _wav.byteRate
    );
}

// ============================================================
// DURATION
// ============================================================

uint32_t SoundManager::getDurationMs() const
{
    return _durationMs;
}

// ============================================================
// STATUS
// ============================================================

bool SoundManager::isInitialized() const
{
    return _initialized;
}

bool SoundManager::isPlaying() const
{
    return _state ==
           State::PLAYING;
}

bool SoundManager::isPaused() const
{
    return _state ==
           State::PAUSED;
}

bool SoundManager::isActive() const
{
    return _state == State::PLAYING ||
           _state == State::PAUSED ||
           _state == State::FADING_OUT;
}

SoundManager::State
SoundManager::getState() const
{
    return _state;
}

// ============================================================
// STATE STRING
// ============================================================

const char* SoundManager::getStateString() const
{
    switch (_state)
    {
        case State::PLAYING:
            return "playing";

        case State::PAUSED:
            return "paused";

        case State::FADING_OUT:
            return "fading_out";

        default:
            return "stopped";
    }
}

// ============================================================
// CURRENT PATH
// ============================================================

const char* SoundManager::getCurrentPath() const
{
    return _currentPath.c_str();
}

// ============================================================
// CURRENT STREAM
// ============================================================

SoundManager::AudioStream
SoundManager::getCurrentStream() const
{
    return _currentStream;
}

// ============================================================
// DEBUG STATUS
// ============================================================

void SoundManager::printStatus()
{
    Serial0.println();

    Serial0.println(
        "[SOUND] ---------- STATUS ----------"
    );

    Serial0.printf(
        "[SOUND] state:       %s\n",
        getStateString()
    );

    Serial0.printf(
        "[SOUND] path:        %s\n",
        _currentPath.length()
            ? _currentPath.c_str()
            : "-"
    );

    Serial0.printf(
        "[SOUND] stream:      %u\n",
        static_cast<uint8_t>(
            _currentStream
        )
    );

    Serial0.printf(
        "[SOUND] local %%:     %u\n",
        _localPercent
    );

    Serial0.printf(
        "[SOUND] stream vol:  %u\n",
        streamVolume(
            _currentStream
        )
    );

    Serial0.printf(
        "[SOUND] effective:   %u\n",
        getEffectiveVolume()
    );

    Serial0.printf(
        "[SOUND] position:    %lu / %lu ms\n",
        static_cast<unsigned long>(
            getPositionMs()
        ),
        static_cast<unsigned long>(
            getDurationMs()
        )
    );

    Serial0.println(
        "[SOUND] --------------------------------"
    );
}

void SoundManager::setLocalPercent(uint8_t percent)
{
    _localPercent = static_cast<uint8_t>(
        constrain(
            static_cast<int>(percent),
            0,
            100
        )
    );
}
