
import { state } from "../state.js";
import { apiFetch } from "../api.js";
import { showMessage, setConnection } from "../ui.js";

import {
    formatAlarmTime,
    formatAlarmDays,
    daysToMask,
    getAlarmTime
} from "./alarm-utils.js";

import {
    showAlarmEditor,
    closeAlarmEditor
} from "./alarm-editor.js";


// ============================================================
// API
// ============================================================

const ALARMS_API = "/api/alarms";


// ============================================================
// LOAD ALARMS
// ============================================================

export async function loadAlarms()
{
    const list =
        document.getElementById(
            "alarmList"
        );

    if (list)
    {
        list.innerHTML =
            '<div class="empty">Loading...</div>';
    }

    try
    {
        const response =
            await apiFetch(
                ALARMS_API
            );

        const data =
            await response.json();

        /*
         * Backend may return either:
         *
         * [
         *   {...},
         *   {...}
         * ]
         *
         * or:
         *
         * {
         *   alarms: [...]
         * }
         */

        if (Array.isArray(data))
        {
            state.alarms = data;
        }
        else if (
            data &&
            Array.isArray(data.alarms)
        )
        {
            state.alarms = data.alarms;
        }
        else
        {
            state.alarms = [];
        }

        renderAlarmList();

        setConnection(true);
    }
    catch (error)
    {
        console.error(
            "[ALARMS] Load failed:",
            error
        );

        state.alarms = [];

        if (list)
        {
            list.innerHTML =
                '<div class="empty">Unable to load alarms</div>';
        }

        setConnection(false);

        showMessage(
            "Unable to load alarms"
        );
    }
}


// ============================================================
// RENDER ALARM LIST
// ============================================================

function renderAlarmList()
{
    const list =
        document.getElementById(
            "alarmList"
        );

    if (!list)
        return;

    list.replaceChildren();

    if (!state.alarms.length)
    {
        list.innerHTML =
            '<div class="empty">No alarms</div>';

        return;
    }

    state.alarms.forEach(
        alarm =>
        {
            renderAlarmItem(
                list,
                alarm
            );
        }
    );
}


// ============================================================
// RENDER SINGLE ALARM
// ============================================================

function renderAlarmItem(
    container,
    alarm
)
{
    const row =
        document.createElement(
            "div"
        );

    row.className =
        "alarm-item";

    row.dataset.id =
        alarm.id || "";

    row.onclick =
        () =>
            openAlarmEditor(
                alarm.id
            );


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    const time =
        document.createElement(
            "div"
        );

    time.className =
        "alarm-time";

    time.textContent =
        formatAlarmTime(
            alarm.time
        );


    // --------------------------------------------------------
    // INFO
    // --------------------------------------------------------

    const info =
        document.createElement(
            "div"
        );

    info.className =
        "alarm-info";


    const name =
        document.createElement(
            "div"
        );

    name.className =
        "alarm-name";

    name.textContent =
        alarm.name ||
        "Alarm";


    const days =
        document.createElement(
            "div"
        );

    days.className =
        "alarm-days";

    days.textContent =
        formatAlarmDays(
            alarm.repeatMask
        );


    info.appendChild(
        name
    );

    info.appendChild(
        days
    );


    // --------------------------------------------------------
    // TOGGLE
    // --------------------------------------------------------

    const toggle =
        document.createElement(
            "button"
        );

    toggle.type =
        "button";

    toggle.className =
        "toggle";

    const enabled =
        Boolean(
            alarm.enabled
        );

    toggle.classList.toggle(
        "on",
        enabled
    );

    toggle.textContent =
        enabled
            ? "ON"
            : "OFF";

    toggle.onclick =
        async event =>
        {
            event.stopPropagation();

            await setAlarmEnabled(
                alarm.id,
                !enabled
            );
        };


    row.appendChild(
        time
    );

    row.appendChild(
        info
    );

    row.appendChild(
        toggle
    );

    container.appendChild(
        row
    );
}


// ============================================================
// CREATE ALARM
// ============================================================

export function createAlarm()
{
    state.currentAlarm =
    {
        schemaVersion: 1,

        id: null,

        name:
            "New alarm",

        enabled:
            false,

        time:
        {
            hour: 7,
            minute: 30,
            second: 0
        },

        repeatMask:
            31,

        matrixEffect:
            "",

        cobEffect:
            "",

        audioEffect:
            ""
    };

    showAlarmEditor();
}


// ============================================================
// OPEN ALARM
// ============================================================

export async function openAlarmEditor(
    id
)
{
    if (!id)
        return;

    try
    {
        const response =
            await apiFetch(
                `${ALARMS_API}/${encodeURIComponent(id)}`
            );

        const alarm =
            await response.json();

        state.currentAlarm =
            normalizeAlarm(
                alarm
            );

        showAlarmEditor();
    }
    catch (error)
    {
        console.error(
            "[ALARMS] Open failed:",
            error
        );

        showMessage(
            "Unable to load alarm"
        );
    }
}


// ============================================================
// NORMALIZE ALARM
// ============================================================

function normalizeAlarm(
    alarm
)
{
    const source =
        alarm || {};

    const time =
        source.time || {};

    return {
        schemaVersion:
            Number(
                source.schemaVersion || 1
            ),

        id:
            source.id || null,

        name:
            source.name || "Alarm",

        enabled:
            Boolean(
                source.enabled
            ),

        time:
        {
            hour:
                clamp(
                    time.hour,
                    0,
                    23
                ),

            minute:
                clamp(
                    time.minute,
                    0,
                    59
                ),

            second:
                clamp(
                    time.second,
                    0,
                    59
                )
        },

        repeatMask:
            clamp(
                source.repeatMask,
                0,
                127
            ),

        matrixEffect:
            typeof source.matrixEffect === "string"
                ? source.matrixEffect
                : "",

        cobEffect:
            typeof source.cobEffect === "string"
                ? source.cobEffect
                : "",

        audioEffect:
            typeof source.audioEffect === "string"
                ? source.audioEffect
                : ""
    };
}


