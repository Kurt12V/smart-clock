#include "SoundManager.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    constexpr uint32_t AUDIO_DEBUG_INTERVAL_MS = 1000;
    constexpr uint32_t I2S_WRITE_TIMEOUT_MS = 100;

    constexpr uint16_t WAV_PCM_FORMAT = 1;
    constexpr uint16_t WAV_BITS_PER_SAMPLE = 16;

    uint8_t clampPercent(uint8_t value)
    {
        return value > 100 ? 100 : value;
    }

    float clampFloat(float value, float minimum, float maximum)
    {
        return std::max(minimum, std::min(value, maximum));
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

SoundManager::SoundManager(
    SettingsManager& settings,
    I2SManager& i2sManager
)
    : _settings(settings)
    , _i2sManager(i2sManager)
{
}

// ============================================================
// BEGIN / END
// ============================================================

bool SoundManager::begin()
{
    if (_initialized)
        return true;

    Serial0.println();
    Serial0.println("[SoundManager] Initializing");

    // LittleFS нужен только для локальных WAV-файлов.
    // Ошибка монтирования не блокирует PCM-воспроизведение.

    _littleFSReady = LittleFS.begin(false);

    if (_littleFSReady)
    {
        Serial0.println("[SoundManager] LittleFS ready");
    }
    else
    {
        Serial0.println(
            "[SoundManager][WARNING] LittleFS unavailable; "
            "PCM playback remains available"
        );
    }

    _state = State::STOPPED;
    _sourceMode = SourceMode::None;

    _localPercent = 100;

    resetPcmQueue();
    resetPlaybackState();

    _initialized = true;

    Serial0.println("[SoundManager] Initialized");

    return true;
}

void SoundManager::end()
{
    if (!_initialized)
        return;

    stop();

    _initialized = false;

    Serial0.println("[SoundManager] Ended");
}

void SoundManager::update()
{
    if (!_initialized)
        return;

    if (_state == State::STOPPED ||
        _state == State::PAUSED)
    {
        return;
    }

    if (_state == State::FADING_OUT && isFadeOutComplete())
    {
        finishPlayback();
        return;
    }

    readAndPlayChunk();
}

bool SoundManager::isLittleFSReady() const
{
    return _littleFSReady;
}

// ============================================================
// WAV PLAYBACK FROM LITTLEFS
// ============================================================

bool SoundManager::play(const char* path)
{
    PlayOptions options;
    return play(path, options);
}

bool SoundManager::play(
    const char* path,
    const PlayOptions& options
)
{
    if (!_initialized ||
        path == nullptr ||
        path[0] == '\0')
    {
        return false;
    }

    stop();

    if (!openWav(path))
    {
        Serial0.printf(
            "[SoundManager][ERROR] Cannot open WAV: %s\n",
            path
        );

        return false;
    }

    _currentStream = options.stream;
    _localPercent = clampPercent(options.localPercent);

    _fadeCurve = options.curve;

    _fadeInMs = options.fadeInMs;
    _fadeOutMs = options.fadeOutMs;

    _fadeStartMs = millis();

    _fadeInEnabled = _fadeInMs > 0;
    _fadeOutEnabled = false;

    _stopFadeActive = false;
    _stopFadeDurationMs = 0;

    if (!startPcmOutput(_wav.sampleRate))
    {
        _file.close();
        resetPlaybackState();
        return false;
    }

    _sourceMode = SourceMode::Wav;
    _state = State::PLAYING;

    _pcmSampleRate = _wav.sampleRate;

    _positionBytes = 0;

    Serial0.printf(
        "[SoundManager] WAV started: %s, %lu Hz, %u channel(s)\n",
        path,
        static_cast<unsigned long>(_wav.sampleRate),
        static_cast<unsigned>(_wav.channels)
    );

    return true;
}

bool SoundManager::openWav(const char* path)
{
    if (!_littleFSReady)
    {
        Serial0.println(
            "[SoundManager][ERROR] LittleFS is not mounted"
        );

        return false;
    }

    if (_file)
        _file.close();

    _currentPath = "";
    _positionBytes = 0;
    _durationMs = 0;

    _file = LittleFS.open(path, FILE_READ);

    if (!_file)
    {
        Serial0.printf(
            "[SoundManager][ERROR] File not found: %s\n",
            path
        );

        return false;
    }

    if (!parseWav(_file))
    {
        Serial0.println(
            "[SoundManager][ERROR] Invalid WAV structure"
        );

        _file.close();
        return false;
    }

    if (!validateWav())
    {
        Serial0.println(
            "[SoundManager][ERROR] Unsupported WAV format"
        );

        _file.close();
        return false;
    }

    if (!_file.seek(_wav.dataOffset))
    {
        Serial0.println(
            "[SoundManager][ERROR] Cannot seek to WAV data"
        );

        _file.close();
        return false;
    }

    if (_wav.byteRate == 0)
    {
        _file.close();
        return false;
    }

    _currentPath = path;

    _positionBytes = 0;

    _durationMs = static_cast<uint32_t>(
        (static_cast<uint64_t>(_wav.dataSize) * 1000ULL)
        / _wav.byteRate
    );

    Serial0.printf(
        "[SoundManager] WAV opened: %s, %lu bytes\n",
        path,
        static_cast<unsigned long>(_wav.dataSize)
    );

    return true;
}

// ============================================================
// WAV PARSER
// ============================================================

uint16_t SoundManager::readLE16(const uint8_t* data)
{
    return static_cast<uint16_t>(
        static_cast<uint16_t>(data[0]) |
        (static_cast<uint16_t>(data[1]) << 8)
    );
}

uint32_t SoundManager::readLE32(const uint8_t* data)
{
    return
        static_cast<uint32_t>(data[0]) |
        (static_cast<uint32_t>(data[1]) << 8) |
        (static_cast<uint32_t>(data[2]) << 16) |
        (static_cast<uint32_t>(data[3]) << 24);
}

bool SoundManager::parseWav(File& file)
{
    _wav = WavInfo{};

    if (!file || file.size() < 44)
        return false;

    uint8_t header[12];

    if (file.read(header, sizeof(header)) != sizeof(header))
        return false;

    if (std::memcmp(header, "RIFF", 4) != 0 ||
        std::memcmp(header + 8, "WAVE", 4) != 0)
    {
        return false;
    }

    bool foundFormat = false;
    bool foundData = false;

    while (file.available())
    {
        uint8_t chunkHeader[8];

        if (file.read(chunkHeader, sizeof(chunkHeader)) !=
            sizeof(chunkHeader))
        {
            break;
        }

        const uint32_t chunkSize =
            readLE32(chunkHeader + 4);

        const uint32_t chunkDataOffset =
            file.position();

        if (std::memcmp(chunkHeader, "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
                return false;

            uint8_t formatData[16];

            if (file.read(formatData, sizeof(formatData)) !=
                sizeof(formatData))
            {
                return false;
            }

            _wav.audioFormat = readLE16(formatData);
            _wav.channels = readLE16(formatData + 2);
            _wav.sampleRate = readLE32(formatData + 4);
            _wav.byteRate = readLE32(formatData + 8);
            _wav.blockAlign = readLE16(formatData + 12);
            _wav.bitsPerSample = readLE16(formatData + 14);

            foundFormat = true;
        }
        else if (std::memcmp(chunkHeader, "data", 4) == 0)
        {
            _wav.dataOffset = chunkDataOffset;
            _wav.dataSize = chunkSize;

            foundData = true;
        }

        const uint64_t nextChunk =
            static_cast<uint64_t>(chunkDataOffset) +
            chunkSize +
            (chunkSize & 1U);

        if (nextChunk > file.size())
            return false;

        if (!file.seek(static_cast<uint32_t>(nextChunk)))
            return false;

        if (foundFormat && foundData)
            break;
    }

    return foundFormat && foundData;
}

bool SoundManager::validateWav() const
{
    if (_wav.audioFormat != WAV_PCM_FORMAT)
        return false;

    if (_wav.bitsPerSample != WAV_BITS_PER_SAMPLE)
        return false;

    if (_wav.channels != 1 && _wav.channels != 2)
        return false;

    if (_wav.sampleRate == 0 || _wav.byteRate == 0)
        return false;

    if (_wav.blockAlign != _wav.channels * sizeof(int16_t))
        return false;

    return _wav.dataSize > 0;
}

// ============================================================
// GENERATED PCM SOURCE
// ============================================================

bool SoundManager::playSource(
    PcmSourceCallback source,
    void* context,
    uint32_t sampleRate,
    AudioStream stream,
    FinishedCallback finished
)
{
    if (!_initialized ||
        source == nullptr ||
        sampleRate == 0)
    {
        return false;
    }

    stop();

    if (!startPcmOutput(sampleRate))
        return false;

    _sourceMode = SourceMode::CallbackPCM;

    _pcmSource = source;
    _pcmContext = context;
    _finishedCallback = finished;

    _pcmSampleRate = sampleRate;

    _currentStream = stream;
    _currentPath = "";

    _positionBytes = 0;
    _durationMs = 0;

    _localPercent = 100;

    _fadeInEnabled = false;
    _fadeOutEnabled = false;
    _stopFadeActive = false;

    _state = State::PLAYING;

    _lastStatusMs = millis();
    _audioBlocksWritten = 0;

    Serial0.printf(
        "[SoundManager] PCM source started: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    return true;
}

// ============================================================
// BUFFERED PCM
// ============================================================

bool SoundManager::beginPCM(
    uint32_t sampleRate,
    AudioStream stream
)
{
    if (!_initialized || sampleRate == 0)
        return false;

    stop();

    if (!startPcmOutput(sampleRate))
        return false;

    resetPcmQueue();

    _sourceMode = SourceMode::BufferedPCM;
    _pcmSampleRate = sampleRate;

    _currentStream = stream;
    _currentPath = "";

    _positionBytes = 0;
    _durationMs = 0;

    _localPercent = 100;

    _fadeInEnabled = false;
    _fadeOutEnabled = false;
    _stopFadeActive = false;

    _state = State::PLAYING;

    _lastStatusMs = millis();
    _audioBlocksWritten = 0;

    return true;
}

bool SoundManager::submitPCM(
    const int16_t* samples,
    size_t sampleCount
)
{
    if (!_initialized ||
        _sourceMode != SourceMode::BufferedPCM ||
        samples == nullptr ||
        sampleCount == 0)
    {
        return false;
    }

    if (sampleCount > PCM_QUEUE_CAPACITY - _pcmCount)
        return false;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        _pcmQueue[_pcmHead] = samples[i];

        _pcmHead =
            (_pcmHead + 1) % PCM_QUEUE_CAPACITY;
    }

    _pcmCount += sampleCount;

    return true;
}

size_t SoundManager::availablePCM() const
{
    return PCM_QUEUE_CAPACITY - _pcmCount;
}

void SoundManager::resetPcmQueue()
{
    _pcmHead = 0;
    _pcmTail = 0;
    _pcmCount = 0;

    std::memset(
        _pcmQueue,
        0,
        sizeof(_pcmQueue)
    );
}

// ============================================================
// I2S
// ============================================================

bool SoundManager::startPcmOutput(uint32_t sampleRate)
{
    if (sampleRate == 0)
        return false;

    _i2sManager.stopSpeaker();

    if (!_i2sManager.beginSpeaker(sampleRate))
    {
        Serial0.println(
            "[SoundManager][ERROR] I2S begin failed"
        );

        return false;
    }

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println(
            "[SoundManager][ERROR] I2S start failed"
        );

        return false;
    }

    return true;
}

// ============================================================
// UPDATE AUDIO SOURCE
// ============================================================

bool SoundManager::readAndPlayChunk()
{
    switch (_sourceMode)
    {
        case SourceMode::Wav:
            return readWavChunk();

        case SourceMode::CallbackPCM:
        case SourceMode::BufferedPCM:
            return readPcmChunk();

        case SourceMode::None:
        default:
            return false;
    }
}

bool SoundManager::readWavChunk()
{
    if (!_file)
    {
        finishPlayback();
        return false;
    }

    const uint32_t remaining =
        _wav.dataSize > _positionBytes
            ? _wav.dataSize - _positionBytes
            : 0;

    if (remaining == 0)
    {
        finishPlayback();
        return false;
    }

    size_t bytesToRead =
        std::min<size_t>(BUFFER_BYTES, remaining);

    bytesToRead -= bytesToRead % _wav.blockAlign;

    if (bytesToRead == 0)
    {
        finishPlayback();
        return false;
    }

    const size_t bytesRead =
        _file.read(_inputBuffer, bytesToRead);

    if (bytesRead == 0)
    {
        finishPlayback();
        return false;
    }

    const size_t frames =
        bytesRead / _wav.blockAlign;

    if (_wav.channels == 1)
    {
        const size_t samples =
            std::min(frames, BUFFER_SAMPLES);

        std::memcpy(
            _outputBuffer,
            _inputBuffer,
            samples * sizeof(int16_t)
        );

        _positionBytes += static_cast<uint32_t>(bytesRead);

        return writeSamples(_outputBuffer, samples);
    }

    const int16_t* stereo =
        reinterpret_cast<const int16_t*>(_inputBuffer);

    for (size_t i = 0; i < frames; ++i)
    {
        const int32_t left = stereo[i * 2];
        const int32_t right = stereo[i * 2 + 1];

        _outputBuffer[i] =
            static_cast<int16_t>((left + right) / 2);
    }

    _positionBytes += static_cast<uint32_t>(bytesRead);

    return writeSamples(_outputBuffer, frames);
}

bool SoundManager::readPcmChunk()
{
    if (_sourceMode == SourceMode::CallbackPCM)
    {
        if (_pcmSource == nullptr)
        {
            finishPlayback();
            return false;
        }

        size_t count = _pcmSource(
            _pcmContext,
            _outputBuffer,
            BUFFER_SAMPLES
        );

        // Для callback ноль означает завершение источника.
        if (count == 0)
        {
            finishPlayback();
            return false;
        }

        if (count > BUFFER_SAMPLES)
            count = BUFFER_SAMPLES;

        if (count < BUFFER_SAMPLES)
        {
            std::memset(
                _outputBuffer + count,
                0,
                (BUFFER_SAMPLES - count) * sizeof(int16_t)
            );
        }

        _positionBytes +=
            static_cast<uint32_t>(count * sizeof(int16_t));

        // Дополняем блок тишиной, если callback вернул
        // меньше BUFFER_SAMPLES.
        return writeSamples(_outputBuffer, BUFFER_SAMPLES);
    }

    if (_sourceMode == SourceMode::BufferedPCM)
    {
        const size_t count =
            std::min(_pcmCount, BUFFER_SAMPLES);

        for (size_t i = 0; i < count; ++i)
        {
            _outputBuffer[i] = _pcmQueue[_pcmTail];

            _pcmTail =
                (_pcmTail + 1) % PCM_QUEUE_CAPACITY;
        }

        _pcmCount -= count;

        // Пустая очередь означает временное отсутствие данных,
        // а не завершение воспроизведения.
        if (count < BUFFER_SAMPLES)
        {
            std::memset(
                _outputBuffer + count,
                0,
                (BUFFER_SAMPLES - count) * sizeof(int16_t)
            );
        }

        _positionBytes +=
            static_cast<uint32_t>(
                BUFFER_SAMPLES * sizeof(int16_t)
            );

        return writeSamples(_outputBuffer, BUFFER_SAMPLES);
    }

    return false;
}

// ============================================================
// WRITE PCM SAMPLES
// ============================================================

bool SoundManager::writeSamples(
    int16_t* samples,
    size_t sampleCount
)
{
    if (samples == nullptr || sampleCount == 0)
        return false;

    applyVolume(
        samples,
        sampleCount,
        getEffectiveVolume()
    );

    size_t bytesWritten = 0;

    const size_t bytesToWrite =
        sampleCount * sizeof(int16_t);

    const bool ok = _i2sManager.writeSpeaker(
        reinterpret_cast<const uint8_t*>(samples),
        bytesToWrite,
        bytesWritten,
        I2S_WRITE_TIMEOUT_MS
    );

    ++_audioBlocksWritten;

    if (!ok || bytesWritten != bytesToWrite)
    {
        Serial0.printf(
            "[AUDIO][ERROR] Block %lu: wrote %u/%u bytes\n",
            static_cast<unsigned long>(_audioBlocksWritten),
            static_cast<unsigned>(bytesWritten),
            static_cast<unsigned>(bytesToWrite)
        );

        return false;
    }

    const uint32_t now = millis();

    if (now - _lastStatusMs >= AUDIO_DEBUG_INTERVAL_MS)
    {
        _lastStatusMs = now;

        Serial0.printf(
            "[AUDIO] Blocks=%lu Volume=%u%% Bytes=%u\n",
            static_cast<unsigned long>(_audioBlocksWritten),
            static_cast<unsigned>(getEffectiveVolume()),
            static_cast<unsigned>(bytesWritten)
        );
    }

    return true;
}

void SoundManager::applyVolume(
    int16_t* samples,
    size_t sampleCount,
    uint8_t volume
)
{
    volume = clampPercent(volume);

    if (volume >= 100)
        return;

    if (volume == 0)
    {
        std::memset(
            samples,
            0,
            sampleCount * sizeof(int16_t)
        );

        return;
    }

    const float gain =
        static_cast<float>(volume) / 100.0f;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        const float scaled =
            static_cast<float>(samples[i]) * gain;

        samples[i] = static_cast<int16_t>(
            clampFloat(
                std::round(scaled),
                -32768.0f,
                32767.0f
            )
        );
    }
}

