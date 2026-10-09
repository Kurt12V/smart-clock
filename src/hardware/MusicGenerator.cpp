#include "MusicGenerator.h"

#include <algorithm>
#include <math.h>

// ============================================================
// MELODY PATTERNS
// ============================================================

namespace
{

// ------------------------------------------------------------
// Sunrise Soft
// ------------------------------------------------------------

constexpr MusicGenerator::NoteEvent SOFT_MELODY[] =
{
    {72, 1, 0.55f},
    {76, 1, 0.50f},
    {79, 2, 0.52f},

    {76, 1, 0.45f},
    {74, 1, 0.48f},
    {72, 2, 0.52f},

    {69, 1, 0.42f},
    {72, 1, 0.45f},
    {74, 2, 0.48f},

    {76, 1, 0.50f},
    {79, 1, 0.52f},
    {81, 2, 0.48f},

    {79, 1, 0.48f},
    {76, 1, 0.45f},
    {74, 2, 0.44f},

    {72, 2, 0.50f}
};

// ------------------------------------------------------------
// Sunrise Ambient
// ------------------------------------------------------------

constexpr MusicGenerator::NoteEvent AMBIENT_MELODY[] =
{
    {72, 2, 0.45f},
    {76, 2, 0.42f},
    {79, 4, 0.45f},

    {74, 2, 0.40f},
    {77, 2, 0.42f},
    {81, 4, 0.44f},

    {79, 2, 0.42f},
    {76, 2, 0.40f},
    {72, 4, 0.44f}
};

// ------------------------------------------------------------
// Sunrise Piano
// ------------------------------------------------------------

constexpr MusicGenerator::NoteEvent PIANO_MELODY[] =
{
    {72, 1, 0.60f},
    {74, 1, 0.52f},
    {76, 1, 0.58f},
    {79, 1, 0.52f},

    {81, 2, 0.58f},
    {79, 1, 0.48f},
    {76, 1, 0.54f},
    {74, 2, 0.48f},

    {72, 1, 0.55f},
    {76, 1, 0.52f},
    {79, 2, 0.55f},
    {76, 2, 0.50f}
};

}

// ============================================================
// CONSTRUCTOR
// ============================================================

MusicGenerator::MusicGenerator()
    : _initialized(false),
      _playing(false),
      _preset(Preset::SunriseSoft),

      _sampleRate(DEFAULT_SAMPLE_RATE),
      _bpm(DEFAULT_BPM),

      _volume(0.0f),
      _targetVolume(0.0f),

      _beatSamples(0),
      _samplePosition(0),
      _nextBeatSample(0),

      _beat(0),
      _bar(0),
      _melodyIndex(0),

      _melody(),
      _bass(),
      _pad1(),
      _pad2()
{
}

// ============================================================
// BEGIN
// ============================================================

void MusicGenerator::begin(
    uint32_t sampleRate
)
{
    if (sampleRate == 0)
        sampleRate = DEFAULT_SAMPLE_RATE;

    _sampleRate = sampleRate;

    updateTiming();

    _initialized = true;
}

// ============================================================
// END
// ============================================================

void MusicGenerator::end()
{
    stop();

    _initialized = false;
}

// ============================================================
// STATE
// ============================================================

bool MusicGenerator::isInitialized() const
{
    return _initialized;
}

// ============================================================
// START
// ============================================================

void MusicGenerator::start(
    Preset preset
)
{
    if (!_initialized)
        return;

    _preset = preset;

    _playing = true;

    _samplePosition = 0;
    _nextBeatSample = 0;

    _beat = 0;
    _bar = 0;

    _melodyIndex = 0;

    _melody = {};
    _bass = {};
    _pad1 = {};
    _pad2 = {};
}

// ============================================================
// STOP
// ============================================================

void MusicGenerator::stop()
{
    _playing = false;

    _targetVolume = 0.0f;
    _volume = 0.0f;

    _melody.active = false;
    _bass.active = false;
    _pad1.active = false;
    _pad2.active = false;
}

// ============================================================
// PLAYING
// ============================================================

bool MusicGenerator::isPlaying() const
{
    return _playing;
}

// ============================================================
// VOLUME
// ============================================================

void MusicGenerator::setVolume(
    float volume01
)
{
    _targetVolume =
        clamp01(volume01);
}

// ============================================================

float MusicGenerator::volume() const
{
    return _volume;
}

// ============================================================
// BPM
// ============================================================

void MusicGenerator::setBpm(
    float bpm
)
{
    _bpm =
        std::max(
            40.0f,
            std::min(
                180.0f,
                bpm
            )
        );

    updateTiming();
}

// ============================================================

float MusicGenerator::bpm() const
{
    return _bpm;
}

