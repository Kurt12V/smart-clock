#include "AudioRecorder.h"

#include "Microphone.h"


// =====================================================
// CONSTRUCTOR
// =====================================================

AudioRecorder::AudioRecorder(
    Microphone& microphone,
    fs::FS& filesystem
)
    : microphone(microphone),
      filesystem(filesystem),
      recording(false),
      recordedBytes(0),
      recordedSamples(0),
      recordingStartTime(0),
      lastChunkId(0)
{
}


// =====================================================
// BEGIN
// =====================================================

bool AudioRecorder::begin()
{
    Serial.println(
        "[AudioRecorder] READY"
    );

    return true;
}


// =====================================================
// START RECORDING
// =====================================================

bool AudioRecorder::startRecording(
    const char* path
)
{
    if (recording)
    {
        Serial.println(
            "[AudioRecorder] Already recording"
        );

        return false;
    }


    if (!microphone.isInitialized())
    {
        Serial.println(
            "[AudioRecorder] Microphone not initialized"
        );

        return false;
    }


    if (!microphone.isEnabled())
    {
        Serial.println(
            "[AudioRecorder] Microphone disabled"
        );

        return false;
    }


    if (!microphone.isListening())
    {
        Serial.println(
            "[AudioRecorder] Microphone not listening"
        );

        return false;
    }


    if (path == nullptr)
    {
        return false;
    }


    // =================================================
    // OPEN FILE
    // =================================================

    file =
        filesystem.open(
            path,
            FILE_WRITE
        );


    if (!file)
    {
        Serial.printf(
            "[AudioRecorder] Failed to open: %s\n",
            path
        );

        return false;
    }


    recordedBytes = 0;

    recordedSamples = 0;

    recordingStartTime =
        millis();


    /*
     * Запоминаем текущий chunk.
     *
     * Мы НЕ хотим повторно записывать
     * уже существующий chunk.
     */

    lastChunkId =
        microphone.getAudioChunkId();


    // =================================================
    // WAV HEADER
    // =================================================

    writeWavHeader();


    recording = true;


    Serial.printf(
        "[AudioRecorder] RECORDING STARTED: %s\n",
        path
    );


    return true;
}


// =====================================================
// STOP RECORDING
// =====================================================

void AudioRecorder::stopRecording()
{
    if (!recording)
        return;


    updateWavHeader();


    file.flush();

    file.close();


    recording = false;


    Serial.println(
        "[AudioRecorder] RECORDING STOPPED"
    );


    Serial.printf(
        "[AudioRecorder] Bytes: %lu\n",
        recordedBytes
    );


    Serial.printf(
        "[AudioRecorder] Samples: %lu\n",
        recordedSamples
    );


    Serial.printf(
        "[AudioRecorder] Duration: %lu ms\n",
        getRecordingDurationMs()
    );
}


// =====================================================
// UPDATE
// =====================================================

void AudioRecorder::update()
{
    if (!recording)
        return;


    /*
     * Получаем ID текущего chunk.
     */

    uint32_t chunkId =
        microphone.getAudioChunkId();


    /*
     * Если Microphone ещё не получил
     * новый chunk — ничего не делаем.
     */

    if (chunkId == lastChunkId)
    {
        return;
    }


    const int16_t* samples =
        nullptr;


    size_t count =
        microphone.getAudioChunk(
            samples
        );


    if (samples == nullptr)
    {
        return;
    }


    if (count == 0)
    {
        return;
    }


    // =================================================
    // WRITE
    // =================================================

    writeSamples(
        samples,
        count
    );


    /*
     * Запоминаем chunk.
     *
     * Поэтому он больше не будет
     * записан повторно.
     */

    lastChunkId =
        chunkId;
}


// =====================================================
// WRITE SAMPLES
// =====================================================

