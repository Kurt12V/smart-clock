
#include "SoundManager.h"

#include <Arduino.h>
#include <cstring>
#include <cmath>
#include <algorithm>

// ============================================================
// CONFIGURATION
// ============================================================

namespace
{
    // Плавное нарастание громкости для PCM-музыки.
    constexpr uint32_t PCM_FADE_IN_MS = 60000;

    // Период вывода диагностического сообщения.
    constexpr uint32_t AUDIO_DEBUG_INTERVAL_MS = 1000;

    // Exponential: progress * progress.
    constexpr SoundManager::FadeCurve DEFAULT_PCM_FADE_CURVE =
        SoundManager::FadeCurve::Exponential;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

SoundManager::SoundManager(
    SDManager& sdManager,
    SettingsManager& settings,
    I2SManager& i2sManager
)
    : _sdManager(sdManager),
      _settings(settings),
      _i2sManager(i2sManager),
      _initialized(false),
      _state(State::STOPPED),
      _sourceMode(SourceMode::None),
      _currentStream(AudioStream::Media),
      _positionBytes(0),
      _durationMs(0),
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
      _lastStatusMs(0),
      _lastUpdateMs(0),
      _pcmSource(nullptr),
      _pcmContext(nullptr),
      _finishedCallback(nullptr),
      _pcmSampleRate(0),
      _pcmHead(0),
      _pcmTail(0),
      _pcmCount(0)
{
    memset(&_wav, 0, sizeof(_wav));
}

// ============================================================
// BEGIN / END
// ============================================================

bool SoundManager::begin()
{
    Serial0.println();
    Serial0.println("[SOUND] ========================================");
    Serial0.println("[SOUND] Initializing SoundManager");
    Serial0.println("[SOUND] ========================================");

    if (!_sdManager.isReady())
    {
        Serial0.println(
            "[SOUND][WARN] SD is not ready; PCM playback remains available"
        );
    }

    if (!_i2sManager.isInitialized())
    {
        Serial0.println(
            "[SOUND][ERROR] I2SManager is not initialized"
        );
        return false;
    }

    _initialized = true;

    Serial0.printf(
        "[SOUND] Media volume:  %d%%\n",
        static_cast<int>(_settings.get(Param::VOLUME_MEDIA))
    );

    Serial0.printf(
        "[SOUND] Alarm volume:  %d%%\n",
        static_cast<int>(_settings.get(Param::VOLUME_ALARM))
    );

    Serial0.printf(
        "[SOUND] System volume: %d%%\n",
        static_cast<int>(_settings.get(Param::VOLUME_SYSTEM))
    );

    Serial0.println("[SOUND] Ready");

    return true;
}

void SoundManager::end()
{
    Serial0.println("[SOUND] Shutting down");

    stop();

    _initialized = false;
}

// ============================================================
// UPDATE
// Must be called once per loop()
// ============================================================

void SoundManager::update()
{
    if (!_initialized)
        return;

    if (_state != State::PLAYING &&
        _state != State::FADING_OUT)
    {
        return;
    }

    if (_stopFadeActive)
    {
        const uint32_t elapsed =
            millis() - _stopFadeStartMs;

        if (elapsed >= _stopFadeDurationMs)
        {
            Serial0.println("[SOUND] Stop fade completed");
            finishPlayback();
            return;
        }
    }

    readAndPlayChunk();

    _lastUpdateMs = millis();
}

// ============================================================
// WAV PLAY
// ============================================================

bool SoundManager::play(const char* path)
{
    PlayOptions opts;
    return play(path, opts);
}

bool SoundManager::play(
    const char* path,
    const PlayOptions& opts
)
{
    if (!_initialized || path == nullptr || path[0] == '\0')
    {
        Serial0.println("[SOUND][ERROR] Invalid WAV path");
        return false;
    }

    stop();

    _currentPath = path;
    _currentStream = opts.stream;

    _localPercent = static_cast<uint8_t>(
        constrain(
            static_cast<int>(opts.localPercent),
            0,
            100
        )
    );

    _fadeInEnabled = opts.fadeInMs > 0;
    _fadeOutEnabled = opts.fadeOutMs > 0;

    _fadeInMs = opts.fadeInMs;
    _fadeOutMs = opts.fadeOutMs;

    _fadeCurve = opts.curve;
    _fadeStartMs = millis();

    _stopFadeActive = false;

    if (!openWav(path))
    {
        Serial0.printf(
            "[SOUND][ERROR] Failed to open WAV: %s\n",
            path
        );

        _currentPath = "";
        return false;
    }

    if (!validateWav())
    {
        Serial0.println("[SOUND][ERROR] Unsupported WAV format");

        _file.close();
        _currentPath = "";
        return false;
    }

    Serial0.printf(
        "[SOUND] WAV format: %u ch, %lu Hz, %u bit\n",
        static_cast<unsigned>(_wav.channels),
        static_cast<unsigned long>(_wav.sampleRate),
        static_cast<unsigned>(_wav.bitsPerSample)
    );

    if (!_i2sManager.beginSpeaker(_wav.sampleRate))
    {
        Serial0.println("[SOUND][ERROR] Cannot initialize I2S speaker");

        _file.close();
        _currentPath = "";
        return false;
    }

    if (!_file.seek(_wav.dataOffset))
    {
        Serial0.println("[SOUND][ERROR] Cannot seek to WAV data");

        _file.close();
        _currentPath = "";
        return false;
    }

    _positionBytes = 0;

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println("[SOUND][ERROR] Cannot start I2S speaker");

        _file.close();
        _currentPath = "";
        return false;
    }