// ============================================================
// VOLUME
// ============================================================

uint8_t SoundManager::calculateBaseVolume() const
{
    const uint16_t stream =
        streamVolume(_currentStream);

    const uint16_t local =
        clampPercent(_localPercent);

    return static_cast<uint8_t>(
        (stream * local + 50U) / 100U
    );
}

float SoundManager::calculateFadeMultiplier() const
{
    const uint32_t now = millis();

    if (_fadeInEnabled && _fadeInMs > 0)
    {
        const uint32_t elapsed = now - _fadeStartMs;

        if (elapsed < _fadeInMs)
        {
            const float progress =
                static_cast<float>(elapsed) /
                static_cast<float>(_fadeInMs);

            return applyFadeCurve(progress);
        }
    }

    if (_fadeOutEnabled && _fadeOutMs > 0)
    {
        const uint32_t elapsed = now - _fadeStartMs;

        if (elapsed < _fadeOutMs)
        {
            const float progress =
                1.0f -
                static_cast<float>(elapsed) /
                static_cast<float>(_fadeOutMs);

            return applyFadeCurve(progress);
        }

        return 0.0f;
    }

    return 1.0f;
}

float SoundManager::applyFadeCurve(float value) const
{
    value = clampFloat(value, 0.0f, 1.0f);

    switch (_fadeCurve)
    {
        case FadeCurve::Linear:
            return value;

        case FadeCurve::Exponential:
            return value * value;

        case FadeCurve::Logarithmic:
            return 1.0f - ((1.0f - value) * (1.0f - value));

        default:
            return value;
    }
}

