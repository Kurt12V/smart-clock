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
     * Время и переходы фаз контролирует AlarmManager.
     *
     * AlarmController здесь ничего не вычисляет.
     *
     * Метод оставлен для будущих задач:
     * например, плавного управления эффектами.
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
        callback;
}


// ============================================================
// SET AUDIO CALLBACK
// ============================================================

void AlarmController::setAudioCallback(
    AudioCallback callback
)
{
    _audioCallback =
        callback;
}


// ============================================================
// SET COB CALLBACK
// ============================================================

void AlarmController::setCobCallback(
    CobCallback callback
)
{
    _cobCallback =
        callback;
}


// ============================================================
// SET STOP CALLBACK
// ============================================================

void AlarmController::setStopCallback(
    StopCallback callback
)
{
    _stopCallback =
        callback;
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
// TRIGGERED
// ============================================================

void AlarmController::onTriggered(
    const Alarm& alarm
)
{
    _active = true;

    _alarmId = alarm.id;

    _phaseIndex = 0;
}


// ============================================================
// PHASE CHANGED
// ============================================================

void AlarmController::onPhaseChanged(
    const Alarm& alarm,
    uint8_t phaseIndex,
    const AlarmPhase& phase
)
{
    // --------------------------------------------------------
    // Проверяем, что это текущий будильник
    // --------------------------------------------------------

    if (alarm.id != _alarmId)
        return;


    _active = true;

    _phaseIndex = phaseIndex;


    // --------------------------------------------------------
    // Применяем временный override
    // --------------------------------------------------------

    executePhase(phase);
}


// ============================================================
// EXECUTE PHASE
// ============================================================

void AlarmController::executePhase(
    const AlarmPhase& phase
)
{
    // ========================================================
    // MATRIX
    // ========================================================

    if (_matrixCallback)
    {
        _matrixCallback(
            phase.matrix
        );
    }


    // ========================================================
    // AUDIO
    // ========================================================

    if (_audioCallback)
    {
        _audioCallback(
            phase.audio
        );
    }


    // ========================================================
    // COB
    // ========================================================

    if (_cobCallback)
    {
        _cobCallback(
            phase.cob
        );
    }
}


// ============================================================
// FINISHED
// ============================================================

void AlarmController::onFinished(
    const String& id
)
{
    if (id != _alarmId)
        return;


    // --------------------------------------------------------
    // Сначала убираем alarm override
    // --------------------------------------------------------

    stopOutputs();


    // --------------------------------------------------------
    // Сбрасываем runtime Controller
    // --------------------------------------------------------

    _active = false;

    _alarmId = "";

    _phaseIndex = 0;
}


// ============================================================
// STOP OUTPUTS
// ============================================================

void AlarmController::stopOutputs()
{
    if (_stopCallback)
    {
        /*
         * ВАЖНО:
         *
         * Здесь не изменяем SettingsManager.
         *
         * StopCallback должен убрать alarm override
         * и вернуть Matrix / COB / Sound к обычному
         * EffectiveState.
         */

        _stopCallback();
    }
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