#include "SoundManager.h"

#include <cstring>
#include <cmath>

// ============================================================
// WAV helpers
// ============================================================

static uint16_t readLE16(const uint8_t* p)
{
    return
        static_cast<uint16_t>(p[0]) |
        (static_cast<uint16_t>(p[1]) << 8);
}

static uint32_t readLE32(const uint8_t* p)
{
    return
        static_cast<uint32_t>(p[0]) |
        (static_cast<uint32_t>(p[1]) << 8) |
        (static_cast<uint32_t>(p[2]) << 16) |
        (static_cast<uint32_t>(p[3]) << 24);
}

// ============================================================
// Constructor
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
      _file(),
      _wav(),
      _positionBytes(0),
      _localVolume(100),
      _fadeEnabled(false),
      _fadeStartMs(0),
      _fadeDurationMs(0),
      _fadeCurve(FadeCurve::Linear),
      _fadeStartVolume(0),
      _fadeTargetVolume(0)
{
    _currentPath[0] = '\0';

    memset(
        _buffer,
        0,
        sizeof(_buffer)
    );
}

// ============================================================
// Destructor
// ============================================================

SoundManager::~SoundManager()
{
    end();
}

// ============================================================
// BEGIN
// ============================================================

bool SoundManager::begin()
{
    Serial.println();
    Serial.println("========================================");
    Serial.println("[SOUND] SoundManager::begin()");
    Serial.println("========================================");

    if (_initialized)
    {
        Serial.println("[SOUND] Already initialized");
        return true;
    }

    // --------------------------------------------------------
    // SD
    // --------------------------------------------------------

    Serial.println("[SOUND] Checking SD...");

    if (!_sdManager.isReady())
    {
        Serial.println("[SOUND] ERROR: SD is not ready");
        return false;
    }

    Serial.println("[SOUND] SD ready");

    // --------------------------------------------------------
    // Settings
    // --------------------------------------------------------

    Serial.printf(
        "[SOUND] Enabled: %s\n",
        _settings.enabled ? "YES" : "NO"
    );

    Serial.printf(
        "[SOUND] Global volume: %u\n",
        _settings.volume
    );

    Serial.printf(
        "[SOUND] Sample rate setting: %u kHz\n",
        _settings.sampleRate
    );

    if (!_settings.enabled)
    {
        Serial.println(
            "[SOUND] WARNING: audio disabled in settings"
        );
    }

    // --------------------------------------------------------
    // I2S manager
    // --------------------------------------------------------

    Serial.println("[SOUND] Checking I2SManager...");

    if (!_i2sManager.isInitialized())
    {
        Serial.println(
            "[SOUND] ERROR: I2SManager is not initialized"
        );

        return false;
    }

    Serial.printf(
        "[SOUND] Speaker port: I2S_NUM_%d\n",
        static_cast<int>(
            _i2sManager.speakerPort()
        )
    );

    // --------------------------------------------------------
    // Do NOT initialize speaker here with fixed 44100.
    //
    // WAV sample rate will be read from file first.
    // --------------------------------------------------------

    Serial.println(
        "[SOUND] Speaker I2S will be configured "
        "from WAV sample rate"
    );

    _initialized = true;

    Serial.println("[SOUND] SoundManager initialized");

    return true;
}

// ============================================================
// END
// ============================================================

void SoundManager::end()
{
    Serial.println("[SOUND] SoundManager::end()");

    stop();

    if (_i2sManager.isSpeakerInitialized())
    {
        Serial.println(
            "[SOUND] Releasing speaker I2S"
        );

        _i2sManager.endSpeaker();
    }

    _initialized = false;
}

// ============================================================
// IS INITIALIZED
// ============================================================

bool SoundManager::isInitialized() const
{
    return _initialized;
}

// ============================================================
// PLAY WAV
// ============================================================

bool SoundManager::playWav(const char* path)
{
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[SOUND] playWav()");
    Serial.println("----------------------------------------");

    Serial.printf(
        "[SOUND] Path: %s\n",
        path ? path : "(null)"
    );

    if (!_initialized)
    {
        Serial.println(
            "[SOUND] FAIL: SoundManager not initialized"
        );

        return false;
    }

    if (!_settings.enabled)
    {
        Serial.println(
            "[SOUND] FAIL: audio disabled in settings"
        );

        return false;
    }

    return playWavLocal(
        path,
        100
    );
}

// ============================================================
// PLAY WAV LOCAL
// ============================================================