uint8_t SoundManager::getEffectiveVolume() const
{
    float result =
        static_cast<float>(calculateBaseVolume()) *
        calculateFadeMultiplier();

    if (_stopFadeActive && _stopFadeDurationMs > 0)
    {
        const uint32_t elapsed =
            millis() - _stopFadeStartMs;

        if (elapsed >= _stopFadeDurationMs)
        {
            result = 0.0f;
        }
        else
        {
            const float progress =
                1.0f -
                static_cast<float>(elapsed) /
                static_cast<float>(_stopFadeDurationMs);

            result *= progress;
        }
    }

    return static_cast<uint8_t>(
        std::lround(clampFloat(result, 0.0f, 100.0f))
    );
}

// ============================================================
// PAUSE / RESUME / STOP
// ============================================================

bool SoundManager::pause()
{
    if (_state != State::PLAYING &&
        _state != State::FADING_OUT)
    {
        return false;
    }

    _i2sManager.stopSpeaker();

    _state = State::PAUSED;

    return true;
}

bool SoundManager::resume()
{
    if (_state != State::PAUSED)
        return false;

    uint32_t sampleRate = _pcmSampleRate;

    if (_sourceMode == SourceMode::Wav)
        sampleRate = _wav.sampleRate;

    if (sampleRate > 0 && !startPcmOutput(sampleRate))
        return false;

    _state = State::PLAYING;

    return true;
}

