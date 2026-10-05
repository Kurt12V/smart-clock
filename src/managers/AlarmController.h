#pragma once

#include <Arduino.h>
#include <functional>

#include "managers/AlarmManager.h"


class AlarmController
{
public:

    // ========================================================
    // CALLBACK TYPES
    // ========================================================

    using MatrixCallback =
        std::function<void(const AlarmMatrix& matrix)>;

    using AudioCallback =
        std::function<void(const AlarmAudio& audio)>;

    using CobCallback =
        std::function<void(const AlarmCob& cob)>;

    using StopCallback =
        std::function<void()>;


    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    explicit AlarmController(
        AlarmManager& alarmManager
    );


    // ========================================================
    // LIFECYCLE
    // ========================================================

    void begin();
    void update();


    // ========================================================
    // CALLBACKS
    // ========================================================

    void setMatrixCallback(
        MatrixCallback callback
    );

    void setAudioCallback(
        AudioCallback callback
    );

    void setCobCallback(
        CobCallback callback
    );

    void setStopCallback(
        StopCallback callback
    );


    // ========================================================
    // STATE
    // ========================================================

    bool isActive() const;

    const String& alarmId() const;

    uint8_t phaseIndex() const;


    // ========================================================
    // CONTROL
    // ========================================================

    bool dismiss();

    bool snooze(
        uint32_t durationMs
    );

    void stop();


private:

    // ========================================================
    // ALARM MANAGER CALLBACKS
    // ========================================================

    void onTriggered(
        const Alarm& alarm
    );

    void onPhaseChanged(
        const Alarm& alarm,
        uint8_t phaseIndex,
        const AlarmPhase& phase
    );

    void onFinished(
        const String& id
    );


    // ========================================================
    // PHASE
    // ========================================================

    void executePhase(
        const AlarmPhase& phase
    );


    // ========================================================
    // OUTPUT CONTROL
    // ========================================================

    void stopOutputs();


private:

    AlarmManager& _alarmManager;


    // ========================================================
    // CALLBACKS
    // ========================================================

    MatrixCallback _matrixCallback;
    AudioCallback  _audioCallback;
    CobCallback    _cobCallback;
    StopCallback   _stopCallback;


    // ========================================================
    // RUNTIME STATE
    // ========================================================

    bool _active = false;

    String _alarmId;

    uint8_t _phaseIndex = 0;
};