bool SoundManager::playWavLocal(
    const char* path,
    uint8_t localVolume
)
{
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[SOUND] playWavLocal()");
    Serial.println("----------------------------------------");

    if (!path)
    {
        Serial.println(
            "[SOUND] FAIL: path == nullptr"
        );

        return false;
    }

    Serial.printf(
        "[SOUND] File: %s\n",
        path
    );

    Serial.printf(
        "[SOUND] Local volume: %u\n",
        localVolume
    );

    // --------------------------------------------------------
    // Stop previous playback
    // --------------------------------------------------------

    if (_state != State::STOPPED)
    {
        Serial.println(
            "[SOUND] Stopping previous playback"
        );

        stop();
    }

    _localVolume = localVolume;

    // --------------------------------------------------------
    // Open WAV
    // --------------------------------------------------------

    Serial.println(
        "[SOUND] Step 1: opening WAV"
    );

    if (!openWav(path))
    {
        Serial.println(
            "[SOUND] FAIL: openWav()"
        );

        return false;
    }

    Serial.println(
        "[SOUND] Step 1 OK"
    );

    // --------------------------------------------------------
    // WAV validation
    // --------------------------------------------------------

    Serial.println(
        "[SOUND] Step 2: validating WAV"
    );

    if (!validateWav())
    {
        Serial.println(
            "[SOUND] FAIL: WAV validation"
        );

        closeFile();

        return false;
    }

    Serial.println(
        "[SOUND] Step 2 OK"
    );

    printWavInfo();

    // --------------------------------------------------------
    // Configure I2S according to WAV
    // --------------------------------------------------------

    Serial.println(
        "[SOUND] Step 3: configuring speaker I2S"
    );

    if (!configureSpeaker())
    {
        Serial.println(
            "[SOUND] FAIL: configureSpeaker()"
        );

        closeFile();

        return false;
    }

    Serial.println(
        "[SOUND] Step 3 OK"
    );

    // --------------------------------------------------------
    // Seek to PCM data
    // --------------------------------------------------------

    Serial.printf(
        "[SOUND] Step 4: seeking to data offset %lu\n",
        static_cast<unsigned long>(
            _wav.dataOffset
        )
    );

    if (!_file.seek(_wav.dataOffset))
    {
        Serial.println(
            "[SOUND] FAIL: file.seek(dataOffset)"
        );

        closeFile();

        return false;
    }

    _positionBytes = 0;

    Serial.println(
        "[SOUND] Step 4 OK"
    );

    // --------------------------------------------------------
    // Start I2S
    // --------------------------------------------------------

    if (!_i2sManager.isSpeakerInitialized())
    {
        Serial.println(
            "[SOUND] FAIL: speaker I2S is not initialized"
        );

        closeFile();

        return false;
    }

    i2s_port_t port = _i2sManager.speakerPort();

    Serial.printf(
        "[SOUND] Starting I2S port I2S_NUM_%d\n",
        static_cast<int>(port)
    );

    esp_err_t err = i2s_start(port);

    if (err != ESP_OK)
    {
        Serial.printf(
            "[SOUND] FAIL: i2s_start() = %s\n",
            esp_err_to_name(err)
        );

        closeFile();

        return false;
    }

    // --------------------------------------------------------
    // State
    // --------------------------------------------------------

    _state = State::PLAYING;

    strncpy(
        _currentPath,
        path,
        sizeof(_currentPath) - 1
    );

    _currentPath[
        sizeof(_currentPath) - 1
    ] = '\0';

    Serial.println(
        "[SOUND] Playback started successfully"
    );

    Serial.printf(
        "[SOUND] Effective volume: %u\n",
        getEffectiveVolume()
    );

    return true;
}

// ============================================================
// PLAY ALARM
// ============================================================

bool SoundManager::playAlarm(
    const char* path,
    uint8_t localVolume,
    uint32_t fadeInMs,
    uint32_t fadeOutMs,
    FadeCurve curve
)
{
    Serial.println();
    Serial.println("----------------------------------------");
    Serial.println("[SOUND] playAlarm()");
    Serial.println("----------------------------------------");

    Serial.printf(
        "[SOUND] Path: %s\n",
        path ? path : "(null)"
    );

    Serial.printf(
        "[SOUND] Local volume: %u\n",
        localVolume
    );

    Serial.printf(
        "[SOUND] Fade in: %lu ms\n",
        static_cast<unsigned long>(fadeInMs)
    );

    Serial.printf(
        "[SOUND] Fade out: %lu ms\n",
        static_cast<unsigned long>(fadeOutMs)
    );

    if (!playWavLocal(
        path,
        localVolume
    ))
    {
        return false;
    }

    _fadeEnabled = false;

    if (fadeInMs > 0)
    {
        _fadeEnabled = true;

        _fadeStartMs = millis();
        _fadeDurationMs = fadeInMs;

        _fadeCurve = curve;

        _fadeStartVolume = 0;
        _fadeTargetVolume = localVolume;

        _state = State::FADING_IN;
    }

    return true;
}

