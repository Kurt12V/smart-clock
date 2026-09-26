#include "SoundManager.h"

#include <cstring>
#include <cmath>

// ============================================================
// CONSTRUCTOR
// ============================================================

SoundManager::SoundManager(
    SDManager& sdManager,
    Settings::Audio& settings,
    I2SManager& i2sManager
)
    : _sdManager(sdManager),
      _settings(settings),
      _i2sManager(i2sManager),
      _initialized(false),
      _state(State::STOPPED),
      _positionBytes(0),
      _durationMs(0),
      _localVolume(100),
      _fadeInEnabled(false),
      _fadeOutEnabled(false),
      _fadeInMs(0),
      _fadeOutMs(0),
      _fadeStartMs(0),
      _fadeStopMs(0),
      _fadeCurve(FadeCurve::Linear),
      _lastStatusMs(0)
{
    memset(&_wav, 0, sizeof(_wav));
}

// ============================================================
// BEGIN
// ============================================================

bool SoundManager::begin()
{
    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("[SOUND] SoundManager::begin()");
    Serial0.println("========================================");

    Serial0.println("[SOUND] Checking SD...");

    if (!_sdManager.isReady())
    {
        Serial0.println("[SOUND] ERROR: SD not ready");
        return false;
    }

    Serial0.println("[SOUND] SD ready");

    Serial0.printf(
        "[SOUND] Enabled: %s\n",
        _settings.enabled ? "YES" : "NO"
    );

    Serial0.printf(
        "[SOUND] Global volume: %u\n",
        _settings.volume
    );

    Serial0.printf(
        "[SOUND] Sample rate setting: %u kHz\n",
        _settings.sampleRate
    );

    Serial0.println("[SOUND] Checking I2SManager...");

    if (!_i2sManager.isInitialized())
    {
        Serial0.println(
            "[SOUND] ERROR: I2SManager not initialized"
        );

        return false;
    }

    Serial0.printf(
        "[SOUND] Speaker port: I2S_NUM_%d\n",
        _i2sManager.speakerPort()
    );

    Serial0.println(
        "[SOUND] Speaker I2S will be configured from WAV sample rate"
    );

    _initialized = true;

    Serial0.println("[SOUND] SoundManager initialized");

    return true;
}

// ============================================================
// END
// ============================================================

void SoundManager::end()
{
    stop();

    _initialized = false;

    Serial0.println("[SOUND] SoundManager stopped");
}

// ============================================================
// UPDATE
// ============================================================

void SoundManager::update()
{
    if (!_initialized)
        return;

    if (_state != State::PLAYING)
        return;

    if (!_file)
    {
        finishPlayback();
        return;
    }

    readAndPlayChunk();

    if (millis() - _lastStatusMs >= 1000)
    {
        _lastStatusMs = millis();

        Serial0.printf(
            "[PLAY] %lu / %lu ms\n",
            static_cast<unsigned long>(getPositionMs()),
            static_cast<unsigned long>(_durationMs)
        );
    }
}

// ============================================================
// PLAY WAV
// ============================================================

bool SoundManager::playWav(const char* path)
{
    Serial0.println();
    Serial0.println("----------------------------------------");
    Serial0.println("[SOUND] playWav()");
    Serial0.println("----------------------------------------");

    Serial0.printf(
        "[SOUND] Path: %s\n",
        path
    );

    return playWavLocal(path, 100);
}

// ============================================================
// PLAY WAV LOCAL
// ============================================================

