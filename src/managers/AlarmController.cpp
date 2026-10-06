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
    Serial0.println(
        "[ALARM_CTRL] Constructor"
    );
}

// ============================================================
// BEGIN
// ============================================================

void AlarmController::begin()
{
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] begin()"
    );

    Serial0.printf(
        "[ALARM_CTRL] AlarmManager initialized: %s\n",
        _alarmManager.isInitialized()
            ? "YES"
            : "NO"
    );

    // --------------------------------------------------------
    // ALREADY INITIALIZED
    // --------------------------------------------------------

    if (_begun)
    {
        Serial0.println(
            "[ALARM_CTRL] Already initialized"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    // --------------------------------------------------------
    // TRIGGER CALLBACK
    // --------------------------------------------------------

    _alarmManager.setTriggerCallback(
        [this](const Alarm& alarm)
        {
            Serial0.printf(
                "[ALARM_CTRL] CALLBACK -> TRIGGER id=%s\n",
                alarm.id.c_str()
            );

            onTriggered(alarm);
        }
    );

    Serial0.println(
        "[ALARM_CTRL] Trigger callback attached"
    );

    // --------------------------------------------------------
    // FINISH CALLBACK
    // --------------------------------------------------------

    _alarmManager.setFinishCallback(
        [this](const String& id)
        {
            Serial0.printf(
                "[ALARM_CTRL] CALLBACK -> FINISH id=%s\n",
                id.c_str()
            );

            onFinished(id);
        }
    );

    Serial0.println(
        "[ALARM_CTRL] Finish callback attached"
    );

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    _begun = true;

    Serial0.println(
        "[ALARM_CTRL] Controller READY"
    );

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );
}

// ============================================================
// UPDATE
// ============================================================

void AlarmController::update()
{
    /*
     * AlarmController does not run the alarm scheduler.
     *
     * AlarmManager owns:
     *
     *   - schedule
     *   - time comparison
     *   - trigger detection
     *   - active alarm runtime
     *   - snooze
     *   - finish
     *
     * AlarmController owns:
     *
     *   - output callbacks
     *   - active alarm state
     *   - dismiss / snooze / stop commands
     *
     * There is currently no phase system and therefore
     * no per-frame controller update is required.
     */
}

// ============================================================
// CALLBACK SETTERS
// ============================================================

void AlarmController::setMatrixCallback(
    MatrixCallback callback
)
{
    _matrixCallback =
        std::move(callback);

    Serial0.printf(
        "[ALARM_CTRL] Matrix callback: %s\n",
        _matrixCallback
            ? "ATTACHED"
            : "CLEARED"
    );
}

// ------------------------------------------------------------

void AlarmController::setAudioCallback(
    AudioCallback callback
)
{
    _audioCallback =
        std::move(callback);

    Serial0.printf(
        "[ALARM_CTRL] Audio callback: %s\n",
        _audioCallback
            ? "ATTACHED"
            : "CLEARED"
    );
}

// ------------------------------------------------------------

void AlarmController::setCobCallback(
    CobCallback callback
)
{
    _cobCallback =
        std::move(callback);

    Serial0.printf(
        "[ALARM_CTRL] COB callback: %s\n",
        _cobCallback
            ? "ATTACHED"
            : "CLEARED"
    );
}

// ------------------------------------------------------------

void AlarmController::setStopCallback(
    StopCallback callback
)
{
    _stopCallback =
        std::move(callback);

    Serial0.printf(
        "[ALARM_CTRL] Stop callback: %s\n",
        _stopCallback
            ? "ATTACHED"
            : "CLEARED"
    );
}

// ============================================================
// STATE
// ============================================================

bool AlarmController::isActive() const
{
    return _active;
}

// ------------------------------------------------------------

const String& AlarmController::alarmId() const
{
    return _alarmId;
}

// ============================================================
// TRIGGER
// ============================================================