    _sourceMode = SourceMode::Wav;
    _state = State::PLAYING;

    _lastStatusMs = millis();
    _lastUpdateMs = millis();

    Serial0.printf(
        "[SOUND] WAV playback started: %s\n",
        path
    );

    Serial0.printf(
        "[SOUND] Duration: %lu ms\n",
        static_cast<unsigned long>(_durationMs)
    );

    Serial0.printf(
        "[SOUND] Effective volume: %u%%\n",
        static_cast<unsigned>(getEffectiveVolume())
    );

    return true;
}

// ============================================================
// PCM SOURCE PLAYBACK
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
        Serial0.println(
            "[SOUND][ERROR] playSource: invalid arguments"
        );
        return false;
    }

    stop();

    Serial0.println();
    Serial0.println("[SOUND] Starting generated PCM playback");

    Serial0.printf(
        "[SOUND] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    if (!startPcmOutput(sampleRate))
    {
        Serial0.println(
            "[SOUND][ERROR] Failed to initialize PCM output"
        );
        return false;
    }

    _currentPath = "";
    _currentStream = stream;
    _localPercent = 100;

    _pcmSource = source;
    _pcmContext = context;
    _finishedCallback = finished;
    _pcmSampleRate = sampleRate;

    // --------------------------------------------------------
    // FADE-IN
    // --------------------------------------------------------

    _fadeInEnabled = PCM_FADE_IN_MS > 0;
    _fadeOutEnabled = false;

    _fadeInMs = PCM_FADE_IN_MS;
    _fadeOutMs = 0;

    _fadeStartMs = millis();
    _fadeCurve = DEFAULT_PCM_FADE_CURVE;

    _stopFadeActive = false;
    _stopFadeStartMs = 0;
    _stopFadeDurationMs = 0;

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    _positionBytes = 0;
    _durationMs = 0;

    _sourceMode = SourceMode::CallbackPCM;
    _state = State::PLAYING;

    _lastUpdateMs = millis();
    _lastStatusMs = millis();

    Serial0.printf(
        "[SOUND] Alarm volume: %u%%\n",
        static_cast<unsigned>(
            streamVolume(_currentStream)
        )
    );

    Serial0.printf(
        "[SOUND] Local volume: %u%%\n",
        static_cast<unsigned>(_localPercent)
    );

    Serial0.printf(
        "[SOUND] Fade-in: %lu ms\n",
        static_cast<unsigned long>(_fadeInMs)
    );

    Serial0.println(
        "[SOUND] Fade curve: exponential"
    );

    Serial0.println("[SOUND] PCM playback started");

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
    {
        Serial0.println(
            "[SOUND][ERROR] beginPCM: invalid arguments"
        );
        return false;
    }

    stop();

    if (!startPcmOutput(sampleRate))
    {
        Serial0.println(
            "[SOUND][ERROR] Failed to start buffered PCM output"
        );
        return false;
    }

    _currentPath = "";
    _currentStream = stream;
    _localPercent = 100;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;
    _pcmSampleRate = sampleRate;

    _durationMs = 0;
    _positionBytes = 0;

    _stopFadeActive = false;
    _fadeInEnabled = false;
    _fadeOutEnabled = false;

    resetPcmQueue();

    _sourceMode = SourceMode::BufferedPCM;
    _state = State::PLAYING;

    _lastUpdateMs = millis();

    Serial0.printf(
        "[SOUND] Buffered PCM started at %lu Hz\n",
        static_cast<unsigned long>(sampleRate)
    );

    return true;
}