bool SoundManager::playWavLocal(
    const char* path,
    uint8_t localVolume
)
{
    Serial0.println();
    Serial0.println("----------------------------------------");
    Serial0.println("[SOUND] playWavLocal()");
    Serial0.println("----------------------------------------");

    if (!_initialized)
    {
        Serial0.println(
            "[SOUND] ERROR: SoundManager not initialized"
        );

        return false;
    }

    if (!_settings.enabled)
    {
        Serial0.println(
            "[SOUND] ERROR: audio disabled in settings"
        );

        return false;
    }

    if (path == nullptr)
    {
        Serial0.println(
            "[SOUND] ERROR: path is NULL"
        );

        return false;
    }

    Serial0.printf(
        "[SOUND] File: %s\n",
        path
    );

    Serial0.printf(
        "[SOUND] Local volume: %u\n",
        localVolume
    );

    stop();

    _localVolume = constrain(localVolume, 0, 100);

    // ========================================================
    // STEP 1
    // ========================================================

    Serial0.println("[SOUND] Step 1: opening WAV");

    if (!openWav(path))
    {
        Serial0.println("[SOUND] Step 1 FAILED");
        return false;
    }

    Serial0.println("[SOUND] Step 1 OK");

    // ========================================================
    // STEP 2
    // ========================================================

    Serial0.println("[SOUND] Step 2: validating WAV");

    if (!validateWav())
    {
        Serial0.println("[SOUND] Step 2 FAILED");

        _file.close();

        return false;
    }

    Serial0.println("[SOUND] Step 2 OK");

    // ========================================================
    // WAV INFO
    // ========================================================

    Serial0.println();
    Serial0.println("[SOUND] ========== WAV INFO ==========");

    Serial0.printf(
        "[SOUND] Format:       %u\n",
        _wav.audioFormat
    );

    Serial0.printf(
        "[SOUND] Channels:     %u\n",
        _wav.channels
    );

    Serial0.printf(
        "[SOUND] Sample rate:  %lu Hz\n",
        static_cast<unsigned long>(_wav.sampleRate)
    );

    Serial0.printf(
        "[SOUND] Byte rate:    %lu\n",
        static_cast<unsigned long>(_wav.byteRate)
    );

    Serial0.printf(
        "[SOUND] Block align:  %u\n",
        _wav.blockAlign
    );

    Serial0.printf(
        "[SOUND] Bits/sample:  %u\n",
        _wav.bitsPerSample
    );

    Serial0.printf(
        "[SOUND] Data offset:  %lu\n",
        static_cast<unsigned long>(_wav.dataOffset)
    );

    Serial0.printf(
        "[SOUND] Data size:    %lu\n",
        static_cast<unsigned long>(_wav.dataSize)
    );

    Serial0.printf(
        "[SOUND] Duration:      %lu ms\n",
        static_cast<unsigned long>(_durationMs)
    );

    Serial0.println("[SOUND] =================================");

    // ========================================================
    // STEP 3
    // ========================================================

    Serial0.println("[SOUND] Step 3: configuring speaker I2S");

    Serial0.printf(
        "[SOUND] Configuring speaker for %lu Hz\n",
        static_cast<unsigned long>(_wav.sampleRate)
    );

    if (!_i2sManager.beginSpeaker(_wav.sampleRate))
    {
        Serial0.println(
            "[SOUND] ERROR: speaker configuration failed"
        );

        _file.close();

        return false;
    }

    Serial0.printf(
        "[SOUND] Speaker I2S configured: %lu Hz\n",
        static_cast<unsigned long>(_wav.sampleRate)
    );

    Serial0.println("[SOUND] Step 3 OK");

    // ========================================================
    // STEP 4
    // ========================================================

    Serial0.printf(
        "[SOUND] Step 4: seeking to data offset %lu\n",
        static_cast<unsigned long>(_wav.dataOffset)
    );

    if (!_file.seek(_wav.dataOffset))
    {
        Serial0.println("[SOUND] Step 4 FAILED");

        _file.close();

        return false;
    }

    _positionBytes = 0;

    Serial0.println("[SOUND] Step 4 OK");

    // ========================================================
    // START I2S
    // ========================================================

    Serial0.printf(
        "[SOUND] Starting I2S port I2S_NUM_%d\n",
        _i2sManager.speakerPort()
    );

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println(
            "[SOUND] ERROR: startSpeaker() failed"
        );

        _file.close();

        return false;
    }

    _state = State::PLAYING;

    _lastStatusMs = millis();

    Serial0.println(
        "[SOUND] Playback started successfully"
    );

    Serial0.printf(
        "[SOUND] Effective volume: %u\n",
        getEffectiveVolume()
    );

    return true;
}

