
#include "MusicGenerator.h"

#include <cmath>
#include <algorithm>

// ============================================================
// CONFIGURATION
// ============================================================

namespace
{
    constexpr float VOLUME_SMOOTHING = 0.0005f;
    constexpr float MIX_GAIN = 0.78f;

    constexpr float MIN_BPM = 40.0f;
    constexpr float MAX_BPM = 180.0f;

    // Ограничение PCM перед преобразованием в int16_t.
    constexpr float PCM_LIMIT = 0.98f;

    constexpr float VOICE_EPSILON = 0.00001f;

    // --------------------------------------------------------
    // MELODY — 16 BARS / 64 BEATS
    // --------------------------------------------------------

    constexpr MusicGenerator::NoteEvent MELODY[] =
    {
        // INTRO — bars 1-4
        {  0, 76, 2, 0.62f },
        {  2, 79, 1, 0.52f },
        {  3, 81, 2, 0.58f },
        {  5, 79, 1, 0.46f },
        {  6, 76, 2, 0.54f },

        {  8, 72, 2, 0.48f },
        { 10, 76, 2, 0.54f },
        { 12, 79, 4, 0.62f },

        // THEME — bars 5-8
        { 16, 81, 2, 0.59f },
        { 18, 84, 2, 0.66f },
        { 20, 81, 1, 0.50f },
        { 21, 79, 3, 0.56f },

        { 24, 76, 2, 0.53f },
        { 26, 79, 2, 0.58f },
        { 28, 81, 4, 0.64f },

        // DEVELOPMENT — bars 9-12
        { 32, 79, 2, 0.58f },
        { 34, 76, 2, 0.51f },
        { 36, 72, 2, 0.46f },
        { 38, 76, 2, 0.54f },

        { 40, 79, 2, 0.57f },
        { 42, 81, 2, 0.61f },
        { 44, 84, 4, 0.66f },

        // RETURN — bars 13-16
        { 48, 81, 2, 0.59f },
        { 50, 79, 2, 0.54f },
        { 52, 76, 2, 0.50f },
        { 54, 72, 2, 0.46f },

        { 56, 74, 2, 0.48f },
        { 58, 76, 2, 0.52f },
        { 60, 72, 4, 0.58f }
    };

    constexpr size_t MELODY_COUNT =
        sizeof(MELODY) / sizeof(MELODY[0]);

