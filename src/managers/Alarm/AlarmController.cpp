
#include "AlarmController.h"

#include <utility>

// ============================================================
// CONSTRUCTOR
// ============================================================

AlarmController::AlarmController(
    AlarmManager& alarmManager
)
    : _alarmManager(alarmManager)
{
}

// ============================================================
// BEGIN
// ============================================================

void AlarmController::begin()
{
    if (_begun)
        return;

    _alarmManager.setTriggerCallback(
        [this](const Alarm& alarm)
        {
            onTriggered(alarm);
        }
    );

    _alarmManager.setFinishCallback(
        [this](const String& id)
        {
            onFinished(id);
        }
    );

    _active = false;
    _alarmId = String();

    _begun = true;

    Serial0.println("[ALARM][CONTROLLER] READY");
}

// ============================================================
// UPDATE
// ============================================================

void AlarmController::update()
{
    if (!_begun)
        return;

    // AlarmManager owns the alarm runtime.
    // Controller only checks for state desynchronization.

    if (_active && !_alarmManager.isRunning())
    {
        Serial0.println(
            "[ALARM][CONTROLLER] Runtime stopped unexpectedly"
        );

        stopOutputs();

        _active = false;
        _alarmId = String();
    }
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void AlarmController::setMatrixCallback(
    MatrixCallback callback
)
{
    _matrixCallback = std::move(callback);
}

void AlarmController::setAudioCallback(
    AudioCallback callback
)
{
    _audioCallback = std::move(callback);
}

void AlarmController::setCobCallback(
    CobCallback callback
)
{
    _cobCallback = std::move(callback);
}

void AlarmController::setStopCallback(
    StopCallback callback
)
{
    _stopCallback = std::move(callback);
}

// ============================================================
// STATE
// ============================================================

bool AlarmController::isActive() const
{
    return _active;
}

const String& AlarmController::alarmId() const
{
    return _alarmId;
}

// ============================================================
// DISMISS
// ============================================================

bool AlarmController::dismiss()
{
    if (!_begun || !_active)
        return false;

    Serial0.println("[ALARM][CONTROLLER] DISMISS");

    return _alarmManager.dismiss();
}

// ============================================================
// SNOOZE
// ============================================================

bool AlarmController::snooze(uint32_t durationMs)
{
    if (!_begun || !_active)
        return false;

    Serial0.printf(
        "[ALARM][CONTROLLER] SNOOZE %lu ms\n",
        static_cast<unsigned long>(durationMs)
    );

    return _alarmManager.snooze(durationMs);
}

// ============================================================
// STOP
// ============================================================

void AlarmController::stop()
{
    if (!_begun)
        return;

    Serial0.println("[ALARM][CONTROLLER] STOP REQUEST");

    if (_alarmManager.isRunning())
    {
        // finish() should invoke the registered finish callback.
        _alarmManager.finish();
        return;
    }

    // Fallback if AlarmManager is already stopped.
    if (_active)
    {
        stopOutputs();

        _active = false;
        _alarmId = String();
    }
}

// ============================================================
// ON TRIGGERED
// ============================================================

void AlarmController::onTriggered(const Alarm& alarm)
{
    if (!_begun)
        return;

    if (_active)
    {
        Serial0.printf(
            "[ALARM][CONTROLLER] TRIGGER IGNORED id=%s\n",
            _alarmId.c_str()
        );

        return;
    }

    _active = true;
    _alarmId = alarm.id;

    Serial0.printf(
        "[ALARM][CONTROLLER] TRIGGER id=%s matrix=%s cob=%s audio=%s\n",
        alarm.id.c_str(),
        alarm.matrixEffect.c_str(),
        alarm.cobEffect.c_str(),
        alarm.audioEffect.c_str()
    );

    executeAlarm(alarm);
}

// ============================================================
// ON FINISHED
// ============================================================

void AlarmController::onFinished(const String& id)
{
    if (!_begun)
        return;

    Serial0.printf(
        "[ALARM][CONTROLLER] FINISH id=%s\n",
        id.c_str()
    );

    // Stop music and lights.
    stopOutputs();

    _active = false;
    _alarmId = String();
}

// ============================================================
// EXECUTE ALARM
// ============================================================

void AlarmController::executeAlarm(const Alarm& alarm)
{
    // MATRIX
    if (!alarm.matrixEffect.isEmpty() && _matrixCallback)
    {
        _matrixCallback(alarm.matrixEffect);
    }

    // COB
    if (!alarm.cobEffect.isEmpty() && _cobCallback)
    {
        _cobCallback(alarm.cobEffect);
    }

    // AUDIO
    if (!alarm.audioEffect.isEmpty() && _audioCallback)
    {
        _audioCallback(alarm.audioEffect);
    }
}

// ============================================================
// STOP OUTPUTS
// ============================================================

void AlarmController::stopOutputs()
{
    Serial0.println("[ALARM][CONTROLLER] STOP OUTPUTS");

    if (_stopCallback)
    {
        _stopCallback();
    }
}