bool SoundManager::startPcmOutput(uint32_t sampleRate)
{
    if (!_i2sManager.beginSpeaker(sampleRate))
    {
        Serial0.println(
            "[SOUND][ERROR] beginSpeaker failed"
        );
        return false;
    }

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println(
            "[SOUND][ERROR] startSpeaker failed"
        );
        return false;
    }

    Serial0.println("[SOUND] PCM output ready");

    return true;
}

bool SoundManager::submitPCM(
    const int16_t* samples,
    size_t sampleCount
)
{
    if (samples == nullptr ||
        _sourceMode != SourceMode::BufferedPCM ||
        _state == State::STOPPED)
    {
        return false;
    }

    if (sampleCount > PCM_QUEUE_CAPACITY - _pcmCount)
    {
        Serial0.println(
            "[SOUND][WARN] PCM queue full; block rejected"
        );
        return false;
    }

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
}

// ============================================================
// STOP / PAUSE / RESUME
// ============================================================

void SoundManager::stop()
{
    const bool wasActive = _state != State::STOPPED;

    if (wasActive)
    {
        Serial0.println("[SOUND] Stopping playback");

        _i2sManager.stopSpeaker();
        _i2sManager.clearSpeaker();
    }

    if (_file)
        _file.close();

    _state = State::STOPPED;
    _sourceMode = SourceMode::None;

    _positionBytes = 0;
    _currentPath = "";

    _stopFadeActive = false;
    _stopFadeStartMs = 0;
    _stopFadeDurationMs = 0;

    _fadeInEnabled = false;
    _fadeOutEnabled = false;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;
    _pcmSampleRate = 0;

    resetPcmQueue();
}

void SoundManager::stop(uint32_t fadeOutMs)
{
    if (_state != State::PLAYING || fadeOutMs == 0)
    {
        stop();
        return;
    }

    _stopFadeActive = true;
    _stopFadeStartMs = millis();
    _stopFadeDurationMs = fadeOutMs;

    _fadeCurve = FadeCurve::Exponential;
    _state = State::FADING_OUT;

    Serial0.printf(
        "[SOUND] Fade-out started: %lu ms\n",
        static_cast<unsigned long>(fadeOutMs)
    );
}

bool SoundManager::pause()
{
    if (_state != State::PLAYING)
        return false;

    if (!_i2sManager.stopSpeaker())
    {
        Serial0.println("[SOUND][ERROR] Pause failed");
        return false;
    }

    _state = State::PAUSED;

    Serial0.println("[SOUND] Paused");

    return true;
}

bool SoundManager::resume()
{
    if (_state != State::PAUSED)
        return false;

    if (!_i2sManager.startSpeaker())
    {
        Serial0.println("[SOUND][ERROR] Resume failed");
        return false;
    }

    _state = State::PLAYING;
    _lastUpdateMs = millis();

    Serial0.println("[SOUND] Resumed");

    return true;
}

// ============================================================
// OPEN / PARSE WAV
// ============================================================

