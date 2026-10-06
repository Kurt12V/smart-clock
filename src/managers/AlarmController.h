#pragma once

#include <Arduino.h>
#include <functional>

#include "managers/AlarmManager.h"

class AlarmController
{
public:
    using MatrixCallback =
        std::function<void(const AlarmMatrix& matrix)>;

    using AudioCallback =
        std::function<void(const AlarmAudio& audio)>;

    using CobCallback =
        std::function<void(const AlarmCob& cob)>;

    using StopCallback =
        std::function<void()>;

    explicit AlarmController(
        AlarmManager& alarmManager
    );

    void begin();
    void update();

    void setMatrixCallback(MatrixCallback callback);
    void setAudioCallback(AudioCallback callback);
    void setCobCallback(CobCallback callback);
    void setStopCallback(StopCallback callback);

    bool isActive() const;
    const String& alarmId() const;
    uint8_t phaseIndex() const;

    bool dismiss();
    bool snooze(uint32_t durationMs);
    void stop();

private:
    void onTriggered(const Alarm& alarm);
    void onPhaseChanged(
        const Alarm& alarm,
        uint8_t phaseIndex,
        const AlarmPhase& phase
    );
    void onFinished(const String& id);

    void executePhase(const AlarmPhase& phase);
    void stopOutputs();

private:
    AlarmManager& _alarmManager;

    MatrixCallback _matrixCallback;
    AudioCallback _audioCallback;
    CobCallback _cobCallback;
    StopCallback _stopCallback;

    bool _active = false;
    String _alarmId;
    uint8_t _phaseIndex = 0;
    bool _begun = false;
};