// ============================================================
// SAMPLE RATE
// ============================================================

uint32_t MusicGenerator::sampleRate() const
{
    return _sampleRate;
}

// ============================================================
// TIMING
// ============================================================

void MusicGenerator::updateTiming()
{
    if (_sampleRate == 0)
        return;

    _beatSamples =
        static_cast<uint32_t>(
            (
                60.0f /
                _bpm
            )
            *
            static_cast<float>(
                _sampleRate
            )
        );

    if (_beatSamples == 0)
        _beatSamples = 1;
}

// ============================================================
// GENERATE BLOCK
// ============================================================

void MusicGenerator::generateBlock(
    int16_t* buffer,
    size_t sampleCount
)
{
    if (buffer == nullptr)
        return;

    if (sampleCount == 0)
        return;

    const float volumeStep =
        (
            _targetVolume -
            _volume
        )
        /
        static_cast<float>(
            sampleCount
        );

    for (size_t i = 0;
         i < sampleCount;
         ++i)
    {
        _volume += volumeStep;

        float sample = 0.0f;

        if (_playing)
        {
            if (
                _samplePosition >=
                _nextBeatSample
            )
            {
                triggerBeat();

                _nextBeatSample +=
                    _beatSamples;
            }

            sample = renderSample();
        }

        sample *= _volume;

        sample =
            std::max(
                -0.95f,
                std::min(
                    0.95f,
                    sample
                )
            );

        const int16_t pcm =
            static_cast<int16_t>(
                sample *
                32767.0f
            );

        buffer[i * 2] = pcm;
        buffer[i * 2 + 1] = pcm;

        ++_samplePosition;
    }
}

// ============================================================
// RENDER
// ============================================================

float MusicGenerator::renderSample()
{
    float sample = 0.0f;

    sample +=
        renderVoice(
            _melody
        )
        *
        0.34f;

    sample +=
        renderVoice(
            _bass
        )
        *
        0.18f;

    sample +=
        renderVoice(
            _pad1
        )
        *
        0.20f;

    sample +=
        renderVoice(
            _pad2
        )
        *
        0.16f;

    return sample;
}

// ============================================================
// VOICE
// ============================================================

float MusicGenerator::renderVoice(
    Voice& voice
)
{
    if (!voice.active)
        return 0.0f;

    if (voice.frequency <= 0.0f)
        return 0.0f;

    ++voice.ageSamples;

    const float age =
        static_cast<float>(
            voice.ageSamples
        )
        /
        static_cast<float>(
            _sampleRate
        );

    const float life =
        static_cast<float>(
            voice.lifeSamples
        )
        /
        static_cast<float>(
            _sampleRate
        );

    float envelope = 1.0f;

    // --------------------------------------------------------
    // ATTACK
    // --------------------------------------------------------

    if (
        voice.attack > 0.0f &&
        age < voice.attack
    )
    {
        envelope =
            age /
            voice.attack;

        envelope =
            smoothStep(
                envelope
            );
    }

    // --------------------------------------------------------
    // RELEASE
    // --------------------------------------------------------

    else if (
        voice.release > 0.0f &&
        age >
            life -
            voice.release
    )
    {
        const float remaining =
            std::max(
                0.0f,
                life - age
            );

        envelope =
            remaining /
            voice.release;

        envelope =
            smoothStep(
                envelope
            );
    }

    // --------------------------------------------------------
    // FINISHED
    // --------------------------------------------------------

    if (
        voice.ageSamples >=
        voice.lifeSamples
    )
    {
        voice.active = false;

        return 0.0f;
    }

    // --------------------------------------------------------
    // OSCILLATOR
    // --------------------------------------------------------

    const float value =
        sine(
            voice.phase
        );

    voice.phase +=
        TWO_PI *
        voice.frequency /
        static_cast<float>(
            _sampleRate
        );

    if (
        voice.phase >=
        TWO_PI
    )
    {
        voice.phase -= TWO_PI;
    }

    return
        value *
        envelope *
        voice.level;
}

// ============================================================
// BEAT
// ============================================================

void MusicGenerator::triggerBeat()
{
    triggerMelody();

    if ((_beat % 2) == 0)
    {
        triggerBass();
    }

    triggerHarmony();

    ++_beat;

    if (_beat >= 16)
    {
        _beat = 0;
        ++_bar;
    }
}

// ============================================================
// MELODY
// ============================================================