void SoundManager::stop()
{
    if (_initialized)
    {
        _i2sManager.stopSpeaker();
        _i2sManager.clearSpeaker();
    }

    if (_file)
        _file.close();

    finishPlayback(false);
}

void SoundManager::stop(uint32_t fadeOutMs)
{
    if (!isActive())
        return;

    if (fadeOutMs == 0)
    {
        stop();
        return;
    }

    _stopFadeActive = true;
    _stopFadeStartMs = millis();
    _stopFadeDurationMs = fadeOutMs;

    _state = State::FADING_OUT;
}

bool SoundManager::isFadeOutComplete() const
{
    if (_state != State::FADING_OUT)
        return false;

    if (_stopFadeActive)
    {
        return millis() - _stopFadeStartMs >=
               _stopFadeDurationMs;
    }

    if (_fadeOutEnabled)
    {
        return millis() - _fadeStartMs >= _fadeOutMs;
    }

    return true;
}

void SoundManager::finishPlayback(bool notify)
{
    FinishedCallback callback = _finishedCallback;
    void* context = _pcmContext;

    if (_initialized)
    {
        _i2sManager.stopSpeaker();
        _i2sManager.clearSpeaker();
    }

    if (_file)
        _file.close();

    resetPlaybackState();

    if (notify && callback != nullptr)
        callback(context);
}

