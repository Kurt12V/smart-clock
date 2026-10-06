#pragma once

#include <Arduino.h>
#include <functional>

#include "managers/AlarmManager.h"

class AlarmController
{
public:
    using MatrixCallback =
        std::function<void(const String& effectId)>;

    using AudioCallback =
        std::function<void(const String& effectId)>;

    using CobCallback =
        std::function<void(const String& effectId)>;

    using StopCallback =
        std::function<void()>;

public:
    explicit AlarmController(
        AlarmManager& alarmManager
    );

    void begin();
    void update();

    // --------------------------------------------------------
    // CALLBACKS
    // --------------------------------------------------------

    void setMatrixCallback(MatrixCallback callback);
    void setAudioCallback(AudioCallback callback);
    void setCobCallback(CobCallback callback);
    void setStopCallback(StopCallback callback);

    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool isActive() const;
    const String& alarmId() const;

    // --------------------------------------------------------
    // CONTROL
    // --------------------------------------------------------

    bool dismiss();
    bool snooze(uint32_t durationMs);
    void stop();

private:
    // --------------------------------------------------------
    // ALARM MANAGER CALLBACKS
    // --------------------------------------------------------

    void onTriggered(const Alarm& alarm);
    void onFinished(const String& id);

    // --------------------------------------------------------
    // OUTPUT CONTROL
    // --------------------------------------------------------

    void executeAlarm(const Alarm& alarm);
    void stopOutputs();

private:
    AlarmManager& _alarmManager;

    MatrixCallback _matrixCallback;
    AudioCallback _audioCallback;
    CobCallback _cobCallback;
    StopCallback _stopCallback;

    bool _active = false;
    String _alarmId;

    bool _begun = false;
};