bool SoundManager::openWav(const char* path)
{
    if (!_sdManager.isReady())
    {
        Serial0.println("[SOUND][ERROR] SD is not ready");
        return false;
    }

    _file = _sdManager.card().fs().open(path, FILE_READ);

    if (!_file)
    {
        Serial0.printf(
            "[SOUND][ERROR] File not found: %s\n",
            path
        );
        return false;
    }

    if (!parseWav(_file))
    {
        Serial0.println("[SOUND][ERROR] WAV parsing failed");
        _file.close();
        return false;
    }

    return true;
}

bool SoundManager::parseWav(File& file)
{
    memset(&_wav, 0, sizeof(_wav));

    uint8_t header[12];

    if (file.read(header, sizeof(header)) != sizeof(header))
        return false;

    if (memcmp(header, "RIFF", 4) != 0 ||
        memcmp(header + 8, "WAVE", 4) != 0)
    {
        Serial0.println("[SOUND][ERROR] Invalid RIFF/WAVE header");
        return false;
    }

    bool foundFmt = false;
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
            static_cast<uint32_t>(chunkHeader[4]) |
            (static_cast<uint32_t>(chunkHeader[5]) << 8) |
            (static_cast<uint32_t>(chunkHeader[6]) << 16) |
            (static_cast<uint32_t>(chunkHeader[7]) << 24);

        const uint32_t chunkDataOffset =
            static_cast<uint32_t>(file.position());

        if (memcmp(chunkHeader, "fmt ", 4) == 0)
        {
            if (chunkSize < 16)
                return false;

            uint8_t fmt[16];

            if (file.read(fmt, sizeof(fmt)) != sizeof(fmt))
                return false;

            _wav.audioFormat =
                static_cast<uint16_t>(fmt[0] | (fmt[1] << 8));

            _wav.channels =
                static_cast<uint16_t>(fmt[2] | (fmt[3] << 8));

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
                static_cast<uint16_t>(fmt[12] | (fmt[13] << 8));

            _wav.bitsPerSample =
                static_cast<uint16_t>(fmt[14] | (fmt[15] << 8));

            foundFmt = true;
        }
        else if (memcmp(chunkHeader, "data", 4) == 0)
        {
            _wav.dataOffset = chunkDataOffset;
            _wav.dataSize = chunkSize;
            foundData = true;
        }

        const uint32_t nextOffset =
            chunkDataOffset + chunkSize + (chunkSize & 1U);

        if (!file.seek(nextOffset))
            break;

        if (foundFmt && foundData)
            break;
    }

    if (!foundFmt || !foundData || _wav.byteRate == 0)
        return false;

    _durationMs = static_cast<uint32_t>(
        (static_cast<uint64_t>(_wav.dataSize) * 1000ULL) /
        _wav.byteRate
    );

    return true;
}

bool SoundManager::validateWav() const
{
    if (_wav.audioFormat != 1)
        return false;

    if (_wav.channels != 1 && _wav.channels != 2)
        return false;

    if (_wav.sampleRate == 0 ||
        _wav.bitsPerSample != 16 ||
        _wav.blockAlign == 0 ||
        _wav.dataSize == 0)
    {
        return false;
    }

    if (_wav.blockAlign != _wav.channels * sizeof(int16_t))
        return false;

    return true;
}

