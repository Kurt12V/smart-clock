#include "AlarmController.h"


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
    _alarmManager.setTriggerCallback(
        [this](const Alarm& alarm)
        {
            onTriggered(alarm);
        }
    );


    _alarmManager.setPhaseCallback(
        [this](
            const Alarm& alarm,
            uint8_t phaseIndex,
            const AlarmPhase& phase
        )
        {
            onPhaseChanged(
                alarm,
                phaseIndex,
                phase
            );
        }
    );


    _alarmManager.setFinishCallback(
        [this](const String& id)
        {
            onFinished(id);
        }
    );
}


// ============================================================
// UPDATE
// ============================================================

void AlarmController::update()
{
    /*
     * AlarmController не управляет временем.
     *
     * Все переходы:
     *
     *   alarm start
     *   phase start
     *   alarm finish
     *
     * выполняются AlarmManager.
     *
     * Метод оставлен для будущих controller-level
     * эффектов и должен вызываться из main loop.
     */
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
// PHASE INDEX
// ============================================================

uint8_t AlarmController::phaseIndex() const
{
    return _phaseIndex;
}


// ============================================================
// ON TRIGGERED
// ============================================================

void AlarmController::onTriggered(
    const Alarm& alarm
)
{
    _active =
        true;

    _alarmId =
        alarm.id;

    _phaseIndex =
        0;
}


// ============================================================
// ON PHASE CHANGED
// ============================================================

void AlarmController::onPhaseChanged(
    const Alarm& alarm,
    uint8_t phaseIndex,
    const AlarmPhase& phase
)
{
    if (!_active)
        return;


    if (
        alarm.id !=
        _alarmId
    )
    {
        return;
    }


    _phaseIndex =
        phaseIndex;


    executePhase(
        phase
    );
}


// ============================================================
// EXECUTE PHASE
// ============================================================

void AlarmController::executePhase(
    const AlarmPhase& phase
)
{
    if (_matrixCallback)
    {
        _matrixCallback(
            phase.matrix
        );
    }


    if (_audioCallback)
    {
        _audioCallback(
            phase.audio
        );
    }


    if (_cobCallback)
    {
        _cobCallback(
            phase.cob
        );
    }
}


// ============================================================
// ON FINISHED
// ============================================================

void AlarmController::onFinished(
    const String& id
)
{
    if (!_active)
        return;


    if (
        id !=
        _alarmId
    )
    {
        return;
    }


    stopOutputs();


    _active =
        false;

    _alarmId =
        String();

    _phaseIndex =
        0;
}


// ============================================================
// STOP OUTPUTS
// ============================================================

void AlarmController::stopOutputs()
{
    if (!_stopCallback)
        return;


    /*
     * Controller не меняет SettingsManager.
     *
     * StopCallback должен:
     *
     * 1. удалить alarm override;
     * 2. пересчитать EffectiveState;
     * 3. вернуть Matrix / COB / Audio
     *    к обычному состоянию.
     */

    _stopCallback();
}


// ============================================================
// DISMISS
// ============================================================

bool AlarmController::dismiss()
{
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
    if (!_active)
        return false;


    if (durationMs == 0)
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
    if (!_active)
        return;


    _alarmManager.finish();
}