void AlarmController::onTriggered(
    const Alarm& alarm
)
{
    Serial0.println(
        "[ALARM_CTRL] ----------------------------------------"
    );

    Serial0.println(
        "[ALARM_CTRL] onTriggered()"
    );

    // --------------------------------------------------------
    // ALARM DATA
    // --------------------------------------------------------

    Serial0.printf(
        "[ALARM_CTRL] id=%s\n",
        alarm.id.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] name=%s\n",
        alarm.name.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] enabled=%s\n",
        alarm.enabled
            ? "true"
            : "false"
    );

    Serial0.printf(
        "[ALARM_CTRL] time=%02u:%02u:%02u\n",
        static_cast<unsigned>(alarm.time.hour),
        static_cast<unsigned>(alarm.time.minute),
        static_cast<unsigned>(alarm.time.second)
    );

    Serial0.printf(
        "[ALARM_CTRL] repeatMask=0x%02X\n",
        static_cast<unsigned>(alarm.repeatMask)
    );

    Serial0.printf(
        "[ALARM_CTRL] matrixEffect=%s\n",
        alarm.matrixEffect.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] cobEffect=%s\n",
        alarm.cobEffect.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] audioEffect=%s\n",
        alarm.audioEffect.c_str()
    );

    // --------------------------------------------------------
    // PREVIOUS STATE
    // --------------------------------------------------------

    Serial0.printf(
        "[ALARM_CTRL] previous active=%s id=%s\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str()
    );

    // --------------------------------------------------------
    // ACTIVATE
    // --------------------------------------------------------

    _active = true;
    _alarmId = alarm.id;

    Serial0.printf(
        "[ALARM_CTRL] state -> ACTIVE id=%s\n",
        _alarmId.c_str()
    );

    // --------------------------------------------------------
    // EXECUTE OUTPUTS
    // --------------------------------------------------------

    executeAlarm(alarm);

    Serial0.println(
        "[ALARM_CTRL] ----------------------------------------"
    );
}

// ============================================================
// EXECUTE ALARM
// ============================================================

void AlarmController::executeAlarm(
    const Alarm& alarm
)
{
    Serial0.println(
        "[ALARM_CTRL] executeAlarm()"
    );

    // ========================================================
    // MATRIX
    // ========================================================

    Serial0.println(
        "[ALARM_CTRL] MATRIX"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        alarm.matrixEffect.c_str()
    );

    if (alarm.matrixEffect.isEmpty())
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Matrix effect EMPTY"
        );
    }
    else if (_matrixCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Matrix callback"
        );

        _matrixCallback(
            alarm.matrixEffect
        );

        Serial0.println(
            "[ALARM_CTRL]   <- Matrix callback"
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Matrix callback NOT SET"
        );
    }

    // ========================================================
    // COB
    // ========================================================

    Serial0.println(
        "[ALARM_CTRL] COB"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        alarm.cobEffect.c_str()
    );

    if (alarm.cobEffect.isEmpty())
    {
        Serial0.println(
            "[ALARM_CTRL]   -> COB effect EMPTY"
        );
    }
    else if (_cobCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> COB callback"
        );

        _cobCallback(
            alarm.cobEffect
        );

        Serial0.println(
            "[ALARM_CTRL]   <- COB callback"
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> COB callback NOT SET"
        );
    }

    // ========================================================
    // AUDIO
    // ========================================================

    Serial0.println(
        "[ALARM_CTRL] AUDIO"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        alarm.audioEffect.c_str()
    );

    if (alarm.audioEffect.isEmpty())
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Audio effect EMPTY"
        );
    }
    else if (_audioCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Audio callback"
        );

        _audioCallback(
            alarm.audioEffect
        );

        Serial0.println(
            "[ALARM_CTRL]   <- Audio callback"
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Audio callback NOT SET"
        );
    }

    Serial0.println(
        "[ALARM_CTRL] executeAlarm() complete"
    );
}

// ============================================================
// FINISH
// ============================================================