// ============================================================
// OPEN WAV
// ============================================================

bool SoundManager::openWav(const char* path)
{
    Serial0.printf(
        "[SOUND] openWav(): %s\n",
        path
    );

    Serial0.println("[SOUND] Opening file...");

    _file =
        _sdManager.card().fs().open(
            path,
            FILE_READ
        );

    if (!_file)
    {
        Serial0.println(
            "[SOUND] ERROR: failed to open file"
        );

        return false;
    }

    Serial0.printf(
        "[SOUND] File opened, size: %lu bytes\n",
        static_cast<unsigned long>(_file.size())
    );

    Serial0.println("[SOUND] Parsing WAV header...");

    if (!parseWav(_file))
    {
        Serial0.println(
            "[SOUND] ERROR: WAV parsing failed"
        );

        _file.close();

        return false;
    }

    return true;
}

// ============================================================
// PARSE WAV
// ============================================================

bool SoundManager::parseWav(File& file)
{
    uint8_t header[12];

    if (file.read(header, sizeof(header)) != sizeof(header))
    {
        Serial0.println(
            "[SOUND] ERROR: cannot read RIFF header"
        );

        return false;
    }

    if (memcmp(header, "RIFF", 4) != 0)
    {
        Serial0.println(
            "[SOUND] ERROR: missing RIFF"
        );

        return false;
    }

    if (memcmp(header + 8, "WAVE", 4) != 0)
    {
        Serial0.println(
            "[SOUND] ERROR: missing WAVE"
        );

        return false;
    }

    Serial0.println("[SOUND] RIFF: RIFF");
    Serial0.println("[SOUND] Format: WAVE");

    bool foundFmt = false;
    bool foundData = false;

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

        chunkId[4] = '\0';

        uint32_t chunkSize =
            static_cast<uint32_t>(chunkHeader[4]) |
            (static_cast<uint32_t>(chunkHeader[5]) << 8) |
            (static_cast<uint32_t>(chunkHeader[6]) << 16) |
            (static_cast<uint32_t>(chunkHeader[7]) << 24);

        uint32_t chunkDataOffset =
            static_cast<uint32_t>(file.position());

        Serial0.printf(
            "[SOUND] Chunk '%s', size=%lu, offset=%lu\n",
            chunkId,
            static_cast<unsigned long>(chunkSize),
            static_cast<unsigned long>(chunkDataOffset)
        );

        // ====================================================
        // FMT
        // ====================================================

        if (memcmp(chunkId, "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
            {
                Serial0.println(
                    "[SOUND] ERROR: invalid fmt chunk"
                );

                return false;
            }

            uint8_t fmt[16];

            if (file.read(fmt, 16) != 16)
            {
                return false;
            }

            _wav.audioFormat =
                static_cast<uint16_t>(fmt[0]) |
                (static_cast<uint16_t>(fmt[1]) << 8);

            _wav.channels =
                static_cast<uint16_t>(fmt[2]) |
                (static_cast<uint16_t>(fmt[3]) << 8);

            _wav.sampleRate =
                static_cast<uint32_t>(fmt[4]) |
                (static_cast<uint32_t>(fmt[5]) << 8) |
                (static_cast<uint32_t>(fmt[6]) << 16) |
                (static_cast<uint32_t>(fmt[7]) << 24);

            _wav.byteRate =
                static_cast<uint32_t>(fmt[8]) |
                (static_cast<uint32_t>(fmt[9]) << 8) |
                (static_cast<uint32_t>(fmt[10]) << 16) |
                (static_cast<uint32_t>(fmt[11]) << 24);

            _wav.blockAlign =
                static_cast<uint16_t>(fmt[12]) |
                (static_cast<uint16_t>(fmt[13]) << 8);

            _wav.bitsPerSample =
                static_cast<uint16_t>(fmt[14]) |
                (static_cast<uint16_t>(fmt[15]) << 8);

            Serial0.println("[SOUND] fmt parsed:");

            Serial0.printf(
                "         format       = %u\n",
                _wav.audioFormat
            );

            Serial0.printf(
                "         channels     = %u\n",
                _wav.channels
            );

            Serial0.printf(
                "         sample rate  = %lu\n",
                static_cast<unsigned long>(_wav.sampleRate)
            );

            Serial0.printf(
                "         byte rate    = %lu\n",
                static_cast<unsigned long>(_wav.byteRate)
            );

            Serial0.printf(
                "         block align  = %u\n",
                _wav.blockAlign
            );

            Serial0.printf(
                "         bits/sample  = %u\n",
                _wav.bitsPerSample
            );

            // Jump to end of fmt chunk.
            file.seek(
                chunkDataOffset + chunkSize
            );

            foundFmt = true;
        }

        // ====================================================
        // DATA
        // ====================================================

        else if (memcmp(chunkId, "data", 4) == 0)
        {
            _wav.dataOffset =
                chunkDataOffset;

            _wav.dataSize =
                chunkSize;

            Serial0.printf(
                "[SOUND] PCM data offset: %lu\n",
                static_cast<unsigned long>(
                    _wav.dataOffset
                )
            );

            Serial0.printf(
                "[SOUND] PCM data size: %lu\n",
                static_cast<unsigned long>(
                    _wav.dataSize
                )
            );

            foundData = true;

            break;
        }

        // Unknown chunk.
        else
        {
            uint32_t next =
                chunkDataOffset + chunkSize;

            // WAV chunks are word aligned.
            if (chunkSize & 1)
                next++;

            file.seek(next);
        }
    }

    if (!foundFmt)
    {
        Serial0.println(
            "[SOUND] ERROR: fmt chunk not found"
        );

        return false;
    }

    if (!foundData)
    {
        Serial0.println(
            "[SOUND] ERROR: data chunk not found"
        );

        return false;
    }

    if (_wav.byteRate == 0)
    {
        return false;
    }

    _durationMs =
        static_cast<uint32_t>(
            (
                static_cast<uint64_t>(_wav.dataSize) *
                1000ULL
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
    if (_wav.audioFormat != 1)
    {
        Serial0.printf(
            "[SOUND] ERROR: unsupported audio format: %u\n",
            _wav.audioFormat
        );

        return false;
    }

    if (_wav.channels != 1 &&
        _wav.channels != 2)
    {
        Serial0.printf(
            "[SOUND] ERROR: unsupported channels: %u\n",
            _wav.channels
        );

        return false;
    }

    if (_wav.sampleRate == 0)
    {
        Serial0.println(
            "[SOUND] ERROR: invalid sample rate"
        );

        return false;
    }

    if (_wav.bitsPerSample != 16)
    {
        Serial0.printf(
            "[SOUND] ERROR: only 16-bit PCM supported, got %u\n",
            _wav.bitsPerSample
        );

        return false;
    }

    if (_wav.blockAlign == 0)
    {
        return false;
    }

    if (_wav.dataSize == 0)
    {
        Serial0.println(
            "[SOUND] ERROR: empty PCM data"
        );

        return false;
    }

    return true;
}

// ============================================================
// READ + PLAY CHUNK
// ============================================================

bool SoundManager::readAndPlayChunk()
{
    if (!_file)
        return false;

    uint32_t remaining =
        _wav.dataSize - _positionBytes;

    if (remaining == 0)
    {
        finishPlayback();
        return true;
    }

    size_t bytesToRead =
        min(
            static_cast<size_t>(remaining),
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
        static_cast<uint32_t>(bytesRead);

    size_t outputSamples = 0;

    // ========================================================
    // MONO
    // ========================================================

    if (_wav.channels == 1)
    {
        size_t sampleCount =
            bytesRead / 2;

        memcpy(
            _outputBuffer,
            _inputBuffer,
            sampleCount * sizeof(int16_t)
        );

        outputSamples = sampleCount;
    }

    // ========================================================
    // STEREO -> MONO
    // ========================================================

    else
    {
        const int16_t* stereo =
            reinterpret_cast<const int16_t*>(
                _inputBuffer
            );

        size_t stereoSamples =
            bytesRead / 2;

        size_t frames =
            stereoSamples / 2;

        for (size_t i = 0; i < frames; ++i)
        {
            int32_t left =
                stereo[i * 2];

            int32_t right =
                stereo[i * 2 + 1];

            int32_t mixed =
                (left + right) / 2;

            mixed =
                constrain(
                    mixed,
                    -32768,
                    32767
                );

            _outputBuffer[i] =
                static_cast<int16_t>(mixed);
        }

        outputSamples = frames;
    }

    // ========================================================
    // VOLUME
    // ========================================================

    uint8_t volume =
        getEffectiveVolume();

    applyVolume(
        _outputBuffer,
        outputSamples,
        volume
    );

    // ========================================================
    // WRITE
    // ========================================================

    size_t outputBytes =
        outputSamples * sizeof(int16_t);

    size_t bytesWritten = 0;

    if (!_i2sManager.writeSpeaker(
            reinterpret_cast<const uint8_t*>(
                _outputBuffer
            ),
            outputBytes,
            bytesWritten,
            100
        ))
    {
        Serial0.printf(
            "[SOUND] writeSpeaker() failed, requested=%u written=%u\n",
            static_cast<unsigned int>(outputBytes),
            static_cast<unsigned int>(bytesWritten)
        );

        finishPlayback();

        return false;
    }

    // ВАЖНО:
    // Позиция WAV должна учитывать именно
    // количество байт исходного файла.
    _positionBytes += sourceBytesRead;

    return true;
}

// ============================================================
// VOLUME
// ============================================================

void SoundManager::applyVolume(
    int16_t* samples,
    size_t sampleCount,
    uint8_t volume
)
{
    if (samples == nullptr)
        return;

    if (volume >= 100)
        return;

    if (volume == 0)
    {
        memset(
            samples,
            0,
            sampleCount * sizeof(int16_t)
        );

        return;
    }

    float multiplier =
        static_cast<float>(volume) / 100.0f;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        int32_t value =
            static_cast<int32_t>(
                static_cast<float>(
                    samples[i]
                ) * multiplier
            );

        value =
            constrain(
                value,
                -32768,
                32767
            );

        samples[i] =
            static_cast<int16_t>(value);
    }
}

// ============================================================
// EFFECTIVE VOLUME
// ============================================================

uint8_t SoundManager::calculateEffectiveVolume() const
{
    uint8_t global =
        constrain(
            _settings.volume,
            0,
            100
        );

    uint8_t local =
        constrain(
            _localVolume,
            0,
            100
        );

    return static_cast<uint8_t>(
        (
            static_cast<uint16_t>(global) *
            static_cast<uint16_t>(local)
        ) / 100
    );
}

uint8_t SoundManager::getEffectiveVolume() const
{
    uint8_t volume =
        calculateEffectiveVolume();

    float fade =
        calculateFadeMultiplier();

    float result =
        static_cast<float>(volume) *
        fade;

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
// GLOBAL VOLUME
// ============================================================

void SoundManager::setGlobalVolume(uint8_t volume)
{
    _settings.volume =
        constrain(volume, 0, 100);
}

uint8_t SoundManager::getGlobalVolume() const
{
    return _settings.volume;
}

// ============================================================
// LOCAL VOLUME
// ============================================================

void SoundManager::setLocalVolume(uint8_t volume)
{
    _localVolume =
        constrain(volume, 0, 100);
}

uint8_t SoundManager::getLocalVolume() const
{
    return _localVolume;
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

    _state = State::PAUSED;

    Serial0.println("[SOUND] PAUSED");

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

    _state = State::PLAYING;

    Serial0.println("[SOUND] RESUMED");

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

        return;
    }

    Serial0.println("[SOUND] STOP");

    _i2sManager.stopSpeaker();
    _i2sManager.clearSpeaker();

    if (_file)
        _file.close();

    _state = State::STOPPED;

    _positionBytes = 0;
}

// ============================================================
// FINISH
// ============================================================

void SoundManager::finishPlayback()
{
    Serial0.println("[SOUND] finishPlayback()");

    _i2sManager.stopSpeaker();
    _i2sManager.clearSpeaker();

    if (_file)
        _file.close();

    _state = State::STOPPED;

    _positionBytes = _wav.dataSize;
}

// ============================================================
// ALARM
// ============================================================

bool SoundManager::playAlarm(
    const char* path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    _fadeInEnabled =
        fadeInMs > 0;

    _fadeOutEnabled =
        fadeOutMs > 0;

    _fadeInMs =
        fadeInMs;

    _fadeOutMs =
        fadeOutMs;

    _fadeCurve =
        curve;

    _fadeStartMs =
        millis();

    _fadeStopMs =
        0;

    // Для alarm localVolume является
    // непосредственной громкостью.
    //
    // Поэтому здесь глобальная громкость
    // НЕ должна ограничивать alarm.
    _localVolume =
        constrain(localVolume, 0, 100);

    return playWavLocal(
        path,
        100
    );
}

// ============================================================
// FADE MULTIPLIER
// ============================================================

float SoundManager::calculateFadeMultiplier() const
{
    if (_state != State::PLAYING)
        return 1.0f;

    uint32_t now =
        millis();

    // ========================================================
    // FADE IN
    // ========================================================

    if (_fadeInEnabled &&
        _fadeInMs > 0)
    {
        uint32_t elapsed =
            now - _fadeStartMs;

        if (elapsed < _fadeInMs)
        {
            float value =
                static_cast<float>(elapsed) /
                static_cast<float>(_fadeInMs);

            return applyFadeCurve(value);
        }
    }

    // ========================================================
    // FADE OUT
    // ========================================================

    if (_fadeOutEnabled &&
        _fadeOutMs > 0 &&
        _durationMs > 0)
    {
        uint32_t position =
            getPositionMs();

        if (position >=
            _durationMs - min(
                _durationMs,
                _fadeOutMs
            ))
        {
            uint32_t start =
                _durationMs -
                min(
                    _durationMs,
                    _fadeOutMs
                );

            uint32_t elapsed =
                position - start;

            float value =
                1.0f -
                (
                    static_cast<float>(elapsed) /
                    static_cast<float>(_fadeOutMs)
                );

            value =
                constrain(
                    value,
                    0.0f,
                    1.0f
                );

            return applyFadeCurve(value);
        }
    }

    return 1.0f;
}

// ============================================================
// FADE CURVE
// ============================================================

float SoundManager::applyFadeCurve(float value) const
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
            static_cast<uint64_t>(_positionBytes) *
            1000ULL
        ) /
        _wav.byteRate
    );
}