void AudioRecorder::writeSamples(
    const int16_t* samples,
    size_t count
)
{
    if (!file)
        return;


    if (samples == nullptr)
        return;


    if (count == 0)
        return;


    size_t bytes =
        count *
        sizeof(int16_t);


    size_t written =
        file.write(
            reinterpret_cast<
                const uint8_t*
            >(samples),
            bytes
        );


    if (written != bytes)
    {
        Serial.println(
            "[AudioRecorder] SD WRITE ERROR"
        );

        return;
    }


    recordedBytes +=
        written;


    recordedSamples +=
        count;
}


// =====================================================
// WAV HEADER
// =====================================================

void AudioRecorder::writeWavHeader()
{
    /*
     * RIFF
     */

    file.write(
        (const uint8_t*)"RIFF",
        4
    );


    /*
     * ChunkSize
     *
     * Будет исправлен после записи.
     */

    writeLE32(0);


    /*
     * WAVE
     */

    file.write(
        (const uint8_t*)"WAVE",
        4
    );


    /*
     * fmt
     */

    file.write(
        (const uint8_t*)"fmt ",
        4
    );


    /*
     * Subchunk1Size
     */

    writeLE32(16);


    /*
     * AudioFormat
     *
     * 1 = PCM
     */

    writeLE16(1);


    /*
     * Channels
     */

    writeLE16(
        CHANNELS
    );


    /*
     * Sample rate
     */

    writeLE32(
        SAMPLE_RATE
    );


    /*
     * Byte rate
     */

    uint32_t byteRate =
        SAMPLE_RATE *
        CHANNELS *
        (BITS_PER_SAMPLE / 8);


    writeLE32(
        byteRate
    );


    /*
     * Block align
     */

    uint16_t blockAlign =
        CHANNELS *
        (BITS_PER_SAMPLE / 8);


    writeLE16(
        blockAlign
    );


    /*
     * Bits per sample
     */

    writeLE16(
        BITS_PER_SAMPLE
    );


    /*
     * data
     */

    file.write(
        (const uint8_t*)"data",
        4
    );


    /*
     * Data size.
     *
     * Исправляется после записи.
     */

    writeLE32(0);
}


// =====================================================
// UPDATE WAV HEADER
// =====================================================

void AudioRecorder::updateWavHeader()
{
    if (!file)
        return;


    /*
     * RIFF ChunkSize:
     *
     * 36 + audio data
     */

    uint32_t riffSize =
        36 +
        recordedBytes;


    /*
     * ChunkSize находится
     * на offset 4.
     */

    file.seek(4);

    writeLE32(
        riffSize
    );


    /*
     * data size находится
     * на offset 40.
     */

    file.seek(40);

    writeLE32(
        recordedBytes
    );


    /*
     * Возвращаемся в конец.
     */

    file.seek(
        file.size()
    );
}


// =====================================================
// WRITE LE16
// =====================================================

void AudioRecorder::writeLE16(
    uint16_t value
)
{
    uint8_t data[2];


    data[0] =
        value & 0xFF;


    data[1] =
        (value >> 8) & 0xFF;


    file.write(
        data,
        2
    );
}


// =====================================================
// WRITE LE32
// =====================================================

void AudioRecorder::writeLE32(
    uint32_t value
)
{
    uint8_t data[4];


    data[0] =
        value & 0xFF;


    data[1] =
        (value >> 8) & 0xFF;


    data[2] =
        (value >> 16) & 0xFF;


    data[3] =
        (value >> 24) & 0xFF;


    file.write(
        data,
        4
    );
}


// =====================================================
// STATUS
// =====================================================

bool AudioRecorder::isRecording() const
{
    return recording;
}


// =====================================================
// RECORDED BYTES
// =====================================================

uint32_t AudioRecorder::getRecordedBytes() const
{
    return recordedBytes;
}


// =====================================================
// RECORDED SAMPLES
// =====================================================

uint32_t AudioRecorder::getRecordedSamples() const
{
    return recordedSamples;
}


// =====================================================
// DURATION
// =====================================================

uint32_t AudioRecorder::getRecordingDurationMs() const
{
    if (recordedSamples == 0)
    {
        return 0;
    }


    return
        (recordedSamples * 1000UL) /
        SAMPLE_RATE;
}