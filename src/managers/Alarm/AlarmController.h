
#pragma once

#include <Arduino.h>
#include <functional>

#include "./managers/Alarm/AlarmManager.h"

class AlarmController
{
public:
    using MatrixCallback = std::function<void(const String&)>;
    using AudioCallback = std::function<void(const String&)>;
    using CobCallback = std::function<void(const String&)>;
    using StopCallback = std::function<void()>;

    explicit AlarmController(AlarmManager& alarmManager);

    void begin();
    void update();

    void setMatrixCallback(MatrixCallback callback);
    void setAudioCallback(AudioCallback callback);
    void setCobCallback(CobCallback callback);
    void setStopCallback(StopCallback callback);

    bool isActive() const;
    const String& alarmId() const;

    bool dismiss();
    bool snooze(uint32_t durationMs);

    void stop();

private:
    void onTriggered(const Alarm& alarm);
    void onFinished(const String& id);

    void executeAlarm(const Alarm& alarm);
    void stopOutputs();

    AlarmManager& _alarmManager;

    MatrixCallback _matrixCallback;
    AudioCallback _audioCallback;
    CobCallback _cobCallback;
    StopCallback _stopCallback;

    bool _active = false;
    bool _begun = false;

    String _alarmId;
};