// ============================================================
// UPDATE
// ============================================================

void SoundManager::update()
{
    if (!_initialized)
        return;

    if (
        _state == State::STOPPED ||
        _state == State::PAUSED
    )
    {
        return;
    }

    updateFade();

    if (_state == State::FADING_IN)
    {
        // Playback continues.
    }

    if (_state == State::FADING_OUT)
    {
        // Playback continues.
    }

    // --------------------------------------------------------
    // Read PCM
    // --------------------------------------------------------

    if (!readAndPlayChunk())
    {
        return;
    }
}

// ============================================================
// READ AND PLAY CHUNK
// ============================================================

bool SoundManager::readAndPlayChunk()
{
    if (!_file)
    {
        printError(
            "readAndPlayChunk(): file invalid"
        );

        finishPlayback();

        return false;
    }

    uint32_t remaining =
        _wav.dataSize - _positionBytes;

    if (remaining == 0)
    {
        Serial.println(
            "[SOUND] End of WAV data"
        );

        finishPlayback();

        return false;
    }

    size_t toRead =
        remaining > BUFFER_SIZE
            ? BUFFER_SIZE
            : remaining;

    // Align to sample frame.
    if (_wav.blockAlign > 0)
    {
        toRead -=
            toRead % _wav.blockAlign;
    }

    if (toRead == 0)
    {
        Serial.println(
            "[SOUND] ERROR: aligned read size == 0"
        );

        finishPlayback();

        return false;
    }

    size_t bytesRead =
        _file.read(
            _buffer,
            toRead
        );

    if (bytesRead == 0)
    {
        Serial.println(
            "[SOUND] ERROR: file.read() returned 0"
        );

        finishPlayback();

        return false;
    }

    // --------------------------------------------------------
    // PCM16 processing
    // --------------------------------------------------------

    if (_wav.bitsPerSample == 16)
    {
        int16_t* samples =
            reinterpret_cast<int16_t*>(
                _buffer
            );

        size_t sampleCount =
            bytesRead / sizeof(int16_t);

        uint8_t volume =
            calculateCurrentVolume();

        applyVolume(
            samples,
            sampleCount,
            volume
        );

        // ----------------------------------------------------
        // Stereo -> mono
        // ----------------------------------------------------

        if (_wav.channels == 2)
        {
            downmixStereoToMono(
                samples,
                sampleCount
            );

            size_t monoSamples =
                sampleCount / 2;

            bytesRead =
                monoSamples *
                sizeof(int16_t);
        }
    }

    // --------------------------------------------------------
    // Write to I2S
    // --------------------------------------------------------

    if (!writeAudio(
        _buffer,
        bytesRead
    ))
    {
        Serial.println(
            "[SOUND] ERROR: writeAudio() failed"
        );

        finishPlayback();

        return false;
    }

    _positionBytes +=
        static_cast<uint32_t>(
            _wav.blockAlign *
            (
                _wav.channels == 2
                    ? (bytesRead / 2)
                    : (bytesRead / 2)
            )
        );

    // Protect position from overflow.
    if (_positionBytes > _wav.dataSize)
    {
        _positionBytes = _wav.dataSize;
    }

    if (_positionBytes >= _wav.dataSize)
    {
        Serial.println(
            "[SOUND] WAV playback completed"
        );

        finishPlayback();
    }

    return true;
}

// ============================================================
// WRITE AUDIO
// ============================================================