// ============================================================
// READ AND PLAY
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

        default:
            finishPlayback();
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

    if (_positionBytes >= _wav.dataSize)
    {
        finishPlayback();
        return true;
    }

    const uint32_t remaining =
        _wav.dataSize - _positionBytes;

    size_t bytesToRead = std::min(
        static_cast<size_t>(remaining),
        static_cast<size_t>(BUFFER_BYTES)
    );

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
        Serial0.println("[SOUND][ERROR] WAV read returned zero bytes");
        finishPlayback();
        return false;
    }

    const size_t completeBytes =
        bytesRead - (bytesRead % _wav.blockAlign);

    size_t outputSamples = 0;

    if (_wav.channels == 1)
    {
        outputSamples = completeBytes / sizeof(int16_t);

        memcpy(
            _outputBuffer,
            _inputBuffer,
            outputSamples * sizeof(int16_t)
        );
    }
    else
    {
        const size_t frames =
            completeBytes / (2 * sizeof(int16_t));

        for (size_t i = 0; i < frames; ++i)
        {
            int16_t left;
            int16_t right;

            memcpy(&_inputBuffer[i * 4], &_inputBuffer[i * 4], 0);

            memcpy(&left, _inputBuffer + i * 4, sizeof(int16_t));
            memcpy(&right, _inputBuffer + i * 4 + 2, sizeof(int16_t));

            _outputBuffer[i] = static_cast<int16_t>(
                (static_cast<int32_t>(left) +
                 static_cast<int32_t>(right)) / 2
            );
        }

        outputSamples = frames;
    }

    if (outputSamples > 0 &&
        !writeSamples(_outputBuffer, outputSamples))
    {
        Serial0.println("[SOUND][ERROR] WAV output failed");
        finishPlayback();
        return false;
    }

    _positionBytes += static_cast<uint32_t>(completeBytes);

    if (_positionBytes >= _wav.dataSize)
        finishPlayback();

    return true;
}

bool SoundManager::readPcmChunk()
{
    size_t samplesRead = 0;

    const size_t capacity =
        sizeof(_outputBuffer) / sizeof(_outputBuffer[0]);

    if (_sourceMode == SourceMode::CallbackPCM)
    {
        if (_pcmSource == nullptr)
        {
            Serial0.println("[SOUND][ERROR] PCM callback is null");
            finishPlayback();
            return false;
        }

        samplesRead = _pcmSource(
            _pcmContext,
            _outputBuffer,
            capacity
        );

        if (samplesRead == 0)
        {
            Serial0.println("[SOUND] PCM source finished");
            finishPlayback();
            return true;
        }

        if (samplesRead > capacity)
        {
            Serial0.println(
                "[SOUND][ERROR] PCM callback returned invalid sample count"
            );
            finishPlayback();
            return false;
        }
    }
    else
    {
        samplesRead = std::min(_pcmCount, capacity);

        if (samplesRead == 0)
            return true;

        for (size_t i = 0; i < samplesRead; ++i)
        {
            _outputBuffer[i] = _pcmQueue[_pcmTail];

            _pcmTail =
                (_pcmTail + 1) % PCM_QUEUE_CAPACITY;
        }

        _pcmCount -= samplesRead;
    }

    if (!writeSamples(_outputBuffer, samplesRead))
    {
        Serial0.println("[SOUND][ERROR] PCM output failed");
        finishPlayback();
        return false;
    }

    _positionBytes += static_cast<uint32_t>(
        samplesRead * sizeof(int16_t)
    );

    return true;
}

// ============================================================
// WRITE SAMPLES / DEBUG
// ============================================================

bool SoundManager::writeSamples(
    int16_t* samples,
    size_t sampleCount
)
{
    if (samples == nullptr || sampleCount == 0)
        return false;

    int32_t peakBefore = 0;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        const int32_t value = samples[i];
        const int32_t absoluteValue =
            value < 0 ? -value : value;

        if (absoluteValue > peakBefore)
            peakBefore = absoluteValue;
    }

    const uint8_t volume = getEffectiveVolume();

    applyVolume(samples, sampleCount, volume);

    int32_t peakAfter = 0;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        const int32_t value = samples[i];
        const int32_t absoluteValue =
            value < 0 ? -value : value;

        if (absoluteValue > peakAfter)
            peakAfter = absoluteValue;
    }

    const size_t outputBytes =
        sampleCount * sizeof(int16_t);

    size_t written = 0;

    const bool ok = _i2sManager.writeSpeaker(
        reinterpret_cast<const uint8_t*>(samples),
        outputBytes,
        written,
        100
    );

    static uint32_t blockCount = 0;
    static uint32_t lastDebugMs = 0;

    ++blockCount;

    const uint32_t now = millis();

    if (blockCount <= 5 ||
        now - lastDebugMs >= AUDIO_DEBUG_INTERVAL_MS)
    {
        lastDebugMs = now;

        Serial0.printf(
            "[AUDIO] block=%lu samples=%u "
            "peakIn=%ld peakOut=%ld "
            "volume=%u%% fadeElapsed=%lu ms "
            "written=%u/%u status=%s\n",
            static_cast<unsigned long>(blockCount),
            static_cast<unsigned>(sampleCount),
            static_cast<long>(peakBefore),
            static_cast<long>(peakAfter),
            static_cast<unsigned>(volume),
            static_cast<unsigned long>(now - _fadeStartMs),
            static_cast<unsigned>(written),
            static_cast<unsigned>(outputBytes),
            ok && written == outputBytes ? "OK" : "ERROR"
        );
    }

    if (!ok || written != outputBytes)
    {
        Serial0.println(
            "[SOUND][ERROR] I2S block was not written completely"
        );
        return false;
    }

    return true;
}