// ============================================================
// SAVE CURRENT ALARM
// ============================================================

export async function saveCurrentAlarm()
{
    const alarm =
        state.currentAlarm;

    if (!alarm)
        return;


    // --------------------------------------------------------
    // READ FORM
    // --------------------------------------------------------

    const name =
        document.getElementById(
            "alarm_name"
        );

    if (name)
    {
        alarm.name =
            name.value.trim();
    }


    const time =
        getAlarmTime();

    if (!time)
    {
        showMessage(
            "Invalid alarm time"
        );

        return;
    }

    alarm.time =
        time;


    alarm.repeatMask =
        daysToMask();


    const matrixEffect =
        document.getElementById(
            "alarm_matrix_effect"
        );

    const cobEffect =
        document.getElementById(
            "alarm_cob_effect"
        );

    const audioEffect =
        document.getElementById(
            "alarm_audio_effect"
        );


    alarm.matrixEffect =
        matrixEffect
            ? matrixEffect.value.trim()
            : "";

    alarm.cobEffect =
        cobEffect
            ? cobEffect.value.trim()
            : "";

    alarm.audioEffect =
        audioEffect
            ? audioEffect.value.trim()
            : "";


    // --------------------------------------------------------
    // BUILD JSON
    // --------------------------------------------------------

    const payload =
    {
        schemaVersion:
            1,

        id:
            alarm.id,

        name:
            alarm.name,

        enabled:
            Boolean(
                alarm.enabled
            ),

        time:
        {
            hour:
                Number(
                    alarm.time.hour
                ),

            minute:
                Number(
                    alarm.time.minute
                ),

            second:
                Number(
                    alarm.time.second
                )
        },

        repeatMask:
            Number(
                alarm.repeatMask
            ),

        matrixEffect:
            alarm.matrixEffect,

        cobEffect:
            alarm.cobEffect,

        audioEffect:
            alarm.audioEffect
    };


    // --------------------------------------------------------
    // CREATE / UPDATE
    // --------------------------------------------------------

    const isNew =
        !alarm.id;

    const url =
        isNew
            ? ALARMS_API
            : `${ALARMS_API}/${encodeURIComponent(alarm.id)}`;

    const method =
        isNew
            ? "POST"
            : "PUT";


    try
    {
        const response =
            await apiFetch(
                url,
                {
                    method,

                    headers:
                    {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(
                            payload
                        )
                }
            );

        const saved =
            await response.json();


        /*
         * Backend returns the saved Alarm.
         */

        if (saved)
        {
            state.currentAlarm =
                normalizeAlarm(
                    saved
                );
        }


        showMessage(
            "Alarm saved"
        );

        setConnection(
            true
        );

        await loadAlarms();

        closeAlarmEditor();
    }
    catch (error)
    {
        console.error(
            "[ALARMS] Save failed:",
            error
        );

        showMessage(
            "Unable to save alarm"
        );
    }
}


// ============================================================
// ENABLE / DISABLE
// ============================================================

export async function setAlarmEnabled(
    id,
    enabled
)
{
    if (!id)
        return false;

    try
    {
        const response =
            await apiFetch(
                `${ALARMS_API}/${encodeURIComponent(id)}/enabled`,
                {
                    method:
                        "POST",

                    headers:
                    {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(
                            {
                                enabled:
                                    Boolean(
                                        enabled
                                    )
                            }
                        )
                }
            );

        const result =
            await response.json();


        /*
         * Update local list immediately.
         */

        const alarm =
            state.alarms.find(
                item =>
                    item.id === id
            );

        if (alarm)
        {
            alarm.enabled =
                Boolean(
                    enabled
                );
        }


        renderAlarmList();

        setConnection(
            true
        );

        return (
            result?.ok !== false
        );
    }
    catch (error)
    {
        console.error(
            "[ALARMS] Enable failed:",
            error
        );

        showMessage(
            "Unable to change alarm"
        );

        return false;
    }
}


// ============================================================
// DELETE CURRENT ALARM
// ============================================================

export async function deleteCurrentAlarm()
{
    const alarm =
        state.currentAlarm;

    if (!alarm)
        return;


    if (!alarm.id)
    {
        closeAlarmEditor();

        return;
    }


    const confirmed =
        window.confirm(
            "Delete this alarm?"
        );

    if (!confirmed)
        return;


    try
    {
        await apiFetch(
            `${ALARMS_API}/${encodeURIComponent(alarm.id)}`,
            {
                method:
                    "DELETE"
            }
        );


        state.currentAlarm =
            null;

        showMessage(
            "Alarm deleted"
        );

        await loadAlarms();

        closeAlarmEditor();
    }
    catch (error)
    {
        console.error(
            "[ALARMS] Delete failed:",
            error
        );

        showMessage(
            "Unable to delete alarm"
        );
    }
}


// ============================================================
// HELPERS
// ============================================================

function clamp(
    value,
    min,
    max
)
{
    const number =
        Number(
            value
        );

    if (!Number.isFinite(number))
        return min;

    return Math.min(
        max,
        Math.max(
            min,
            Math.trunc(
                number
            )
        )
    );
}