bool SoundManager::writeAudio(
    const uint8_t* data,
    size_t bytes
)
{
    if (!data || bytes == 0)
        return false;

    if (!_i2sManager.isSpeakerInitialized())
    {
        Serial.println(
            "[SOUND] writeAudio: speaker not initialized"
        );

        return false;
    }

    size_t totalWritten = 0;

    i2s_port_t port =
        _i2sManager.speakerPort();

    esp_err_t err =
        i2s_write(
            port,
            data,
            bytes,
            &totalWritten,
            0
        );

    if (err != ESP_OK)
    {
        Serial.printf(
            "[SOUND] i2s_write ERROR: %s\n",
            esp_err_to_name(err)
        );

        return false;
    }

    if (totalWritten == 0)
    {
        Serial.println(
            "[SOUND] i2s_write wrote 0 bytes"
        );

        return false;
    }

    return true;
}

// ============================================================
// OPEN WAV
// ============================================================

bool SoundManager::openWav(
    const char* path
)
{
    Serial.printf(
        "[SOUND] openWav(): %s\n",
        path
    );

    fs::FS& fs =
        _sdManager.card().fs();

    Serial.println(
        "[SOUND] Opening file..."
    );

    _file = fs.open(
        path,
        FILE_READ
    );

    if (!_file)
    {
        Serial.println(
            "[SOUND] ERROR: fs.open() failed"
        );

        Serial.printf(
            "[SOUND] Path checked: %s\n",
            path
        );

        return false;
    }

    Serial.printf(
        "[SOUND] File opened, size: %lu bytes\n",
        static_cast<unsigned long>(
            _file.size()
        )
    );

    if (_file.size() < 44)
    {
        Serial.println(
            "[SOUND] ERROR: file smaller than WAV header"
        );

        _file.close();

        return false;
    }

    Serial.println(
        "[SOUND] Parsing WAV header..."
    );

    if (!parseWavHeader(_file))
    {
        Serial.println(
            "[SOUND] ERROR: parseWavHeader() failed"
        );

        _file.close();

        return false;
    }

    return true;
}

// ============================================================
// PARSE WAV HEADER
// ============================================================

bool SoundManager::parseWavHeader(
    File& file
)
{
    _wav = WavInfo();

    if (!file.seek(0))
    {
        Serial.println(
            "[SOUND] WAV: seek(0) failed"
        );

        return false;
    }

    uint8_t riff[12];

    if (file.read(
        riff,
        sizeof(riff)
    ) != sizeof(riff))
    {
        Serial.println(
            "[SOUND] WAV: cannot read RIFF header"
        );

        return false;
    }

    Serial.printf(
        "[SOUND] RIFF: %c%c%c%c\n",
        riff[0],
        riff[1],
        riff[2],
        riff[3]
    );

    Serial.printf(
        "[SOUND] Format: %c%c%c%c\n",
        riff[8],
        riff[9],
        riff[10],
        riff[11]
    );

    if (
        memcmp(
            riff,
            "RIFF",
            4
        ) != 0
    )
    {
        Serial.println(
            "[SOUND] ERROR: missing RIFF"
        );

        return false;
    }

    if (
        memcmp(
            riff + 8,
            "WAVE",
            4
        ) != 0
    )
    {
        Serial.println(
            "[SOUND] ERROR: missing WAVE"
        );

        return false;
    }

    bool foundFmt = false;
    bool foundData = false;

    while (file.position() + 8 <= file.size())
    {
        uint8_t chunkHeader[8];

        if (
            file.read(
                chunkHeader,
                8
            ) != 8
        )
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
            readLE32(
                chunkHeader + 4
            );

        uint32_t chunkDataPosition =
            file.position();

        Serial.printf(
            "[SOUND] Chunk '%s', size=%lu, offset=%lu\n",
            chunkId,
            static_cast<unsigned long>(chunkSize),
            static_cast<unsigned long>(
                chunkDataPosition
            )
        );

        // ----------------------------------------------------
        // fmt
        // ----------------------------------------------------

        if (
            memcmp(
                chunkId,
                "fmt ",
                4
            ) == 0
        )
        {
            if (!parseFmtChunk(
                file,
                chunkSize
            ))
            {
                return false;
            }

            foundFmt = true;
        }

        // ----------------------------------------------------
        // data
        // ----------------------------------------------------

        else if (
            memcmp(
                chunkId,
                "data",
                4
            ) == 0
        )
        {
            _wav.dataOffset =
                chunkDataPosition;

            _wav.dataSize =
                chunkSize;

            foundData = true;

            Serial.printf(
                "[SOUND] PCM data offset: %lu\n",
                static_cast<unsigned long>(
                    _wav.dataOffset
                )
            );

            Serial.printf(
                "[SOUND] PCM data size: %lu\n",
                static_cast<unsigned long>(
                    _wav.dataSize
                )
            );

            // We can stop after fmt + data.
            if (foundFmt)
            {
                break;
            }
        }

        // ----------------------------------------------------
        // Skip unknown chunk
        // ----------------------------------------------------

        uint32_t nextPosition =
            chunkDataPosition +
            chunkSize;

        // WAV chunks are word aligned.
        if (chunkSize & 1)
        {
            nextPosition++;
        }

        if (!file.seek(nextPosition))
        {
            Serial.println(
                "[SOUND] ERROR: cannot skip WAV chunk"
            );

            return false;
        }
    }

    if (!foundFmt)
    {
        Serial.println(
            "[SOUND] ERROR: fmt chunk not found"
        );

        return false;
    }

    if (!foundData)
    {
        Serial.println(
            "[SOUND] ERROR: data chunk not found"
        );

        return false;
    }

    _wav.valid = true;

    return true;
}

