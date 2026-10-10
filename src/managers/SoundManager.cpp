
#include "SoundManager.h"

#include <cstring>
#include <cmath>
#include <algorithm>

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
    Serial0.println("[SOUND] Starting...");

    if (!_sdManager.isReady())
    {
        Serial0.println("[SOUND] SD not ready");
        // PCM playback does not inherently require SD, so continue
        // initialization if I2S is ready.
    }

    if (!_i2sManager.isInitialized())
    {
        Serial0.println("[SOUND] ERROR: I2S not initialized");
        return false;
    }

    _initialized = true;

    Serial0.printf("[SOUND] vol_media  = %d\n",
        static_cast<int>(_settings.get(Param::VOLUME_MEDIA)));
    Serial0.printf("[SOUND] vol_alarm  = %d\n",
        static_cast<int>(_settings.get(Param::VOLUME_ALARM)));
    Serial0.printf("[SOUND] vol_system = %d\n",
        static_cast<int>(_settings.get(Param::VOLUME_SYSTEM)));

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

    if (_stopFadeActive)
    {
        const uint32_t elapsed = millis() - _stopFadeStartMs;

        if (elapsed >= _stopFadeDurationMs)
        {
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
        return false;

    stop();

    _currentPath = path;
    _currentStream = opts.stream;
    _localPercent = static_cast<uint8_t>(
        constrain(static_cast<int>(opts.localPercent), 0, 100));

    _fadeInEnabled = opts.fadeInMs > 0;
    _fadeOutEnabled = opts.fadeOutMs > 0;
    _fadeInMs = opts.fadeInMs;
    _fadeOutMs = opts.fadeOutMs;
    _fadeCurve = opts.curve;
    _fadeStartMs = millis();
    _stopFadeActive = false;

    if (!openWav(path))
    {
        _currentPath = "";
        return false;
    }

    if (!validateWav())
    {
        _file.close();
        _currentPath = "";
        return false;
    }

    if (!_i2sManager.beginSpeaker(_wav.sampleRate))
    {
        _file.close();
        _currentPath = "";
        return false;
    }

    if (!_file.seek(_wav.dataOffset))
    {
        _file.close();
        _currentPath = "";
        return false;
    }

    _positionBytes = 0;

    if (!_i2sManager.startSpeaker())
    {
        _file.close();
        _currentPath = "";
        return false;
    }

    _sourceMode = SourceMode::Wav;
    _state = State::PLAYING;
    _lastStatusMs = millis();
    _lastUpdateMs = millis();

    Serial0.printf(
        "[SOUND] WAV: %s, %u ch, %lu Hz, %lu ms\n",
        path,
        static_cast<unsigned>(_wav.channels),
        static_cast<unsigned long>(_wav.sampleRate),
        static_cast<unsigned long>(_durationMs)
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
    if (!_initialized || source == nullptr || sampleRate == 0)
        return false;

    stop();

    if (!startPcmOutput(sampleRate))
        return false;

    _currentPath = "";
    _currentStream = stream;
    _localPercent = 100;

    _pcmSource = source;
    _pcmContext = context;
    _finishedCallback = finished;
    _pcmSampleRate = sampleRate;

    _fadeInEnabled = false;
    _fadeOutEnabled = false;
    _fadeInMs = 0;
    _fadeOutMs = 0;
    _fadeStartMs = millis();
    _fadeCurve = FadeCurve::Linear;
    _stopFadeActive = false;

    _positionBytes = 0;
    _durationMs = 0;
    _sourceMode = SourceMode::CallbackPCM;
    _state = State::PLAYING;
    _lastUpdateMs = millis();

    return true;
}

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

    return true;
}

bool SoundManager::startPcmOutput(uint32_t sampleRate)
{
    if (!_i2sManager.beginSpeaker(sampleRate))
        return false;

    if (!_i2sManager.startSpeaker())
        return false;

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
        return false;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        _pcmQueue[_pcmHead] = samples[i];
        _pcmHead = (_pcmHead + 1) % PCM_QUEUE_CAPACITY;
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
    if (_state != State::STOPPED)
    {
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
    _fadeCurve = FadeCurve::Linear;
    _state = State::FADING_OUT;
}

bool SoundManager::pause()
{
    if (_state != State::PLAYING)
        return false;

    if (!_i2sManager.stopSpeaker())
        return false;

    _state = State::PAUSED;
    return true;
}

bool SoundManager::resume()
{
    if (_state != State::PAUSED)
        return false;

    if (!_i2sManager.startSpeaker())
        return false;

    _state = State::PLAYING;
    _lastUpdateMs = millis();
    return true;
}

// ============================================================
// OPEN / PARSE WAV
// ============================================================

bool SoundManager::openWav(const char* path)
{
    if (!_sdManager.isReady())
        return false;

    _file = _sdManager.card().fs().open(path, FILE_READ);

    if (!_file)
        return false;

    if (!parseWav(_file))
    {
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
        return false;
    }

    bool foundFmt = false;
    bool foundData = false;

    while (file.available())
    {
        uint8_t chunkHeader[8];

        if (file.read(chunkHeader, sizeof(chunkHeader)) != sizeof(chunkHeader))
            break;

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

    const uint32_t remaining = _wav.dataSize - _positionBytes;

    size_t bytesToRead = std::min(
        static_cast<size_t>(remaining),
        static_cast<size_t>(BUFFER_BYTES)
    );

    // Keep complete frames so stereo samples never split across chunks.
    bytesToRead -= bytesToRead % _wav.blockAlign;

    if (bytesToRead == 0)
    {
        finishPlayback();
        return false;
    }

    const size_t bytesRead = _file.read(_inputBuffer, bytesToRead);

    if (bytesRead == 0)
    {
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

            memcpy(&left, _inputBuffer + (i * 4), 2);
            memcpy(&right, _inputBuffer + (i * 4) + 2, 2);

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

    if (_sourceMode == SourceMode::CallbackPCM)
    {
        if (_pcmSource == nullptr)
        {
            finishPlayback();
            return false;
        }

        samplesRead = _pcmSource(
            _pcmContext,
            _outputBuffer,
            sizeof(_outputBuffer) / sizeof(_outputBuffer[0])
        );

        if (samplesRead == 0)
        {
            finishPlayback();
            return true;
        }

        if (samplesRead > sizeof(_outputBuffer) / sizeof(_outputBuffer[0]))
        {
            finishPlayback();
            return false;
        }
    }
    else
    {
        samplesRead = std::min(
            _pcmCount,
            sizeof(_outputBuffer) / sizeof(_outputBuffer[0])
        );

        if (samplesRead == 0)
        {
            // The producer may submit more samples on the next loop.
            return true;
        }

        for (size_t i = 0; i < samplesRead; ++i)
        {
            _outputBuffer[i] = _pcmQueue[_pcmTail];
            _pcmTail = (_pcmTail + 1) % PCM_QUEUE_CAPACITY;
        }

        _pcmCount -= samplesRead;
    }

    if (!writeSamples(_outputBuffer, samplesRead))
    {
        finishPlayback();
        return false;
    }

    _positionBytes += static_cast<uint32_t>(
        samplesRead * sizeof(int16_t)
    );

    return true;
}

bool SoundManager::writeSamples(
    int16_t* samples,
    size_t sampleCount
)
{
    if (samples == nullptr || sampleCount == 0)
        return true;

    applyVolume(samples, sampleCount, getEffectiveVolume());

    const size_t outputBytes = sampleCount * sizeof(int16_t);
    size_t written = 0;

    if (!_i2sManager.writeSpeaker(
            reinterpret_cast<const uint8_t*>(samples),
            outputBytes,
            written,
            100))
    {
        return false;
    }

    return written == outputBytes;
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

    FinishedCallback callback = _finishedCallback;
    void* context = _pcmContext;

    _state = State::STOPPED;
    _sourceMode = SourceMode::None;
    _positionBytes = 0;
    _currentPath = "";
    _stopFadeActive = false;

    _pcmSource = nullptr;
    _pcmContext = nullptr;
    _finishedCallback = nullptr;
    _pcmSampleRate = 0;

    resetPcmQueue();

    if (callback != nullptr)
        callback(context);
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

    const float multiplier = volume / 100.0f;

    for (size_t i = 0; i < sampleCount; ++i)
    {
        const int32_t value = static_cast<int32_t>(
            static_cast<float>(samples[i]) * multiplier
        );

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

    return static_cast<uint8_t>(constrain(value, 0, 100));
}

void SoundManager::setStreamVolume(AudioStream stream, uint8_t volume)
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
    const uint16_t streamVol = streamVolume(_currentStream);

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

    if (_stopFadeActive && _stopFadeDurationMs > 0)
    {
        const uint32_t elapsed = now - _stopFadeStartMs;

        const float value = constrain(
            1.0f - static_cast<float>(elapsed) /
                   static_cast<float>(_stopFadeDurationMs),
            0.0f,
            1.0f
        );

        return applyFadeCurve(value);
    }

    if (_fadeInEnabled && _fadeInMs > 0)
    {
        const uint32_t elapsed = now - _fadeStartMs;

        if (elapsed < _fadeInMs)
        {
            const float value =
                static_cast<float>(elapsed) /
                static_cast<float>(_fadeInMs);

            return applyFadeCurve(value);
        }
    }

    if (_fadeOutEnabled &&
        _fadeOutMs > 0 &&
        _durationMs > 0 &&
        _sourceMode == SourceMode::Wav)
    {
        const uint32_t position = getPositionMs();
        const uint32_t fadeStart =
            _durationMs > _fadeOutMs ? _durationMs - _fadeOutMs : 0;

        if (position >= fadeStart)
        {
            const uint32_t elapsed = position - fadeStart;

            const float value = constrain(
                1.0f - static_cast<float>(elapsed) /
                       static_cast<float>(_fadeOutMs),
                0.0f,
                1.0f
            );

            return applyFadeCurve(value);
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
// DEBUG
// ============================================================

void SoundManager::printStatus()
{
    Serial0.println();
    Serial0.println("[SOUND] ---------- STATUS ----------");

    Serial0.printf("[SOUND] state:       %s\n", getStateString());
    Serial0.printf(
        "[SOUND] path:        %s\n",
        _currentPath.length() ? _currentPath.c_str() : "-"
    );
    Serial0.printf(
        "[SOUND] stream:      %u\n",
        static_cast<unsigned>(_currentStream)
    );
    Serial0.printf("[SOUND] local %%:     %u\n", _localPercent);
    Serial0.printf("[SOUND] stream vol:  %u\n", streamVolume(_currentStream));
    Serial0.printf("[SOUND] effective:   %u\n", getEffectiveVolume());
    Serial0.printf(
        "[SOUND] position:    %lu / %lu ms\n",
        static_cast<unsigned long>(getPositionMs()),
        static_cast<unsigned long>(getDurationMs())
    );

    Serial0.println("[SOUND] --------------------------------");
}
