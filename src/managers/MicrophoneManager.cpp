#include "MicrophoneManager.h"

#include <Arduino.h>

#include "./hardware/audio/Microphone.h"
#include "./hardware/audio/AudioRecorder.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

MicrophoneManager::MicrophoneManager(
    Microphone& microphone,
    AudioRecorder& recorder
)
    : microphone(microphone),
      recorder(recorder),
      initialized(false),
      enabled(true)
{
}


// ============================================================
// BEGIN
// ============================================================

bool MicrophoneManager::begin()
{
    Serial.println(
        "[MicrophoneManager] BEGIN"
    );


    if (!microphone.begin())
    {
        Serial.println(
            "[MicrophoneManager] ERROR: "
            "microphone.begin() failed"
        );

        return false;
    }


    if (!recorder.begin())
    {
        Serial.println(
            "[MicrophoneManager] ERROR: "
            "recorder.begin() failed"
        );

        return false;
    }


    initialized = true;


    microphone.setEnabled(
        enabled
    );


    Serial.println(
        "[MicrophoneManager] READY"
    );


    return true;
}


// ============================================================
// END
// ============================================================

void MicrophoneManager::end()
{
    if (!initialized)
        return;


    if (recorder.isRecording())
    {
        recorder.stopRecording();
    }


    microphone.stopListening();

    microphone.end();


    initialized = false;


    Serial.println(
        "[MicrophoneManager] STOPPED"
    );
}


// ============================================================
// UPDATE
// ============================================================

void MicrophoneManager::update()
{
    if (!initialized)
        return;


    if (!enabled)
        return;


    updateMicrophone();

    updateRecorder();
}


// ============================================================
// MICROPHONE
// ============================================================

void MicrophoneManager::updateMicrophone()
{
    if (!microphone.isListening())
        return;


    microphone.updateAudio();
}


// ============================================================
// RECORDER
// ============================================================

void MicrophoneManager::updateRecorder()
{
    if (!recorder.isRecording())
        return;


    recorder.update();
}


// ============================================================
// LISTENING
// ============================================================

bool MicrophoneManager::startListening()
{
    if (!initialized)
        return false;


    if (!enabled)
        return false;


    return microphone.startListening();
}


void MicrophoneManager::stopListening()
{
    microphone.stopListening();
}


bool MicrophoneManager::isListening() const
{
    return microphone.isListening();
}


// ============================================================
// ENABLE
// ============================================================

void MicrophoneManager::setEnabled(
    bool enabled
)
{
    this->enabled = enabled;


    microphone.setEnabled(
        enabled
    );


    if (!enabled)
    {
        if (recorder.isRecording())
        {
            recorder.stopRecording();
        }

        microphone.stopListening();
    }
}


bool MicrophoneManager::isEnabled() const
{
    return enabled;
}


bool MicrophoneManager::isInitialized() const
{
    return initialized;
}


// ============================================================
// ANALYSIS
// ============================================================

float MicrophoneManager::getRMS() const
{
    return microphone.getRMS();
}


float MicrophoneManager::getLevel() const
{
    return microphone.getLevel();
}


int16_t MicrophoneManager::getPeak() const
{
    return microphone.getPeak();
}


// ============================================================
// AUDIO CHUNK
// ============================================================

bool MicrophoneManager::hasNewAudio() const
{
    return microphone.hasNewAudio();
}


size_t MicrophoneManager::getAudioChunk(
    const int16_t*& data
) const
{
    return microphone.getAudioChunk(data);
}


size_t MicrophoneManager::getAudioChunkSize() const
{
    return microphone.getAudioChunkSize();
}


uint32_t MicrophoneManager::getAudioChunkId() const
{
    return microphone.getAudioChunkId();
}


// ============================================================
// RECORDING
// ============================================================

bool MicrophoneManager::startRecording(
    const char* path
)
{
    if (!initialized)
        return false;


    if (!enabled)
        return false;


    if (!microphone.isListening())
    {
        if (!microphone.startListening())
        {
            return false;
        }
    }


    if (!recorder.startRecording(path))
    {
        return false;
    }


    Serial.print(
        "[MicrophoneManager] "
        "RECORDING: "
    );

    Serial.println(path);


    return true;
}


void MicrophoneManager::stopRecording()
{
    if (!recorder.isRecording())
        return;


    recorder.stopRecording();
}


bool MicrophoneManager::isRecording() const
{
    return recorder.isRecording();
}


// ============================================================
// RECORDING INFO
// ============================================================

uint32_t MicrophoneManager::getRecordedBytes() const
{
    return recorder.getRecordedBytes();
}


uint32_t MicrophoneManager::getRecordedSamples() const
{
    return recorder.getRecordedSamples();
}


uint32_t MicrophoneManager::getRecordingDurationMs() const
{
    return recorder.getRecordingDurationMs();
}


// ============================================================
// NAME
// ============================================================

const char* MicrophoneManager::getName() const
{
    return microphone.getName();
}