// ============================================================
// FINISH PLAYBACK
// ============================================================

void SoundManager::finishPlayback()
{
    Serial0.println("[SOUND] Finishing playback");

    _i2sManager.stopSpeaker();
    _i2sManager.clearSpeaker();

    if (_file)
        _file.close();

    FinishedCallback callback = _finishedCallback;
    void* context = _pcmContext;

    _state = State::STOPPED;
    _sourceMode = SourceMode::None;

    _positionBytes = 0;
    _currentPath = "";

    _stopFadeActive = false;
    _fadeInEnabled = false;
    _fadeOutEnabled = false;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;
    _pcmSampleRate = 0;

    resetPcmQueue();

    if (callback != nullptr)
    {
        Serial0.println("[SOUND] Calling finished callback");
        callback(context);
    }
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
        memset(samples, 0, sampleCount * sizeof(int16_t));
        return;
    }

    for (size_t i = 0; i < sampleCount; ++i)
    {
        const int32_t value =
            (static_cast<int32_t>(samples[i]) * volume) / 100;

        samples[i] = static_cast<int16_t>(
            constrain(value, -32768, 32767)
        );
    }
}

uint8_t SoundManager::streamVolume(AudioStream stream) const
{
    int value = 100;

    switch (stream)
    {
        case AudioStream::Alarm:
            value = _settings.get(Param::VOLUME_ALARM);
            break;

        case AudioStream::System:
            value = _settings.get(Param::VOLUME_SYSTEM);
            break;

        case AudioStream::Media:
        default:
            value = _settings.get(Param::VOLUME_MEDIA);
            break;
    }

    return static_cast<uint8_t>(
        constrain(value, 0, 100)
    );
}

void SoundManager::setStreamVolume(
    AudioStream stream,
    uint8_t volume
)
{
    volume = static_cast<uint8_t>(
        constrain(static_cast<int>(volume), 0, 100)
    );

    Param param = Param::VOLUME_MEDIA;

    switch (stream)
    {
        case AudioStream::Alarm:
            param = Param::VOLUME_ALARM;
            break;

        case AudioStream::System:
            param = Param::VOLUME_SYSTEM;
            break;

        case AudioStream::Media:
        default:
            param = Param::VOLUME_MEDIA;
            break;
    }

    _settings.set(param, volume);

    Serial0.printf(
        "[SOUND] Stream %u volume set to %u%%\n",
        static_cast<unsigned>(stream),
        static_cast<unsigned>(volume)
    );
}

uint8_t SoundManager::getLocalPercent() const
{
    return _localPercent;
}

void SoundManager::setLocalPercent(uint8_t percent)
{
    _localPercent = static_cast<uint8_t>(
        constrain(static_cast<int>(percent), 0, 100)
    );
}

uint8_t SoundManager::calculateBaseVolume() const
{
    const uint16_t streamVol =
        streamVolume(_currentStream);

    return static_cast<uint8_t>(
        (streamVol * static_cast<uint16_t>(_localPercent)) / 100
    );
}