void MusicGenerator::triggerMelody()
{
    size_t count = 0;

    const NoteEvent* pattern =
        melodyPattern(
            count
        );

    if (!pattern || count == 0)
        return;

    const NoteEvent& event =
        pattern[
            _melodyIndex %
            static_cast<int>(
                count
            )
        ];

    ++_melodyIndex;

    if (
        _melodyIndex >=
        static_cast<int>(
            count
        )
    )
    {
        _melodyIndex = 0;
    }

    _melody.frequency =
        midiToFrequency(
            event.note
        );

    _melody.phase = 0.0f;

    _melody.level =
        event.velocity;

    _melody.ageSamples = 0;

    const float duration =
        (
            60.0f /
            _bpm
        )
        *
        static_cast<float>(
            event.beats
        )
        *
        0.90f;

    _melody.lifeSamples =
        std::max<uint32_t>(
            1,
            static_cast<uint32_t>(
                duration *
                static_cast<float>(
                    _sampleRate
                )
            )
        );

    _melody.attack = 0.035f;

    _melody.release =
        std::min(
            0.35f,
            duration * 0.45f
        );

    _melody.active = true;
}

// ============================================================
// HARMONY
// ============================================================

void MusicGenerator::triggerHarmony()
{
    int root = 60;
    int third = 64;
    int fifth = 67;

    selectChord(
        root,
        third,
        fifth
    );

    const bool alternate =
        (
            (
                _bar +
                _beat / 4
            )
            % 2
        ) != 0;

    _pad1.frequency =
        midiToFrequency(
            alternate
                ? third
                : root
        );

    _pad2.frequency =
        midiToFrequency(
            alternate
                ? fifth
                : third
        );

    _pad1.phase = 0.0f;
    _pad2.phase = 0.0f;

    _pad1.level = 0.20f;
    _pad2.level = 0.16f;

    _pad1.ageSamples = 0;
    _pad2.ageSamples = 0;

    _pad1.lifeSamples =
        _beatSamples * 4;

    _pad2.lifeSamples =
        _beatSamples * 4;

    _pad1.attack = 0.12f;
    _pad2.attack = 0.18f;

    _pad1.release = 0.40f;
    _pad2.release = 0.45f;

    _pad1.active = true;
    _pad2.active = true;
}

// ============================================================
// BASS
// ============================================================

void MusicGenerator::triggerBass()
{
    int root = 48;
    int third = 52;
    int fifth = 55;

    selectChord(
        root,
        third,
        fifth
    );

    _bass.frequency =
        midiToFrequency(
            root
        );

    _bass.phase = 0.0f;

    _bass.level = 0.20f;

    _bass.ageSamples = 0;

    _bass.lifeSamples =
        _beatSamples * 2;

    _bass.attack = 0.025f;
    _bass.release = 0.30f;

    _bass.active = true;
}

// ============================================================
// PATTERN
// ============================================================

const MusicGenerator::NoteEvent*
MusicGenerator::melodyPattern(
    size_t& count
) const
{
    switch (_preset)
    {
        case Preset::SunriseAmbient:

            count =
                sizeof(AMBIENT_MELODY) /
                sizeof(AMBIENT_MELODY[0]);

            return AMBIENT_MELODY;

        case Preset::SunrisePiano:

            count =
                sizeof(PIANO_MELODY) /
                sizeof(PIANO_MELODY[0]);

            return PIANO_MELODY;

        case Preset::SunriseSoft:

        default:

            count =
                sizeof(SOFT_MELODY) /
                sizeof(SOFT_MELODY[0]);

            return SOFT_MELODY;
    }
}

// ============================================================
// CHORD
// ============================================================

void MusicGenerator::selectChord(
    int& root,
    int& third,
    int& fifth
) const
{
    switch (_bar % 4)
    {
        // C major
        case 0:

            root = 48;
            third = 52;
            fifth = 55;

            break;

        // A minor
        case 1:

            root = 45;
            third = 48;
            fifth = 52;

            break;

        // F major
        case 2:

            root = 41;
            third = 45;
            fifth = 48;

            break;

        // G major
        default:

            root = 43;
            third = 47;
            fifth = 50;

            break;
    }
}

// ============================================================
// MIDI
// ============================================================

float MusicGenerator::midiToFrequency(
    int midi
)
{
    return
        440.0f *
        powf(
            2.0f,
            (
                static_cast<float>(
                    midi
                )
                -
                69.0f
            )
            /
            12.0f
        );
}

// ============================================================
// SINE
// ============================================================

float MusicGenerator::sine(
    float phase
)
{
    return sinf(phase);
}

// ============================================================
// CLAMP
// ============================================================

float MusicGenerator::clamp01(
    float value
)
{
    return std::max(
        0.0f,
        std::min(
            1.0f,
            value
        )
    );
}

// ============================================================
// SMOOTH STEP
// ============================================================

float MusicGenerator::smoothStep(
    float value
)
{
    value =
        clamp01(
            value
        );

    return
        value *
        value *
        (
            3.0f -
            2.0f *
            value
        );
}