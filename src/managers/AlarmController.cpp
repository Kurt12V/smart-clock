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
    // PHASE CALLBACK
    // --------------------------------------------------------

    _alarmManager.setPhaseCallback(
        [this](
            const Alarm& alarm,
            uint8_t phaseIndex,
            const AlarmPhase& phase
        )
        {
            Serial0.printf(
                "[ALARM_CTRL] CALLBACK -> PHASE "
                "id=%s index=%u\n",
                alarm.id.c_str(),
                static_cast<unsigned>(phaseIndex)
            );

            onPhaseChanged(
                alarm,
                phaseIndex,
                phase
            );
        }
    );

    Serial0.println(
        "[ALARM_CTRL] Phase callback attached"
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
     * AlarmController не управляет временем.
     *
     * AlarmManager отвечает за:
     *
     *   schedule
     *   trigger
     *   phase timing
     *   finish
     *
     * Controller только получает callbacks
     * и управляет выходными подсистемами.
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

const String& AlarmController::alarmId() const
{
    return _alarmId;
}

uint8_t AlarmController::phaseIndex() const
{
    return _phaseIndex;
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
        alarm.time.hour,
        alarm.time.minute,
        alarm.time.second
    );

    Serial0.printf(
        "[ALARM_CTRL] repeatMask=0x%02X\n",
        alarm.repeatMask
    );

    Serial0.printf(
        "[ALARM_CTRL] phaseCount=%u\n",
        static_cast<unsigned>(
            alarm.phaseCount
        )
    );

    Serial0.printf(
        "[ALARM_CTRL] previous active=%s id=%s phase=%u\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str(),
        static_cast<unsigned>(
            _phaseIndex
        )
    );

    _active = true;
    _alarmId = alarm.id;
    _phaseIndex = 0;

    Serial0.printf(
        "[ALARM_CTRL] state -> ACTIVE id=%s\n",
        _alarmId.c_str()
    );

    Serial0.println(
        "[ALARM_CTRL] ----------------------------------------"
    );
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
    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );

    Serial0.println(
        "[ALARM_CTRL] onPhaseChanged()"
    );

    Serial0.printf(
        "[ALARM_CTRL] alarm id=%s\n",
        alarm.id.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] controller id=%s\n",
        _alarmId.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL] phase index=%u\n",
        static_cast<unsigned>(
            phaseIndex
        )
    );

    Serial0.printf(
        "[ALARM_CTRL] controller active=%s\n",
        _active
            ? "true"
            : "false"
    );

    // --------------------------------------------------------
    // VALIDATION
    // --------------------------------------------------------

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] PHASE IGNORED: controller inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    if (alarm.id != _alarmId)
    {
        Serial0.printf(
            "[ALARM_CTRL] PHASE IGNORED: ID mismatch "
            "controller=%s callback=%s\n",
            _alarmId.c_str(),
            alarm.id.c_str()
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    if (
        phaseIndex >=
        alarm.phaseCount
    )
    {
        Serial0.printf(
            "[ALARM_CTRL] PHASE IGNORED: invalid index "
            "index=%u phaseCount=%u\n",
            static_cast<unsigned>(phaseIndex),
            static_cast<unsigned>(alarm.phaseCount)
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    // --------------------------------------------------------
    // UPDATE STATE
    // --------------------------------------------------------

    _phaseIndex =
        phaseIndex;

    Serial0.printf(
        "[ALARM_CTRL] state -> phase=%u\n",
        static_cast<unsigned>(
            _phaseIndex
        )
    );

    // --------------------------------------------------------
    // PHASE DATA
    // --------------------------------------------------------

    Serial0.printf(
        "[ALARM_CTRL] phase offset=%ld ms\n",
        static_cast<long>(
            phase.startOffsetMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL] phase duration=%lu ms\n",
        static_cast<unsigned long>(
            phase.durationMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL] phase condition=%u\n",
        static_cast<unsigned>(
            phase.condition
        )
    );

    // --------------------------------------------------------
    // OUTPUTS
    // --------------------------------------------------------

    executePhase(phase);

    Serial0.println(
        "[ALARM_CTRL] ========================================"
    );
}

// ============================================================
// EXECUTE PHASE
// ============================================================

void AlarmController::executePhase(
    const AlarmPhase& phase
)
{
    Serial0.println(
        "[ALARM_CTRL] executePhase()"
    );

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    Serial0.println(
        "[ALARM_CTRL] MATRIX"
    );

    Serial0.printf(
        "[ALARM_CTRL]   enabled=%s\n",
        phase.matrix.enabled
            ? "true"
            : "false"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        phase.matrix.effectId.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL]   range=%u-%u\n",
        static_cast<unsigned>(
            phase.matrix.start
        ),
        static_cast<unsigned>(
            phase.matrix.end
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   speedMs=%lu\n",
        static_cast<unsigned long>(
            phase.matrix.speedMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   durationMs=%lu\n",
        static_cast<unsigned long>(
            phase.matrix.durationMs
        )
    );

    if (_matrixCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Matrix callback"
        );

        _matrixCallback(
            phase.matrix
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Matrix callback NOT SET"
        );
    }

    // --------------------------------------------------------
    // AUDIO
    // --------------------------------------------------------

    Serial0.println(
        "[ALARM_CTRL] AUDIO"
    );

    Serial0.printf(
        "[ALARM_CTRL]   enabled=%s\n",
        phase.audio.enabled
            ? "true"
            : "false"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        phase.audio.effectId.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL]   range=%u-%u\n",
        static_cast<unsigned>(
            phase.audio.start
        ),
        static_cast<unsigned>(
            phase.audio.end
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   speedMs=%lu\n",
        static_cast<unsigned long>(
            phase.audio.speedMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   durationMs=%lu\n",
        static_cast<unsigned long>(
            phase.audio.durationMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   loop=%s\n",
        phase.audio.loop
            ? "true"
            : "false"
    );

    if (_audioCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Audio callback"
        );

        _audioCallback(
            phase.audio
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> Audio callback NOT SET"
        );
    }

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    Serial0.println(
        "[ALARM_CTRL] COB"
    );

    Serial0.printf(
        "[ALARM_CTRL]   enabled=%s\n",
        phase.cob.enabled
            ? "true"
            : "false"
    );

    Serial0.printf(
        "[ALARM_CTRL]   effect=%s\n",
        phase.cob.effectId.c_str()
    );

    Serial0.printf(
        "[ALARM_CTRL]   range=%u-%u\n",
        static_cast<unsigned>(
            phase.cob.start
        ),
        static_cast<unsigned>(
            phase.cob.end
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   speedMs=%lu\n",
        static_cast<unsigned long>(
            phase.cob.speedMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   durationMs=%lu\n",
        static_cast<unsigned long>(
            phase.cob.durationMs
        )
    );

    Serial0.printf(
        "[ALARM_CTRL]   maxDurationMs=%lu\n",
        static_cast<unsigned long>(
            phase.cob.maxDurationMs
        )
    );

    if (_cobCallback)
    {
        Serial0.println(
            "[ALARM_CTRL]   -> COB callback"
        );

        _cobCallback(
            phase.cob
        );
    }
    else
    {
        Serial0.println(
            "[ALARM_CTRL]   -> COB callback NOT SET"
        );
    }

    Serial0.println(
        "[ALARM_CTRL] executePhase() complete"
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

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] FINISH ignored: already inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    if (id != _alarmId)
    {
        Serial0.printf(
            "[ALARM_CTRL] FINISH ignored: ID mismatch "
            "controller=%s callback=%s\n",
            _alarmId.c_str(),
            id.c_str()
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    Serial0.println(
        "[ALARM_CTRL] Stopping outputs"
    );

    stopOutputs();

    _active = false;
    _alarmId = String();
    _phaseIndex = 0;

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
            "[ALARM_CTRL] Stop callback complete"
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
        "[ALARM_CTRL] active=%s id=%s phase=%u\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str(),
        static_cast<unsigned>(
            _phaseIndex
        )
    );

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] DISMISS rejected: inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    const bool result =
        _alarmManager.dismiss();

    Serial0.printf(
        "[ALARM_CTRL] dismiss result=%s\n",
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
        "[ALARM_CTRL] active=%s id=%s phase=%u\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str(),
        static_cast<unsigned>(
            _phaseIndex
        )
    );

    Serial0.printf(
        "[ALARM_CTRL] durationMs=%lu\n",
        static_cast<unsigned long>(
            durationMs
        )
    );

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] SNOOZE rejected: inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    if (durationMs == 0)
    {
        Serial0.println(
            "[ALARM_CTRL] SNOOZE rejected: duration=0"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return false;
    }

    const bool result =
        _alarmManager.snooze(
            durationMs
        );

    Serial0.printf(
        "[ALARM_CTRL] snooze result=%s\n",
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
        "[ALARM_CTRL] active=%s id=%s phase=%u\n",
        _active
            ? "true"
            : "false",
        _alarmId.c_str(),
        static_cast<unsigned>(
            _phaseIndex
        )
    );

    if (!_active)
    {
        Serial0.println(
            "[ALARM_CTRL] STOP ignored: inactive"
        );

        Serial0.println(
            "[ALARM_CTRL] ========================================"
        );

        return;
    }

    Serial0.printf(
        "[ALARM_CTRL] Requesting AlarmManager::finish() "
        "id=%s\n",
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