
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
    : _settings(settings),
      _i2sManager(i2sManager)
{
}

// ============================================================
// BEGIN / END
// ============================================================

bool SoundManager::begin()
{
    if (_initialized)
        return true;

    Serial0.println("[SoundManager] Initializing");

    _littleFSReady = LittleFS.begin(false);

    if (_littleFSReady)
        Serial0.println("[SoundManager] LittleFS ready");
    else
        Serial0.println("[SoundManager] LittleFS unavailable");

    resetPcmQueue();
    resetPlaybackState();

    _localPercent = 100;
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
        requestDrain();
    }

    if (_drainPending)
    {
        processDrain();
        return;
    }

    readAndPlayChunk();
}

bool SoundManager::isLittleFSReady() const
{
    return _littleFSReady;
}

// ============================================================
// WAV PLAYBACK
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
    if (!_initialized || path == nullptr || path[0] == '\0')
        return false;

    stop();

    if (!openWav(path))
    {
        Serial0.printf(
            "[SoundManager] Cannot open WAV: %s\n",
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

    _drainPending = false;

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
        return false;

    if (_file)
        _file.close();

    _currentPath = "";
    _positionBytes = 0;
    _durationMs = 0;

    _file = LittleFS.open(path, FILE_READ);

    if (!_file)
        return false;

    if (!parseWav(_file) || !validateWav())
    {
        _file.close();
        return false;
    }

    if (!_file.seek(_wav.dataOffset))
    {
        _file.close();
        return false;
    }

    if (_wav.byteRate == 0)
    {
        _file.close();
        return false;
    }

    _currentPath = path;

    _durationMs = static_cast<uint32_t>(
        (static_cast<uint64_t>(_wav.dataSize) * 1000ULL) /
        _wav.byteRate
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

    const uint32_t fileSize = file.size();

    while (file.position() + 8 <= fileSize)
    {
        uint8_t chunkHeader[8];

        if (file.read(chunkHeader, sizeof(chunkHeader)) != sizeof(chunkHeader))
            return false;

        const uint32_t chunkSize = readLE32(chunkHeader + 4);
        const uint32_t chunkDataOffset = file.position();

        const uint64_t nextChunk =
            static_cast<uint64_t>(chunkDataOffset) +
            chunkSize +
            (chunkSize & 1U);

        if (nextChunk > fileSize)
            return false;

        if (std::memcmp(chunkHeader, "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
                return false;

            uint8_t formatData[16];

            if (file.read(formatData, sizeof(formatData)) != sizeof(formatData))
                return false;

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

        if (foundFormat && foundData)
            break;

        if (!file.seek(static_cast<uint32_t>(nextChunk)))
            return false;
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
// CALLBACK PCM
// ============================================================

bool SoundManager::playSource(
    PcmSourceCallback source,
    void* context,
    uint32_t sampleRate,
    AudioStream stream,
    FinishedCallback finished
)
{
    if (!_initialized || source == nullptr || sampleRate == 0)
        return false;

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

    _drainPending = false;
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

    resetPcmQueue();

    if (!startPcmOutput(sampleRate))
        return false;

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

    _pcmInputFinished = false;
    _pcmPlaybackFinished = false;
    _pcmOutputStarted = false;
    _pcmUnderrunActive = false;
    _pcmReceivedSamples = 0;
    _pcmPlayedSamples = 0;
    _pcmUnderrunCount = 0;
    _pcmMaxQueued = 0;
    _pcmLastDiagnosticMs = millis();

    _drainPending = false;

    _state = State::PLAYING;

    // startPcmOutput configures the I2S driver.  Stop it until the network
    // FIFO has reached the prebuffer threshold; otherwise the DMA begins
    // consuming silence before the first WebSocket blocks arrive.
    _i2sManager.stopSpeaker();
    _i2sManager.clearSpeaker();

    _lastStatusMs = millis();
    _audioBlocksWritten = 0;

    Serial0.printf(
        "[SoundManager] Buffered PCM started: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    return true;
}

bool SoundManager::submitPCM(
    const int16_t* samples,
    size_t sampleCount
)
{
    if (!_initialized ||
        _sourceMode != SourceMode::BufferedPCM ||
        _pcmInputFinished ||
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

        _pcmHead = (_pcmHead + 1) % PCM_QUEUE_CAPACITY;
    }

    _pcmCount += sampleCount;

    _pcmReceivedSamples += static_cast<uint32_t>(sampleCount);

    if (_pcmCount > _pcmMaxQueued)
        _pcmMaxQueued = _pcmCount;

    return true;
}

size_t SoundManager::availablePCM() const
{
    return PCM_QUEUE_CAPACITY - _pcmCount;
}

size_t SoundManager::queuedPCM() const
{
    return _pcmCount;
}

void SoundManager::finishPCM()
{
    if (_sourceMode != SourceMode::BufferedPCM)
        return;

    _pcmInputFinished = true;

    // Если очередь пуста, update() запустит процедуру
    // ожидания окончания передачи I2S.
    if (_pcmCount == 0)
        requestDrain();
}

bool SoundManager::isPCMPrebufferReady() const
{
    if (_sourceMode != SourceMode::BufferedPCM)
        return false;

    if (_pcmOutputStarted || _pcmInputFinished)
        return true;

    return _pcmCount >= PCM_PREBUFFER_SAMPLES;
}

bool SoundManager::isPCMPlaybackFinished() const
{
    return _pcmPlaybackFinished;
}

void SoundManager::resetPcmQueue()
{
    _pcmHead = 0;
    _pcmTail = 0;
    _pcmCount = 0;

    _pcmInputFinished = false;
    _pcmPlaybackFinished = false;

    std::memset(_pcmQueue, 0, sizeof(_pcmQueue));
}

// ============================================================
// I2S OUTPUT
// ============================================================

bool SoundManager::startPcmOutput(uint32_t sampleRate)
{
    if (sampleRate == 0)
        return false;

    _i2sManager.stopSpeaker();

    if (!_i2sManager.beginSpeaker(sampleRate))
    {
        Serial0.println("[SoundManager] I2S begin failed");
        return false;
    }

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println("[SoundManager] I2S start failed");
        return false;
    }

    return true;
}

// ============================================================
// AUDIO UPDATE
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
        requestDrain();
        return false;
    }

    const uint32_t remaining =
        _wav.dataSize > _positionBytes
            ? _wav.dataSize - _positionBytes
            : 0;

    if (remaining == 0)
    {
        requestDrain();
        return false;
    }

    size_t bytesToRead = std::min<size_t>(BUFFER_BYTES, remaining);

    bytesToRead -= bytesToRead % _wav.blockAlign;

    if (bytesToRead == 0)
    {
        requestDrain();
        return false;
    }

    const size_t bytesRead = _file.read(_inputBuffer, bytesToRead);

    if (bytesRead == 0)
    {
        requestDrain();
        return false;
    }

    const size_t frames = bytesRead / _wav.blockAlign;

    if (_wav.channels == 1)
    {
        const size_t samples = std::min(frames, BUFFER_SAMPLES);

        std::memcpy(
            _outputBuffer,
            _inputBuffer,
            samples * sizeof(int16_t)
        );

        _positionBytes += static_cast<uint32_t>(bytesRead);

        if (!writeSamples(_outputBuffer, samples))
            return false;
    }
    else
    {
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

        if (!writeSamples(_outputBuffer, frames))
            return false;
    }

    if (_positionBytes >= _wav.dataSize)
        requestDrain();

    return true;
}

bool SoundManager::readPcmChunk()
{
    // --------------------------------------------------------
    // CALLBACK PCM
    // --------------------------------------------------------

    if (_sourceMode == SourceMode::CallbackPCM)
    {
        if (_pcmSource == nullptr)
        {
            requestDrain();
            return false;
        }

        size_t count = _pcmSource(
            _pcmContext,
            _outputBuffer,
            BUFFER_SAMPLES
        );

        if (count == 0)
        {
            requestDrain();
            return false;
        }

        if (count > BUFFER_SAMPLES)
            count = BUFFER_SAMPLES;

        _positionBytes +=
            static_cast<uint32_t>(count * sizeof(int16_t));

        return writeSamples(_outputBuffer, count);
    }

    // --------------------------------------------------------
    // BUFFERED PCM
    // --------------------------------------------------------

    if (_sourceMode == SourceMode::BufferedPCM)
    {
        // Do not start I2S on the first packet. A real prebuffer absorbs
        // normal Wi-Fi/WebSocket packet jitter before audible playback.
        if (!_pcmOutputStarted)
        {
            if (!_pcmInputFinished &&
                _pcmCount < PCM_PREBUFFER_SAMPLES)
            {
                return false;
            }

            if (_pcmCount == 0)
            {
                requestDrain();
                return false;
            }

            if (!_i2sManager.startSpeaker())
            {
                Serial0.println("[SoundManager][ERROR] Cannot start I2S PCM output");
                return false;
            }

            _pcmOutputStarted = true;

            Serial0.printf(
                "[SoundManager] PCM playback started: prebuffer=%u samples (%lu ms)\n",
                static_cast<unsigned>(_pcmCount),
                static_cast<unsigned long>(
                    (static_cast<uint64_t>(_pcmCount) * 1000ULL) /
                    _pcmSampleRate
                )
            );
        }

        if (_pcmCount == 0)
        {
            if (_pcmInputFinished)
            {
                requestDrain();
            }
            else if (!_pcmUnderrunActive)
            {
                _pcmUnderrunActive = true;
                ++_pcmUnderrunCount;
                Serial0.printf(
                    "[SoundManager][WARN] PCM underrun #%lu\n",
                    static_cast<unsigned long>(_pcmUnderrunCount)
                );
            }

            return false;
        }

        const size_t count =
            std::min(_pcmCount, BUFFER_SAMPLES);

        for (size_t i = 0; i < count; ++i)
        {
            _outputBuffer[i] = _pcmQueue[_pcmTail];

            _pcmTail = (_pcmTail + 1) % PCM_QUEUE_CAPACITY;
        }

        _pcmCount -= count;

        _pcmUnderrunActive = false;

        _pcmPlayedSamples += static_cast<uint32_t>(count);

        _positionBytes +=
            static_cast<uint32_t>(count * sizeof(int16_t));

        if (!writeSamples(_outputBuffer, count))
            return false;

        if (_pcmInputFinished && _pcmCount == 0)
            requestDrain();

        const uint32_t now = millis();

        if (now - _pcmLastDiagnosticMs >= AUDIO_DEBUG_INTERVAL_MS)
        {
            _pcmLastDiagnosticMs = now;

            Serial0.printf(
                "[SoundManager][PCM] queued=%u/%u samples (%lu ms) max=%u rx=%lu played=%lu underruns=%lu\n",
                static_cast<unsigned>(_pcmCount),
                static_cast<unsigned>(PCM_QUEUE_CAPACITY),
                static_cast<unsigned long>(
                    (static_cast<uint64_t>(_pcmCount) * 1000ULL) /
                    _pcmSampleRate
                ),
                static_cast<unsigned>(_pcmMaxQueued),
                static_cast<unsigned long>(_pcmReceivedSamples),
                static_cast<unsigned long>(_pcmPlayedSamples),
                static_cast<unsigned long>(_pcmUnderrunCount)
            );
        }

        return true;
    }

    return false;
}

// ============================================================
// WRITE SAMPLES
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

    const uint8_t* data =
        reinterpret_cast<const uint8_t*>(samples);

    const size_t totalBytes = sampleCount * sizeof(int16_t);

    size_t offset = 0;

    while (offset < totalBytes)
    {
        size_t bytesWritten = 0;

        const bool ok = _i2sManager.writeSpeaker(
            data + offset,
            totalBytes - offset,
            bytesWritten,
            I2S_WRITE_TIMEOUT_MS
        );

        if (!ok || bytesWritten == 0)
        {
            Serial0.printf(
                "[SoundManager] I2S write failed at %u/%u bytes\n",
                static_cast<unsigned>(offset),
                static_cast<unsigned>(totalBytes)
            );

            return false;
        }

        offset += bytesWritten;
    }

    ++_audioBlocksWritten;

    const uint32_t now = millis();

    if (now - _lastStatusMs >= AUDIO_DEBUG_INTERVAL_MS)
    {
        _lastStatusMs = now;

        Serial0.printf(
            "[AUDIO] Blocks=%lu Volume=%u%% Bytes=%u\n",
            static_cast<unsigned long>(_audioBlocksWritten),
            static_cast<unsigned>(getEffectiveVolume()),
            static_cast<unsigned>(totalBytes)
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

    const float gain = static_cast<float>(volume) / 100.0f;

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
// DRAIN / FINISH
// ============================================================

void SoundManager::requestDrain()
{
    _drainPending = true;
}

void SoundManager::processDrain()
{
    // Не останавливаем I2S, пока данные ещё передаются.
    if (!_i2sManager.waitSpeakerTxDone(5))
        return;

    finishPlayback();
}

void SoundManager::finishPlayback(bool notify)
{
    FinishedCallback callback = _finishedCallback;
    void* context = _pcmContext;

    const bool wasBufferedPCM =
        _sourceMode == SourceMode::BufferedPCM;

    if (_file)
        _file.close();

    if (_i2sManager.isSpeakerInitialized())
    {
        _i2sManager.stopSpeaker();
        _i2sManager.clearSpeaker();
    }

    if (wasBufferedPCM)
        _pcmPlaybackFinished = true;

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

    _drainPending = false;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;

    _pcmSampleRate = 0;

    resetPcmQueue();
}

// ============================================================
// FADE
// ============================================================

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

// ============================================================
// VOLUME
// ============================================================

uint8_t SoundManager::calculateBaseVolume() const
{
    const uint16_t stream = streamVolume(_currentStream);
    const uint16_t local = clampPercent(_localPercent);

    return static_cast<uint8_t>(
        (stream * local + 50U) / 100U
    );
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

uint8_t SoundManager::streamVolume(AudioStream stream) const
{
    switch (stream)
    {
        case AudioStream::Media:
            return clampPercent(
                _settings.get(SettingsManager::Param::VOLUME_MEDIA)
            );

        case AudioStream::Alarm:
            return clampPercent(
                _settings.get(SettingsManager::Param::VOLUME_ALARM)
            );

        case AudioStream::System:
            return clampPercent(
                _settings.get(SettingsManager::Param::VOLUME_SYSTEM)
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

    if (_i2sManager.isSpeakerInitialized())
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

    if (sampleRate == 0 || !startPcmOutput(sampleRate))
        return false;

    _state = State::PLAYING;

    return true;
}

void SoundManager::stop()
{
    if (_i2sManager.isSpeakerInitialized())
    {
        _i2sManager.stopSpeaker();
        _i2sManager.clearSpeaker();
    }

    if (_file)
        _file.close();

    if (_sourceMode == SourceMode::BufferedPCM)
        _pcmPlaybackFinished = true;

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

void SoundManager::printStatus()
{
    Serial0.printf(
        "[SoundManager] state=%s stream=%u volume=%u%% "
        "local=%u%% position=%lu ms duration=%lu ms "
        "queue=%u LittleFS=%s\n",
        getStateString(),
        static_cast<unsigned>(_currentStream),
        static_cast<unsigned>(streamVolume(_currentStream)),
        static_cast<unsigned>(_localPercent),
        static_cast<unsigned long>(getPositionMs()),
        static_cast<unsigned long>(getDurationMs()),
        static_cast<unsigned>(_pcmCount),
        _littleFSReady ? "ready" : "unavailable"
    );
}
