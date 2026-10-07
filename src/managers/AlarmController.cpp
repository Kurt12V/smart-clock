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

    // --------------------------------------------------------
    // AlarmManager -> Controller
    // --------------------------------------------------------

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

    Serial0.println(
        "[ALARM][CONTROLLER] READY"
    );
}

// ============================================================
// UPDATE
// ============================================================

void AlarmController::update()
{
    if (!_begun)
        return;

    // --------------------------------------------------------
    // AlarmManager является владельцем scheduler/runtime.
    //
    // Controller здесь ничего самостоятельно
    // не планирует и не отслеживает.
    //
    // Оставляем метод для единого lifecycle API.
    // --------------------------------------------------------

    if (!_alarmManager.isRunning() &&
        _active)
    {
        stopOutputs();

        _active = false;

        _alarmId = String();
    }
}

// ============================================================
// SET MATRIX CALLBACK
// ============================================================

void AlarmController::setMatrixCallback(
    MatrixCallback callback
)
{
    _matrixCallback =
        std::move(callback);
}

// ============================================================
// SET AUDIO CALLBACK
// ============================================================

void AlarmController::setAudioCallback(
    AudioCallback callback
)
{
    _audioCallback =
        std::move(callback);
}

// ============================================================
// SET COB CALLBACK
// ============================================================

void AlarmController::setCobCallback(
    CobCallback callback
)
{
    _cobCallback =
        std::move(callback);
}

// ============================================================
// SET STOP CALLBACK
// ============================================================

void AlarmController::setStopCallback(
    StopCallback callback
)
{
    _stopCallback =
        std::move(callback);
}

// ============================================================
// IS ACTIVE
// ============================================================

bool AlarmController::isActive() const
{
    return _active;
}

// ============================================================
// ALARM ID
// ============================================================

const String& AlarmController::alarmId() const
{
    return _alarmId;
}

// ============================================================
// DISMISS
// ============================================================

bool AlarmController::dismiss()
{
    if (!_begun)
        return false;

    if (!_active)
        return false;

    return _alarmManager.dismiss();
}

// ============================================================
// SNOOZE
// ============================================================

bool AlarmController::snooze(
    uint32_t durationMs
)
{
    if (!_begun)
        return false;

    if (!_active)
        return false;

    return _alarmManager.snooze(
        durationMs
    );
}

// ============================================================
// STOP
// ============================================================

void AlarmController::stop()
{
    if (!_begun)
        return;

    // --------------------------------------------------------
    // Если AlarmManager считает alarm активным,
    // завершаем его через AlarmManager.
    //
    // Finish callback вызовет onFinished(),
    // который остановит outputs.
    // --------------------------------------------------------

    if (_alarmManager.isRunning())
    {
        _alarmManager.finish();

        return;
    }

    // --------------------------------------------------------
    // Защита от рассинхронизации.
    // --------------------------------------------------------

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

void AlarmController::onTriggered(
    const Alarm& alarm
)
{
    if (!_begun)
        return;

    // --------------------------------------------------------
    // Защита от повторного trigger.
    // --------------------------------------------------------

    if (_active)
    {
        Serial0.printf(
            "[ALARM][CONTROLLER] "
            "TRIGGER IGNORED "
            "already active id=%s\n",
            _alarmId.c_str()
        );

        return;
    }

    _active = true;

    _alarmId =
        alarm.id;

    Serial0.printf(
        "[ALARM][CONTROLLER] "
        "TRIGGER id=%s "
        "matrix=%s "
        "cob=%s "
        "audio=%s\n",

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

void AlarmController::onFinished(
    const String& id
)
{
    if (!_begun)
        return;

    Serial0.printf(
        "[ALARM][CONTROLLER] "
        "FINISH id=%s\n",
        id.c_str()
    );

    // --------------------------------------------------------
    // Останавливаем все выходы.
    // --------------------------------------------------------

    stopOutputs();

    // --------------------------------------------------------
    // Controller state
    // --------------------------------------------------------

    _active = false;

    _alarmId = String();
}

// ============================================================
// EXECUTE ALARM
// ============================================================

void AlarmController::executeAlarm(
    const Alarm& alarm
)
{
    // ========================================================
    // MATRIX
    // ========================================================

    if (!alarm.matrixEffect.isEmpty())
    {
        if (_matrixCallback)
        {
            _matrixCallback(
                alarm.matrixEffect
            );
        }
    }

    // ========================================================
    // COB
    // ========================================================

    if (!alarm.cobEffect.isEmpty())
    {
        if (_cobCallback)
        {
            _cobCallback(
                alarm.cobEffect
            );
        }
    }

    // ========================================================
    // AUDIO
    // ========================================================

    if (!alarm.audioEffect.isEmpty())
    {
        if (_audioCallback)
        {
            _audioCallback(
                alarm.audioEffect
            );
        }
    }
}

// ============================================================
// STOP OUTPUTS
// ============================================================

void AlarmController::stopOutputs()
{
    // --------------------------------------------------------
    // Stop callback является общим callback'ом остановки.
    //
    // Его задача на стороне App:
    //
    // - остановить matrix alarm override
    // - остановить COB alarm override
    // - остановить alarm audio
    // - вернуть обычное состояние устройств
    // --------------------------------------------------------

    if (_stopCallback)
    {
        _stopCallback();
    }
}