void SoundManager::resetPlaybackState()
{
    _state = State::STOPPED;
    _sourceMode = SourceMode::None;

    _currentPath = "";

    _positionBytes = 0;
    _durationMs = 0;

    _fadeInEnabled = false;
    _fadeOutEnabled = false;

    _fadeInMs = 0;
    _fadeOutMs = 0;

    _stopFadeActive = false;
    _stopFadeDurationMs = 0;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;

    _pcmSampleRate = 0;

    resetPcmQueue();
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
    return _state == State::PLAYING ||
           _state == State::PAUSED ||
           _state == State::FADING_OUT;
}

SoundManager::State SoundManager::getState() const
{
    return _state;
}

const char* SoundManager::getStateString() const
{
    switch (_state)
    {
        case State::STOPPED:
            return "stopped";

        case State::PLAYING:
            return "playing";

        case State::PAUSED:
            return "paused";

        case State::FADING_OUT:
            return "fading_out";

        default:
            return "unknown";
    }
}

const char* SoundManager::getCurrentPath() const
{
    return _currentPath.c_str();
}

SoundManager::AudioStream SoundManager::getCurrentStream() const
{
    return _currentStream;
}

uint32_t SoundManager::getWavPositionMs() const
{
    if (_sourceMode != SourceMode::Wav ||
        _wav.byteRate == 0)
    {
        return 0;
    }

    return static_cast<uint32_t>(
        (static_cast<uint64_t>(_positionBytes) * 1000ULL) /
        _wav.byteRate
    );
}