// ============================================================
// PARSE FMT
// ============================================================

bool SoundManager::parseFmtChunk(
    File& file,
    uint32_t chunkSize
)
{
    if (chunkSize < 16)
    {
        Serial.println(
            "[SOUND] ERROR: fmt chunk < 16 bytes"
        );

        return false;
    }

    uint8_t fmt[16];

    if (
        file.read(
            fmt,
            16
        ) != 16
    )
    {
        Serial.println(
            "[SOUND] ERROR: cannot read fmt chunk"
        );

        return false;
    }

    _wav.audioFormat =
        readLE16(fmt + 0);

    _wav.channels =
        readLE16(fmt + 2);

    _wav.sampleRate =
        readLE32(fmt + 4);

    _wav.byteRate =
        readLE32(fmt + 8);

    _wav.blockAlign =
        readLE16(fmt + 12);

    _wav.bitsPerSample =
        readLE16(fmt + 14);

    Serial.println(
        "[SOUND] fmt parsed:"
    );

    Serial.printf(
        "         format       = %u\n",
        _wav.audioFormat
    );

    Serial.printf(
        "         channels     = %u\n",
        _wav.channels
    );

    Serial.printf(
        "         sample rate  = %lu\n",
        static_cast<unsigned long>(
            _wav.sampleRate
        )
    );

    Serial.printf(
        "         byte rate    = %lu\n",
        static_cast<unsigned long>(
            _wav.byteRate
        )
    );

    Serial.printf(
        "         block align  = %u\n",
        _wav.blockAlign
    );

    Serial.printf(
        "         bits/sample  = %u\n",
        _wav.bitsPerSample
    );

    // Skip any extended fmt data.
    if (chunkSize > 16)
    {
        uint32_t extra =
            chunkSize - 16;

        if (
            !file.seek(
                file.position() + extra
            )
        )
        {
            Serial.println(
                "[SOUND] ERROR: fmt skip failed"
            );

            return false;
        }
    }

    return true;
}

// ============================================================
// VALIDATE WAV
// ============================================================

bool SoundManager::validateWav() const
{
    if (!_wav.valid)
    {
        Serial.println(
            "[SOUND] WAV invalid flag"
        );

        return false;
    }

    if (_wav.audioFormat != 1)
    {
        Serial.printf(
            "[SOUND] ERROR: unsupported audio format: %u\n",
            _wav.audioFormat
        );

        return false;
    }

    if (
        _wav.channels != 1 &&
        _wav.channels != 2
    )
    {
        Serial.printf(
            "[SOUND] ERROR: unsupported channels: %u\n",
            _wav.channels
        );

        return false;
    }

    if (_wav.bitsPerSample != 16)
    {
        Serial.printf(
            "[SOUND] ERROR: unsupported bits/sample: %u\n",
            _wav.bitsPerSample
        );

        return false;
    }

    if (_wav.sampleRate == 0)
    {
        Serial.println(
            "[SOUND] ERROR: sample rate == 0"
        );

        return false;
    }

    if (_wav.blockAlign == 0)
    {
        Serial.println(
            "[SOUND] ERROR: block align == 0"
        );

        return false;
    }

    if (_wav.dataSize == 0)
    {
        Serial.println(
            "[SOUND] ERROR: data size == 0"
        );

        return false;
    }

    if (
        _wav.dataOffset +
        _wav.dataSize >
        _file.size()
    )
    {
        Serial.printf(
            "[SOUND] ERROR: data extends beyond file "
            "(offset=%lu size=%lu file=%lu)\n",
            static_cast<unsigned long>(
                _wav.dataOffset
            ),
            static_cast<unsigned long>(
                _wav.dataSize
            ),
            static_cast<unsigned long>(
                _file.size()
            )
        );

        return false;
    }

    return true;
}