uint8_t SoundManager::getEffectiveVolume() const
{
    const float result =
        calculateBaseVolume() * calculateFadeMultiplier();

    return static_cast<uint8_t>(
        constrain(static_cast<int>(result), 0, 100)
    );
}

// ============================================================
// FADE
// ============================================================

float SoundManager::calculateFadeMultiplier() const
{
    if (_state != State::PLAYING &&
        _state != State::FADING_OUT)
    {
        return 1.0f;
    }

    const uint32_t now = millis();

    // --------------------------------------------------------
    // STOP FADE-OUT
    // --------------------------------------------------------

    if (_stopFadeActive && _stopFadeDurationMs > 0)
    {
        const uint32_t elapsed =
            now - _stopFadeStartMs;

        const float progress = constrain(
            static_cast<float>(elapsed) /
                static_cast<float>(_stopFadeDurationMs),
            0.0f,
            1.0f
        );

        return applyFadeCurve(1.0f - progress);
    }

    // --------------------------------------------------------
    // FADE-IN
    // --------------------------------------------------------

    if (_fadeInEnabled && _fadeInMs > 0)
    {
        const uint32_t elapsed =
            now - _fadeStartMs;

        if (elapsed < _fadeInMs)
        {
            const float progress =
                static_cast<float>(elapsed) /
                static_cast<float>(_fadeInMs);

            return applyFadeCurve(progress);
        }
    }

    // --------------------------------------------------------
    // WAV FADE-OUT
    // --------------------------------------------------------

    if (_fadeOutEnabled &&
        _fadeOutMs > 0 &&
        _durationMs > 0 &&
        _sourceMode == SourceMode::Wav)
    {
        const uint32_t position = getPositionMs();

        const uint32_t fadeStart =
            _durationMs > _fadeOutMs
                ? _durationMs - _fadeOutMs
                : 0;

        if (position >= fadeStart)
        {
            const uint32_t elapsed =
                position - fadeStart;

            const float progress = constrain(
                static_cast<float>(elapsed) /
                    static_cast<float>(_fadeOutMs),
                0.0f,
                1.0f
            );

            return applyFadeCurve(1.0f - progress);
        }
    }

    return 1.0f;
}

float SoundManager::applyFadeCurve(float value) const
{
    value = constrain(value, 0.0f, 1.0f);

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
// POSITION / DURATION
// ============================================================

uint32_t SoundManager::getPositionMs() const
{
    if (_sourceMode == SourceMode::Wav)
    {
        if (_wav.byteRate == 0)
            return 0;

        return static_cast<uint32_t>(
            (static_cast<uint64_t>(_positionBytes) * 1000ULL) /
            _wav.byteRate
        );
    }

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
        case State::PLAYING:
            return "playing";

        case State::PAUSED:
            return "paused";

        case State::FADING_OUT:
            return "fading_out";

        case State::STOPPED:
        default:
            return "stopped";
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

// ============================================================
// DEBUG STATUS
// ============================================================

void SoundManager::printStatus()
{
    Serial0.println();
    Serial0.println("[SOUND] ---------- STATUS ----------");

    Serial0.printf("[SOUND] State:        %s\n", getStateString());

    Serial0.printf(
        "[SOUND] Path:         %s\n",
        _currentPath.length()
            ? _currentPath.c_str()
            : "-"
    );

    Serial0.printf(
        "[SOUND] Stream:       %u\n",
        static_cast<unsigned>(_currentStream)
    );

    Serial0.printf(
        "[SOUND] Local volume: %u%%\n",
        static_cast<unsigned>(_localPercent)
    );

    Serial0.printf(
        "[SOUND] Stream volume:%u%%\n",
        static_cast<unsigned>(streamVolume(_currentStream))
    );

    Serial0.printf(
        "[SOUND] Effective:    %u%%\n",
        static_cast<unsigned>(getEffectiveVolume())
    );

    Serial0.printf(
        "[SOUND] Position:     %lu / %lu ms\n",
        static_cast<unsigned long>(getPositionMs()),
        static_cast<unsigned long>(getDurationMs())
    );

    Serial0.println("[SOUND] --------------------------------");
}