uint32_t SoundManager::getPositionMs() const
{
    if (_sourceMode == SourceMode::Wav)
        return getWavPositionMs();

    if (_pcmSampleRate == 0)
        return 0;

    return static_cast<uint32_t>(
        (static_cast<uint64_t>(_positionBytes) * 1000ULL) /
        (static_cast<uint64_t>(_pcmSampleRate) * sizeof(int16_t))
    );
}

uint32_t SoundManager::getDurationMs() const
{
    return _durationMs;
}

// ============================================================
// STREAM VOLUME
// ============================================================

uint8_t SoundManager::streamVolume(AudioStream stream) const
{
    switch (stream)
    {
        case AudioStream::Media:
            return clampPercent(
                _settings.get(
                    SettingsManager::Param::VOLUME_MEDIA
                )
            );

        case AudioStream::Alarm:
            return clampPercent(
                _settings.get(
                    SettingsManager::Param::VOLUME_ALARM
                )
            );

        case AudioStream::System:
            return clampPercent(
                _settings.get(
                    SettingsManager::Param::VOLUME_SYSTEM
                )
            );

        default:
            return 100;
    }
}

void SoundManager::setStreamVolume(
    AudioStream stream,
    uint8_t volume
)
{
    volume = clampPercent(volume);

    switch (stream)
    {
        case AudioStream::Media:
            _settings.set(
                SettingsManager::Param::VOLUME_MEDIA,
                volume
            );
            break;

        case AudioStream::Alarm:
            _settings.set(
                SettingsManager::Param::VOLUME_ALARM,
                volume
            );
            break;

        case AudioStream::System:
            _settings.set(
                SettingsManager::Param::VOLUME_SYSTEM,
                volume
            );
            break;
    }
}

uint8_t SoundManager::getLocalPercent() const
{
    return _localPercent;
}

void SoundManager::setLocalPercent(uint8_t percent)
{
    _localPercent = clampPercent(percent);
}

void SoundManager::printStatus()
{
    Serial0.printf(
        "[SoundManager] state=%s stream=%u volume=%u%% "
        "local=%u%% position=%lu ms duration=%lu ms "
        "LittleFS=%s\n",
        getStateString(),
        static_cast<unsigned>(_currentStream),
        static_cast<unsigned>(streamVolume(_currentStream)),
        static_cast<unsigned>(_localPercent),
        static_cast<unsigned long>(getPositionMs()),
        static_cast<unsigned long>(getDurationMs()),
        _littleFSReady ? "ready" : "unavailable"
    );
}