// ============================================================
// CONFIGURE SPEAKER
// ============================================================

bool SoundManager::configureSpeaker()
{
    Serial.printf(
        "[SOUND] Configuring speaker for %lu Hz\n",
        static_cast<unsigned long>(
            _wav.sampleRate
        )
    );

    if (
        !isSupportedSampleRate(
            _wav.sampleRate
        )
    )
    {
        Serial.printf(
            "[SOUND] WARNING: unusual sample rate %lu\n",
            static_cast<unsigned long>(
                _wav.sampleRate
            )
        );
    }

    // --------------------------------------------------------
    // If already initialized, release current configuration.
    // --------------------------------------------------------

    if (_i2sManager.isSpeakerInitialized())
    {
        Serial.println(
            "[SOUND] Speaker I2S already initialized"
        );

        Serial.println(
            "[SOUND] Reconfiguring speaker"
        );

        _i2sManager.endSpeaker();
    }

    // --------------------------------------------------------
    // Start with WAV sample rate.
    // --------------------------------------------------------

    if (
        !_i2sManager.beginSpeaker(
            _wav.sampleRate
        )
    )
    {
        Serial.printf(
            "[SOUND] ERROR: beginSpeaker(%lu) failed\n",
            static_cast<unsigned long>(
                _wav.sampleRate
            )
        );

        return false;
    }

    Serial.printf(
        "[SOUND] Speaker I2S configured: %lu Hz\n",
        static_cast<unsigned long>(
            _wav.sampleRate
        )
    );

    return true;
}

// ============================================================
// SUPPORTED SAMPLE RATE
// ============================================================

bool SoundManager::isSupportedSampleRate(
    uint32_t sampleRate
) const
{
    switch (sampleRate)
    {
        case 8000:
        case 11025:
        case 16000:
        case 22050:
        case 32000:
        case 44100:
        case 48000:
        case 96000:
            return true;

        default:
            return false;
    }
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

    for (size_t i = 0; i < sampleCount; ++i)
    {
        int32_t value =
            samples[i];

        value =
            (
                value *
                static_cast<int32_t>(volume)
            ) / 100;

        if (value > 32767)
            value = 32767;

        if (value < -32768)
            value = -32768;

        samples[i] =
            static_cast<int16_t>(
                value
            );
    }
}

// ============================================================
// DOWNMIX STEREO
// ============================================================

void SoundManager::downmixStereoToMono(
    int16_t* samples,
    size_t sampleCount
)
{
    if (!samples)
        return;

    size_t frames =
        sampleCount / 2;

    for (size_t i = 0; i < frames; ++i)
    {
        int32_t left =
            samples[i * 2];

        int32_t right =
            samples[i * 2 + 1];

        int32_t mono =
            (left + right) / 2;

        if (mono > 32767)
            mono = 32767;

        if (mono < -32768)
            mono = -32768;

        samples[i] =
            static_cast<int16_t>(
                mono
            );
    }
}

// ============================================================
// VOLUME
// ============================================================

uint8_t SoundManager::getEffectiveVolume() const
{
    uint16_t global =
        _settings.volume;

    uint16_t local =
        _localVolume;

    return static_cast<uint8_t>(
        (
            global *
            local
        ) / 100
    );
}

// ============================================================
// CURRENT VOLUME
// ============================================================

uint8_t SoundManager::calculateCurrentVolume() const
{
    uint8_t target =
        getEffectiveVolume();

    if (!_fadeEnabled)
        return target;

    uint32_t elapsed =
        millis() -
        _fadeStartMs;

    if (
        elapsed >=
        _fadeDurationMs
    )
    {
        return _fadeTargetVolume;
    }

    float progress =
        static_cast<float>(elapsed) /
        static_cast<float>(_fadeDurationMs);

    float factor =
        calculateFade(progress);

    float value =
        static_cast<float>(
            _fadeStartVolume
        ) +
        (
            static_cast<float>(
                _fadeTargetVolume -
                _fadeStartVolume
            ) *
            factor
        );

    if (value < 0.0f)
        value = 0.0f;

    if (value > 100.0f)
        value = 100.0f;

    return static_cast<uint8_t>(
        value
    );
}

