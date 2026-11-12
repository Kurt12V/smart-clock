
#include "MicrophoneManager.h"

#include <Arduino.h>

#include "./hardware/audio/Microphone.h"
#include "./hardware/audio/AudioRecorder.h"

MicrophoneManager::MicrophoneManager(
    Microphone& microphone,
    AudioRecorder& recorder
)
    : microphone(microphone),
      recorder(recorder),
      initialized(false),
      enabled(true),
      recorderChunkId(0)
{
}

bool MicrophoneManager::begin()
{
    if (initialized)
        return true;

    if (!microphone.begin())
        return false;

    if (!recorder.begin())
    {
        microphone.end();
        return false;
    }

    microphone.setEnabled(enabled);

    initialized = true;

    Serial0.println("[MicrophoneManager] Ready");

    return true;
}

void MicrophoneManager::end()
{
    if (!initialized)
        return;

    stopRecording();
    microphone.stopListening();
    microphone.end();

    initialized = false;

    Serial0.println("[MicrophoneManager] Stopped");
}

void MicrophoneManager::update()
{
    if (!initialized || !enabled)
        return;

    updateMicrophone();
    updateRecorder();
}

void MicrophoneManager::updateMicrophone()
{
    if (!microphone.isListening())
        return;

    microphone.updateAudio();
}

void MicrophoneManager::updateRecorder()
{
    if (!recorder.isRecording())
        return;

    recorder.update();
}

bool MicrophoneManager::startListening()
{
    if (!initialized || !enabled)
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

void MicrophoneManager::setEnabled(bool enabled)
{
    this->enabled = enabled;

    microphone.setEnabled(enabled);

    if (!enabled)
    {
        stopRecording();
        stopListening();
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

size_t MicrophoneManager::copyAudioChunk(
    int16_t* destination,
    size_t capacity,
    uint32_t& chunkId
) const
{
    return microphone.copyAudioChunk(
        destination,
        capacity,
        chunkId
    );
}

size_t MicrophoneManager::getAudioChunkSize() const
{
    return microphone.getAudioChunkSize();
}

uint32_t MicrophoneManager::getAudioChunkId() const
{
    return microphone.getAudioChunkId();
}

bool MicrophoneManager::startRecording(
    const char* path
)
{
    if (!initialized || !enabled || path == nullptr)
        return false;

    if (!microphone.isListening())
    {
        if (!microphone.startListening())
            return false;
    }

    if (!recorder.startRecording(path))
        return false;

    recorderChunkId = microphone.getAudioChunkId();

    Serial0.printf(
        "[MicrophoneManager] Recording: %s\n",
        path
    );

    return true;
}

void MicrophoneManager::stopRecording()
{
    if (recorder.isRecording())
        recorder.stopRecording();
}

bool MicrophoneManager::isRecording() const
{
    return recorder.isRecording();
}

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

const char* MicrophoneManager::getName() const
{
    return microphone.getName();
}