void AlarmController::onFinished(
    const String& id
)
{
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] onFinished()"
    );

    Serial0.printf(
        "[ALARM_CTRL] callback id=%s\n",
        id.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] controller active=%s\n",
        _active
            ? "true"
            : "false"
    );

    Serial0.printf(
        "[ALARM_CTRL] controller id=%s\n",
        _alarmId.c_str()
    );

    // --------------------------------------------------------
    // ALREADY INACTIVE
    // --------------------------------------------------------

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] FINISH ignored:"
            " controller inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    // --------------------------------------------------------
    // ID VALIDATION
    // --------------------------------------------------------

    if (id != _alarmId)
    {
        Serial0.printf(
            "[ALARM_CTRL] FINISH ignored:"
            " ID mismatch"
            " controller=%s"
            " callback=%s\n",
            _alarmId.c_str(),
            id.c_str()
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    // --------------------------------------------------------
    // STOP OUTPUTS
    // --------------------------------------------------------

    Serial0.println(
        "[ALARM_CTRL] Stopping outputs"
    );

    stopOutputs();

    // --------------------------------------------------------
    // CLEAR STATE
    // --------------------------------------------------------

    _active = false;
    _alarmId = String();

    Serial0.println(
        "[ALARM_CTRL] state -> INACTIVE"
    );

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );
}

// ============================================================
// STOP OUTPUTS
// ============================================================

void AlarmController::stopOutputs()
{
    Serial0.println(
        "[ALARM_CTRL] stopOutputs()"
    );

    if (_stopCallback)
    {
        Serial0.println(
            "[ALARM_CTRL] -> Stop callback"
        );

        _stopCallback();

        Serial0.println(
            "[ALARM_CTRL] <- Stop callback"
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL] Stop callback NOT SET"
        );
    }
}

// ============================================================
// DISMISS
// ============================================================

bool AlarmController::dismiss()
{
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] dismiss()"
    );

    Serial0.printf(
        "[ALARM_CTRL] active=%s id=%s\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str()
    );

    // --------------------------------------------------------
    // VALIDATE
    // --------------------------------------------------------

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] DISMISS rejected:"
            " controller inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    // --------------------------------------------------------
    // MANAGER
    // --------------------------------------------------------

    const bool result =
        _alarmManager.dismiss();

    Serial0.printf(
        "[ALARM_CTRL] AlarmManager::dismiss() -> %s\n",
        result
            ? "SUCCESS"
            : "FAILED"
    );

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    return result;
}

// ============================================================
// SNOOZE
// ============================================================

bool AlarmController::snooze(
    uint32_t durationMs
)
{
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] snooze()"
    );

    Serial0.printf(
        "[ALARM_CTRL] active=%s id=%s\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] durationMs=%lu\n",
        static_cast<unsigned long>(durationMs)
    );

    // --------------------------------------------------------
    // VALIDATE ACTIVE
    // --------------------------------------------------------

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] SNOOZE rejected:"
            " controller inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    // --------------------------------------------------------
    // VALIDATE DURATION
    // --------------------------------------------------------

    if (durationMs == 0)
    {
        Serial0.println(
            "[ALARM_CTRL] SNOOZE rejected:"
            " duration=0"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    // --------------------------------------------------------
    // MANAGER
    // --------------------------------------------------------

    const bool result =
        _alarmManager.snooze(durationMs);

    Serial0.printf(
        "[ALARM_CTRL] AlarmManager::snooze(%lu) -> %s\n",
        static_cast<unsigned long>(durationMs),
        result
            ? "SUCCESS"
            : "FAILED"
    );

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    return result;
}

// ============================================================
// STOP
// ============================================================

void AlarmController::stop()
{
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] stop()"
    );

    Serial0.printf(
        "[ALARM_CTRL] active=%s id=%s\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str()
    );

    // --------------------------------------------------------
    // VALIDATE ACTIVE
    // --------------------------------------------------------

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] STOP ignored:"
            " controller inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    // --------------------------------------------------------
    // FINISH
    // --------------------------------------------------------

    Serial0.printf(
        "[ALARM_CTRL] Requesting "
        "AlarmManager::finish() id=%s\n",
        _alarmId.c_str()
    );

    _alarmManager.finish();

    Serial0.println(
        "[ALARM_CTRL] AlarmManager::finish() returned"
    );

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );
}