    const char* voiceTypeName(MusicGenerator::VoiceType type)
    {
        switch (type)
        {
            case MusicGenerator::VoiceType::Piano:
                return "Piano";

            case MusicGenerator::VoiceType::Bass:
                return "Bass";

            case MusicGenerator::VoiceType::Pad:
                return "Pad";

            case MusicGenerator::VoiceType::Harp:
                return "Harp";

            case MusicGenerator::VoiceType::Bell:
                return "Bell";

            default:
                return "Unknown";
        }
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

MusicGenerator::MusicGenerator() = default;

// ============================================================
// BEGIN
// ============================================================

void MusicGenerator::begin(uint32_t sampleRate)
{
    if (sampleRate == 0)
    {
        Serial0.printf(
            "[MUSIC][WARNING] Invalid sample rate: %lu. Using %lu.\n",
            static_cast<unsigned long>(sampleRate),
            static_cast<unsigned long>(DEFAULT_SAMPLE_RATE)
        );

        sampleRate = DEFAULT_SAMPLE_RATE;
    }

    const uint32_t previousSampleRate = _sampleRate;

    _sampleRate = sampleRate;

    updateTiming();
    clearVoices();

    _playing = false;
    _beat = 0;
    _melodyIndex = 0;
    _samplesUntilBeat = 0.0f;
    _volume = _targetVolume;

    _initialized = true;

    Serial0.println("[MUSIC] Initialized");
    Serial0.printf(
        "[MUSIC] Sample rate: %lu Hz\n",
        static_cast<unsigned long>(_sampleRate)
    );
    Serial0.printf("[MUSIC] BPM: %.1f\n", _bpm);
    Serial0.printf("[MUSIC] Volume: %.2f\n", _targetVolume);
    Serial0.printf("[MUSIC] Channels: %u\n",
                  static_cast<unsigned>(CHANNELS));
    Serial0.printf("[MUSIC] Voices: %u\n",
                  static_cast<unsigned>(MAX_VOICES));

    if (previousSampleRate != _sampleRate)
    {
        Serial0.printf(
            "[MUSIC] Sample rate changed: %lu -> %lu Hz\n",
            static_cast<unsigned long>(previousSampleRate),
            static_cast<unsigned long>(_sampleRate)
        );
    }
}

// ============================================================
// END
// ============================================================

void MusicGenerator::end()
{
    if (!_initialized)
    {
        Serial0.println("[MUSIC] end(): already stopped");
        return;
    }

    stop();

    _initialized = false;

    Serial0.println("[MUSIC] Deinitialized");
}

bool MusicGenerator::isInitialized() const
{
    return _initialized;
}

// ============================================================
// START
// ============================================================

void MusicGenerator::start()
{
    if (!_initialized)
    {
        Serial0.println(
            "[MUSIC][WARNING] start() ignored: generator not initialized"
        );
        return;
    }

    if (_playing)
    {
        Serial0.println("[MUSIC] start(): already playing");
        return;
    }

    clearVoices();

    _beat = 0;
    _melodyIndex = 0;
    _samplesUntilBeat = 0.0f;

    _playing = true;

    Serial0.println("[MUSIC] Playback started");
    Serial0.printf("[MUSIC] BPM: %.1f\n", _bpm);
    Serial0.printf("[MUSIC] Volume: %.2f\n", _targetVolume);
}

// ============================================================
// STOP
// ============================================================

void MusicGenerator::stop()
{
    const bool wasPlaying = _playing;

    _playing = false;

    clearVoices();

    _beat = 0;
    _melodyIndex = 0;
    _samplesUntilBeat = 0.0f;

    if (wasPlaying)
    {
        Serial0.println("[MUSIC] Playback stopped");
    }
    else
    {
        Serial0.println("[MUSIC] Stop requested while idle");
    }
}

bool MusicGenerator::isPlaying() const
{
    return _playing;
}

// ============================================================
// VOLUME
// ============================================================

void MusicGenerator::setVolume(float volume)
{
    if (!std::isfinite(volume))
    {
        Serial0.println("[MUSIC][WARNING] Ignored non-finite volume");
        return;
    }

    volume = clamp01(volume);

    if (std::fabs(_targetVolume - volume) < 0.0001f)
        return;

    const float previousVolume = _targetVolume;

    _targetVolume = volume;

    Serial0.printf(
        "[MUSIC] Target volume changed: %.3f -> %.3f\n",
        previousVolume,
        _targetVolume
    );
}

float MusicGenerator::volume() const
{
    return _targetVolume;
}

// ============================================================
// BPM
// ============================================================

void MusicGenerator::setBpm(float bpm)
{
    if (!std::isfinite(bpm))
    {
        Serial0.println("[MUSIC][WARNING] Ignored non-finite BPM");
        return;
    }

    const float requestedBpm = bpm;

    bpm = std::max(MIN_BPM, std::min(MAX_BPM, bpm));

    if (std::fabs(_bpm - bpm) < 0.001f)
        return;

    const float previousBpm = _bpm;

    _bpm = bpm;

    updateTiming();

    Serial0.printf(
        "[MUSIC] BPM changed: %.2f -> %.2f\n",
        previousBpm,
        _bpm
    );

    if (requestedBpm != bpm)
    {
        Serial0.printf(
            "[MUSIC] BPM clamped to range %.0f-%.0f\n",
            MIN_BPM,
            MAX_BPM
        );
    }
}

float MusicGenerator::bpm() const
{
    return _bpm;
}

uint32_t MusicGenerator::sampleRate() const
{
    return _sampleRate;
}

// ============================================================
// TIMING
// ============================================================

void MusicGenerator::updateTiming()
{
    if (_sampleRate == 0 || _bpm <= 0.0f)
    {
        _samplesPerBeat = 0.0f;

        Serial0.println("[MUSIC][ERROR] Invalid timing parameters");
        return;
    }

    _samplesPerBeat =
        static_cast<float>(_sampleRate) * 60.0f / _bpm;

    Serial0.printf(
        "[MUSIC] Samples per beat: %.2f\n",
        _samplesPerBeat
    );
}

// ============================================================
// GENERATE PCM BLOCK
// ============================================================

void MusicGenerator::generateBlock(
    int16_t* buffer,
    size_t sampleCount
)
{
    if (buffer == nullptr || sampleCount == 0)
        return;

    if (!_initialized || !_playing || _samplesPerBeat <= 0.0f)
    {
        for (size_t i = 0; i < sampleCount; ++i)
            buffer[i] = 0;

        return;
    }

    for (size_t i = 0; i < sampleCount; ++i)
    {
        // ----------------------------------------------------
        // BEAT SCHEDULER
        // ----------------------------------------------------

        if (_samplesUntilBeat <= 0.0f)
        {
            triggerBeat();
            _samplesUntilBeat += _samplesPerBeat;
        }

        _samplesUntilBeat -= 1.0f;

        // ----------------------------------------------------
        // MASTER VOLUME RAMP
        // ----------------------------------------------------

        _volume +=
            (_targetVolume - _volume) * VOLUME_SMOOTHING;

        if (std::fabs(_targetVolume - _volume) < 0.00001f)
            _volume = _targetVolume;

        // ----------------------------------------------------
        // RENDER MIX
        // ----------------------------------------------------

        float sample = renderSample();

        sample *= _volume;
        sample *= MIX_GAIN;

        sample = std::max(
            -PCM_LIMIT,
            std::min(PCM_LIMIT, sample)
        );

        buffer[i] = static_cast<int16_t>(
            sample * 32767.0f
        );
    }
}

// ============================================================
// BEAT
// ============================================================

void MusicGenerator::triggerBeat()
{
    if (_beat == 0)
    {
        _melodyIndex = 0;
        Serial0.println("[MUSIC] Loop started");
    }

    // Chord changes every four bars = 16 beats.
    if ((_beat % 16U) == 0U)
        triggerHarmony();

    triggerBass();
    triggerHarp();
    triggerMelody();
    triggerBell();

    ++_beat;

    if (_beat >= LOOP_BEATS)
    {
        _beat = 0;
        _melodyIndex = 0;

        Serial0.println("[MUSIC] Loop completed");
    }
}

// ============================================================
// CHORD PROGRESSION
// ============================================================

MusicGenerator::Chord MusicGenerator::chordForBar(
    uint32_t barIndex
)
{
    switch (barIndex % 4U)
    {
        case 0:
            // Cmaj7
            return { 60, 64, 67, 71 };

        case 1:
            // Am7
            return { 57, 60, 64, 67 };

        case 2:
            // Fmaj7
            return { 53, 57, 60, 64 };

        default:
            // G7
            return { 55, 59, 62, 65 };
    }
}

// ============================================================
// HARMONY / PAD
// ============================================================

void MusicGenerator::triggerHarmony()
{
    const uint32_t chordIndex = _beat / 16U;
    const Chord chord = chordForBar(chordIndex);

    Serial0.printf(
        "[MUSIC] Chord changed: %s\n",
        chordIndex % 4U == 0U ? "Cmaj7" :
        chordIndex % 4U == 1U ? "Am7" :
        chordIndex % 4U == 2U ? "Fmaj7" : "G7"
    );

    triggerNote(chord.root + 12, 16.0f, 0.15f, VoiceType::Pad);
    triggerNote(chord.third + 12, 16.0f, 0.115f, VoiceType::Pad);
    triggerNote(chord.fifth + 12, 16.0f, 0.10f, VoiceType::Pad);
    triggerNote(chord.seventh + 12, 16.0f, 0.075f, VoiceType::Pad);
}

// ============================================================
// BASS
// ============================================================

void MusicGenerator::triggerBass()
{
    if ((_beat % 2U) != 0U)
        return;

    // The chord changes every four bars.
    const uint32_t chordIndex = _beat / 16U;
    const Chord chord = chordForBar(chordIndex);

    triggerNote(
        chord.root - 12,
        1.5f,
        0.20f,
        VoiceType::Bass
    );
}

// ============================================================
// MELODY
// ============================================================

void MusicGenerator::triggerMelody()
{
    while (_melodyIndex < MELODY_COUNT)
    {
        const NoteEvent& event = MELODY[_melodyIndex];

        if (event.beat < _beat)
        {
            ++_melodyIndex;
            continue;
        }

        if (event.beat > _beat)
            break;

        triggerNote(
            event.note,
            static_cast<float>(event.duration),
            event.velocity,
            VoiceType::Piano
        );

        ++_melodyIndex;
    }
}

// ============================================================
// HARP
// ============================================================

void MusicGenerator::triggerHarp()
{
    const uint32_t beatInBar = _beat % 4U;

    if (beatInBar >= 3U)
        return;

    const uint32_t chordIndex = _beat / 16U;
    const Chord chord = chordForBar(chordIndex);

    uint8_t note = chord.root;

    switch (beatInBar)
    {
        case 0:
            note = chord.root;
            break;

        case 1:
            note = chord.third;
            break;

        case 2:
            note = chord.fifth;
            break;

        default:
            return;
    }

    triggerNote(
        note + 12,
        0.85f,
        beatInBar == 0U ? 0.16f : 0.12f,
        VoiceType::Harp
    );
}

// ============================================================
// BELL
// ============================================================

void MusicGenerator::triggerBell()
{
    const uint32_t position = _beat % 16U;

    if (position != 14U)
        return;

    const uint32_t chordIndex = _beat / 16U;
    const Chord chord = chordForBar(chordIndex);

    triggerNote(
        chord.fifth + 24,
        1.8f,
        0.075f,
        VoiceType::Bell
    );
}

// ============================================================
// TRIGGER NOTE
// ============================================================

void MusicGenerator::triggerNote(
    uint8_t midiNote,
    float durationBeats,
    float velocity,
    VoiceType type
)
{
    if (!_initialized || _sampleRate == 0)
        return;

    Voice* voice = nullptr;

    // Find a free voice.
    for (size_t i = 0; i < MAX_VOICES; ++i)
    {
        if (!_voices[i].active)
        {
            voice = &_voices[i];
            break;
        }
    }

    // Steal the quietest voice when all voices are occupied.
    if (voice == nullptr)
    {
        voice = &_voices[0];

        float lowestLevel = voice->velocity;

        for (size_t i = 1; i < MAX_VOICES; ++i)
        {
            if (_voices[i].velocity < lowestLevel)
            {
                lowestLevel = _voices[i].velocity;
                voice = &_voices[i];
            }
        }
    }

    *voice = Voice{};

    voice->active = true;
    voice->type = type;
    voice->frequency = midiToFrequency(midiNote);
    voice->phase = 0.0f;
    voice->velocity = clamp01(velocity);

    const float safeDuration = std::max(0.1f, durationBeats);

    voice->durationSamples = static_cast<uint64_t>(
        safeDuration * _samplesPerBeat
    );

    float attackSeconds = 0.01f;
    float decaySeconds = 0.15f;
    float releaseSeconds = 0.20f;
    float sustain = 0.35f;

    switch (type)
    {
        case VoiceType::Piano:
            attackSeconds = 0.008f;
            decaySeconds = 0.18f;
            releaseSeconds = 0.24f;
            sustain = 0.22f;
            break;

        case VoiceType::Bass:
            attackSeconds = 0.025f;
            decaySeconds = 0.12f;
            releaseSeconds = 0.20f;
            sustain = 0.42f;
            break;

        case VoiceType::Pad:
            attackSeconds = 1.20f;
            decaySeconds = 1.50f;
            releaseSeconds = 0.85f;
            sustain = 0.72f;
            break;

        case VoiceType::Harp:
            attackSeconds = 0.006f;
            decaySeconds = 0.12f;
            releaseSeconds = 0.28f;
            sustain = 0.12f;
            break;

        case VoiceType::Bell:
            attackSeconds = 0.003f;
            decaySeconds = 0.35f;
            releaseSeconds = 0.90f;
            sustain = 0.025f;
            break;
    }

    voice->attackSamples = std::max<uint32_t>(
        1U,
        static_cast<uint32_t>(attackSeconds * _sampleRate)
    );

    voice->decaySamples = std::max<uint32_t>(
        1U,
        static_cast<uint32_t>(decaySeconds * _sampleRate)
    );

    voice->releaseSamples = std::max<uint32_t>(
        1U,
        static_cast<uint32_t>(releaseSeconds * _sampleRate)
    );

    voice->sustainLevel = sustain;
}

// ============================================================
// RENDER SAMPLE
// ============================================================

float MusicGenerator::renderSample()
{
    float mix = 0.0f;

    for (size_t i = 0; i < MAX_VOICES; ++i)
    {
        Voice& voice = _voices[i];

        if (voice.active)
            mix += renderVoice(voice);
    }

    return mix;
}

// ============================================================
// ENVELOPE
// ============================================================

float MusicGenerator::envelope(const Voice& voice) const
{
    const uint64_t age = voice.ageSamples;
    const uint64_t attack = voice.attackSamples;
    const uint64_t decay = voice.decaySamples;
    const uint64_t duration = voice.durationSamples;

    // Release starts at the requested note duration.
    if (age >= duration)
    {
        const uint64_t releaseAge = age - duration;

        if (releaseAge >= voice.releaseSamples)
            return 0.0f;

        const float progress =
            static_cast<float>(releaseAge) /
            static_cast<float>(voice.releaseSamples);

        return voice.sustainLevel * (1.0f - clamp01(progress));
    }

    // Attack.
    if (age < attack)
    {
        return static_cast<float>(age) /
               static_cast<float>(attack);
    }

    // Decay.
    const uint64_t decayEnd = attack + decay;

    if (age < decayEnd)
    {
        const float progress =
            static_cast<float>(age - attack) /
            static_cast<float>(decay);

        return 1.0f -
            (1.0f - voice.sustainLevel) * clamp01(progress);
    }

    // Sustain.
    return voice.sustainLevel;
}

// ============================================================
// RENDER VOICE
// ============================================================

float MusicGenerator::renderVoice(Voice& voice)
{
    const float env = envelope(voice);

    if (
        env <= VOICE_EPSILON &&
        voice.ageSamples >=
            voice.durationSamples + voice.releaseSamples
    )
    {
        voice.active = false;
        return 0.0f;
    }

    const float phase = voice.phase;
    const float fundamental = sinf(phase);

    float signal = fundamental;

    switch (voice.type)
    {
        case VoiceType::Piano:
            signal =
                fundamental * 0.72f +
                sinf(phase * 2.0f) * 0.22f +
                sinf(phase * 3.0f) * 0.10f +
                sinf(phase * 4.0f) * 0.045f;
            break;

        case VoiceType::Bass:
            signal =
                fundamental * 0.86f +
                sinf(phase * 2.0f) * 0.12f +
                sinf(phase * 3.0f) * 0.035f;
            break;

        case VoiceType::Pad:
            signal =
                fundamental * 0.78f +
                sinf(phase * 2.0f) * 0.16f +
                sinf(phase * 3.0f) * 0.045f;
            break;

        case VoiceType::Harp:
            signal =
                fundamental * 0.76f +
                sinf(phase * 2.0f) * 0.19f +
                sinf(phase * 3.0f) * 0.05f;
            break;

        case VoiceType::Bell:
            signal =
                fundamental * 0.48f +
                sinf(phase * 2.76f) * 0.32f +
                sinf(phase * 5.40f) * 0.15f +
                sinf(phase * 8.10f) * 0.05f;
            break;
    }

    const float phaseStep =
        TWO_PI * voice.frequency /
        static_cast<float>(_sampleRate);

    voice.phase += phaseStep;

    if (voice.phase >= TWO_PI)
        voice.phase = fmodf(voice.phase, TWO_PI);

    ++voice.ageSamples;

    return signal * env * voice.velocity;
}

// ============================================================
// MIDI TO FREQUENCY
// ============================================================

float MusicGenerator::midiToFrequency(uint8_t midiNote)
{
    return 440.0f * powf(
        2.0f,
        (static_cast<float>(midiNote) - 69.0f) / 12.0f
    );
}

// ============================================================
// CLAMP
// ============================================================

float MusicGenerator::clamp01(float value)
{
    if (value < 0.0f)
        return 0.0f;

    if (value > 1.0f)
        return 1.0f;

    return value;
}

// ============================================================
// CLEAR VOICES
// ============================================================

void MusicGenerator::clearVoices()
{
    for (size_t i = 0; i < MAX_VOICES; ++i)
        _voices[i] = Voice{};
}