uint32_t SoundManager::getDurationMs() const
{
    return _durationMs;
}

uint32_t SoundManager::getPositionBytes() const
{
    return _positionBytes;
}

uint32_t SoundManager::getDataSize() const
{
    return _wav.dataSize;
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
    return _state == State::PLAYING;
}

bool SoundManager::isPaused() const
{
    return _state == State::PAUSED;
}

bool SoundManager::isActive() const
{
    return _state != State::STOPPED;
}

SoundManager::State SoundManager::getState() const
{
    return _state;
}

// ============================================================
// DEBUG
// ============================================================

void SoundManager::printStatus()
{
    Serial0.println();
    Serial0.println("[SOUND] ---------- STATUS ----------");

    Serial0.printf(
        "[SOUND] Initialized: %s\n",
        _initialized ? "YES" : "NO"
    );

    Serial0.printf(
        "[SOUND] Playing: %s\n",
        isPlaying() ? "YES" : "NO"
    );

    Serial0.printf(
        "[SOUND] Paused: %s\n",
        isPaused() ? "YES" : "NO"
    );

    Serial0.printf(
        "[SOUND] Local volume: %u\n",
        _localVolume
    );

    Serial0.printf(
        "[SOUND] Global volume: %u\n",
        _settings.volume
    );

    Serial0.printf(
        "[SOUND] Effective volume: %u\n",
        getEffectiveVolume()
    );

    Serial0.printf(
        "[SOUND] Position: %lu / %lu ms\n",
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