// ============================================================
// FADE CURVE
// ============================================================

float SoundManager::calculateFade(
    float progress
) const
{
    if (progress <= 0.0f)
        return 0.0f;

    if (progress >= 1.0f)
        return 1.0f;

    switch (_fadeCurve)
    {
        case FadeCurve::Linear:
            return progress;

        case FadeCurve::Exponential:
            return
                progress *
                progress;

        case FadeCurve::Logarithmic:
            return
                1.0f -
                (
                    (1.0f - progress) *
                    (1.0f - progress)
                );
    }

    return progress;
}

// ============================================================
// UPDATE FADE
// ============================================================

void SoundManager::updateFade()
{
    if (!_fadeEnabled)
        return;

    uint32_t elapsed =
        millis() -
        _fadeStartMs;

    if (
        elapsed <
        _fadeDurationMs
    )
    {
        return;
    }

    _fadeEnabled = false;

    if (
        _state ==
        State::FADING_IN
    )
    {
        _state = State::PLAYING;

        Serial.println(
            "[SOUND] Fade-in completed"
        );
    }

    else if (
        _state ==
        State::FADING_OUT
    )
    {
        Serial.println(
            "[SOUND] Fade-out completed"
        );

        finishPlayback();
    }
}

// ============================================================
// STOP
// ============================================================

void SoundManager::stop()
{
    if (
        _state ==
        State::STOPPED
    )
    {
        return;
    }

    Serial.println(
        "[SOUND] stop()"
    );

    if (_i2sManager.isSpeakerInitialized())
    {
        i2s_port_t port =
            _i2sManager.speakerPort();

        i2s_stop(port);

        i2s_zero_dma_buffer(port);
    }

    closeFile();

    _state =
        State::STOPPED;

    _fadeEnabled = false;

    _positionBytes = 0;

    _currentPath[0] = '\0';
}

// ============================================================
// PAUSE
// ============================================================

bool SoundManager::pause()
{
    if (_state != State::PLAYING)
    {
        Serial.println(
            "[SOUND] pause(): not playing"
        );

        return false;
    }

    if (
        !_i2sManager.isSpeakerInitialized()
    )
    {
        return false;
    }

    esp_err_t err =
        i2s_stop(
            _i2sManager.speakerPort()
        );

    if (err != ESP_OK)
    {
        Serial.printf(
            "[SOUND] pause: i2s_stop failed: %s\n",
            esp_err_to_name(err)
        );

        return false;
    }

    _state =
        State::PAUSED;

    Serial.println(
        "[SOUND] PAUSED"
    );

    return true;
}

// ============================================================
// RESUME
// ============================================================

bool SoundManager::resume()
{
    if (_state != State::PAUSED)
    {
        Serial.println(
            "[SOUND] resume(): not paused"
        );

        return false;
    }

    if (
        !_i2sManager.isSpeakerInitialized()
    )
    {
        return false;
    }

    esp_err_t err =
        i2s_start(
            _i2sManager.speakerPort()
        );

    if (err != ESP_OK)
    {
        Serial.printf(
            "[SOUND] resume: i2s_start failed: %s\n",
            esp_err_to_name(err)
        );

        return false;
    }

    _state =
        State::PLAYING;

    Serial.println(
        "[SOUND] RESUMED"
    );

    return true;
}

// ============================================================
// FINISH PLAYBACK
// ============================================================

void SoundManager::finishPlayback()
{
    Serial.println(
        "[SOUND] finishPlayback()"
    );

    if (
        _i2sManager.isSpeakerInitialized()
    )
    {
        i2s_stop(
            _i2sManager.speakerPort()
        );

        i2s_zero_dma_buffer(
            _i2sManager.speakerPort()
        );
    }

    closeFile();

    _state =
        State::STOPPED;

    _fadeEnabled = false;
}

// ============================================================
// CLOSE FILE
// ============================================================

void SoundManager::closeFile()
{
    if (_file)
    {
        _file.close();
    }
}

// ============================================================
// LOCAL VOLUME
// ============================================================

void SoundManager::setLocalVolume(
    uint8_t volume
)
{
    if (volume > 100)
        volume = 100;

    _localVolume =
        volume;
}

// ============================================================
// GET LOCAL VOLUME
// ============================================================

uint8_t SoundManager::getLocalVolume() const
{
    return _localVolume;
}

// ============================================================
// STATE
// ============================================================

bool SoundManager::isPlaying() const
{
    return
        _state == State::PLAYING ||
        _state == State::FADING_IN ||
        _state == State::FADING_OUT;
}

bool SoundManager::isPaused() const
{
    return _state == State::PAUSED;
}

SoundManager::State SoundManager::getState() const
{
    return _state;
}

// ============================================================
// POSITION
// ============================================================

uint32_t SoundManager::getPositionMs() const
{
    if (
        _wav.byteRate == 0
    )
    {
        return 0;
    }

    return
        (
            _positionBytes *
            1000UL
        ) /
        _wav.byteRate;
}

// ============================================================
// DURATION
// ============================================================

uint32_t SoundManager::getDurationMs() const
{
    if (
        _wav.byteRate == 0
    )
    {
        return 0;
    }

    return
        (
            _wav.dataSize *
            1000UL
        ) /
        _wav.byteRate;
}

// ============================================================
// POSITION BYTES
// ============================================================

uint32_t SoundManager::getPositionBytes() const
{
    return _positionBytes;
}

// ============================================================
// DATA BYTES
// ============================================================

uint32_t SoundManager::getDataBytes() const
{
    return _wav.dataSize;
}

// ============================================================
// WAV INFO
// ============================================================

uint32_t SoundManager::getSampleRate() const
{
    return _wav.sampleRate;
}

uint16_t SoundManager::getChannels() const
{
    return _wav.channels;
}

uint16_t SoundManager::getBitsPerSample() const
{
    return _wav.bitsPerSample;
}

uint16_t SoundManager::getAudioFormat() const
{
    return _wav.audioFormat;
}

// ============================================================
// PRINT WAV INFO
// ============================================================

void SoundManager::printWavInfo() const
{
    Serial.println();
    Serial.println(
        "[SOUND] ========== WAV INFO =========="
    );

    Serial.printf(
        "[SOUND] Format:       %u\n",
        _wav.audioFormat
    );

    Serial.printf(
        "[SOUND] Channels:     %u\n",
        _wav.channels
    );

    Serial.printf(
        "[SOUND] Sample rate:  %lu Hz\n",
        static_cast<unsigned long>(
            _wav.sampleRate
        )
    );

    Serial.printf(
        "[SOUND] Byte rate:    %lu\n",
        static_cast<unsigned long>(
            _wav.byteRate
        )
    );

    Serial.printf(
        "[SOUND] Block align:  %u\n",
        _wav.blockAlign
    );

    Serial.printf(
        "[SOUND] Bits/sample:  %u\n",
        _wav.bitsPerSample
    );

    Serial.printf(
        "[SOUND] Data offset:  %lu\n",
        static_cast<unsigned long>(
            _wav.dataOffset
        )
    );

    Serial.printf(
        "[SOUND] Data size:    %lu\n",
        static_cast<unsigned long>(
            _wav.dataSize
        )
    );

    Serial.printf(
        "[SOUND] Duration:     %lu ms\n",
        static_cast<unsigned long>(
            getDurationMs()
        )
    );

    Serial.println(
        "[SOUND] ================================="
    );
    Serial.println();
}

// ============================================================
// STATUS
// ============================================================

void SoundManager::printStatus() const
{
    Serial.println();
    Serial.println(
        "[SOUND] ---------- STATUS ----------"
    );

    Serial.printf(
        "[SOUND] Initialized: %s\n",
        _initialized ? "YES" : "NO"
    );

    Serial.printf(
        "[SOUND] Playing: %s\n",
        isPlaying() ? "YES" : "NO"
    );

    Serial.printf(
        "[SOUND] Paused: %s\n",
        isPaused() ? "YES" : "NO"
    );

    Serial.printf(
        "[SOUND] Local volume: %u\n",
        _localVolume
    );

    Serial.printf(
        "[SOUND] Global volume: %u\n",
        _settings.volume
    );

    Serial.printf(
        "[SOUND] Effective volume: %u\n",
        getEffectiveVolume()
    );

    Serial.printf(
        "[SOUND] Position: %lu / %lu ms\n",
        static_cast<unsigned long>(
            getPositionMs()
        ),
        static_cast<unsigned long>(
            getDurationMs()
        )
    );

    Serial.println(
        "[SOUND] --------------------------------"
    );
}

// ============================================================
// ERROR
// ============================================================

void SoundManager::printError(
    const char* message
) const
{
    Serial.printf(
        "[SOUND] ERROR: %s\n",
        message
    );
}
