import { state } from "../state.js";
import { apiFetch } from "../api.js";
import { showMessage, setConnection } from "../ui.js";
import { formatAlarmTime, formatAlarmDays, daysToMask, getAlarmTime } from "./alarm-utils.js";
import { showAlarmEditor, closeAlarmEditor } from "./alarm-editor.js";
import { readPhase } from "./alarm-phases.js";

async function loadAlarms() {

    const list =
        document.getElementById(
            "alarmList"
        );

    list.innerHTML =
        '<div class="empty">Loading...</div>';

    try {

        const response =
            await apiFetch(
                "/api/state.alarms",
                {},
                2000
            );

        const data =
            await response.json();

        state.alarms =
            Array.isArray(data.alarms)
                ? data.alarms
                : [];

        renderAlarmList();

        setConnection(true);

    } catch (e) {

        list.innerHTML =
            '<div class="empty">Unable to load state.alarms</div>';

    }
}

function renderAlarmList() {

    const list =
        document.getElementById(
            "alarmList"
        );

    list.replaceChildren();

    if (!state.alarms.length) {

        list.innerHTML =
            '<div class="empty">No state.alarms</div>';

        return;

    }


    state.alarms.forEach(
        alarm => {

            const row =
                document.createElement(
                    "div"
                );

            row.className =
                "alarm-item";

            row.onclick =
                () =>
                    openAlarmEditor(
                        alarm.id
                    );


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


            const toggle =
                document.createElement(
                    "button"
                );

            toggle.className =
                "toggle";

            const enabled =
                Boolean(
                    alarm.enabled
                );

            toggle.textContent =
                enabled
                    ? "ON"
                    : "OFF";

            toggle.classList.toggle(
                "on",
                enabled
            );


            toggle.onclick =
                async event => {

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

            list.appendChild(
                row
            );

        }
    );
}

function createAlarm() {

    state.currentAlarm = {

        schemaVersion: 1,

        id: null,

        name:
            "New alarm",

        enabled:
            false,

        time: {

            hour:
                7,

            minute:
                30,

            second:
                0

        },

        repeatMask:
            31,

        phases: [

            {

                startOffsetMs:
                    0,

                durationMs:
                    30000,

                condition:
                    "always",

                matrix: {

                    enabled:
                        true,

                    effectId:
                        "static",

                    start:
                        0,

                    end:
                        100,

                    speedMs:
                        0,

                    durationMs:
                        30000

                },

                audio: {

                    enabled:
                        false,

                    effectId:
                        "melody",

                    start:
                        0,

                    end:
                        100,

                    speedMs:
                        0,

                    durationMs:
                        30000,

                    loop:
                        false

                },

                cob: {

                    enabled:
                        false,

                    effectId:
                        "static",

                    start:
                        0,

                    end:
                        100,

                    speedMs:
                        0,

                    durationMs:
                        30000,

                    maxDurationMs:
                        0

                }

            }

        ]

    };

    showAlarmEditor();

}

async function openAlarmEditor(id) {

    try {

        const response =
            await apiFetch(
                "/api/state.alarms/" +
                encodeURIComponent(
                    id
                ),
                {},
                1500
            );

        state.currentAlarm =
            await response.json();

        showAlarmEditor();

    } catch (e) {

        showMessage(
            "Unable to load alarm"
        );

    }
}

async function saveCurrentAlarm() {

    if (!state.currentAlarm) {
        return;
    }


    state.currentAlarm.name =
        document.getElementById(
            "alarm_name"
        ).value.trim();


    if (!state.currentAlarm.name) {

        showMessage(
            "Enter alarm name"
        );

        return;

    }


    const parsedTime =
        getAlarmTime();


    if (!parsedTime) {

        showMessage(
            "Invalid time"
        );

        document.getElementById(
            "alarm_hour"
        ).focus();

        return;

    }


    state.currentAlarm.time =
        parsedTime;


    state.currentAlarm.repeatMask =
        daysToMask();


    state.currentAlarm.phases.forEach(
        (
            phase,
            index
        ) => {

            readPhase(
                phase,
                index
            );

        }
    );


    try {

        const method =
            state.currentAlarm.id
                ? "PUT"
                : "POST";


        const url =
            state.currentAlarm.id
                ? "/api/state.alarms/" +
                  encodeURIComponent(
                      state.currentAlarm.id
                  )
                : "/api/state.alarms";


        const response =
            await apiFetch(
                url,
                {

                    method,

                    headers: {

                        "Content-Type":
                            "application/json"

                    },

                    body:
                        JSON.stringify(
                            state.currentAlarm
                        )

                },

                2000
            );


        const result =
            await response.json();


        if (!result.ok) {

            throw new Error(
                result.error ||
                "Save failed"
            );

        }


        showMessage(
            "Alarm saved"
        );


        closeAlarmEditor();


        await loadAlarms();


    } catch (e) {

        showMessage(
            "Alarm save error"
        );

    }
}

async function setAlarmEnabled(
    id,
    enabled
) {

    try {

        const response =
            await apiFetch(
                "/api/state.alarms/" +
                encodeURIComponent(
                    id
                ) +
                "/enabled",
                {

                    method: "POST",

                    headers: {

                        "Content-Type":
                            "application/json"

                    },

                    body:
                        JSON.stringify({
                            enabled
                        })

                },

                1200
            );


        const result =
            await response.json();


        if (!result.ok) {

            throw new Error();

        }


        await loadAlarms();


    } catch (e) {

        showMessage(
            "Unable to change alarm"
        );

    }
}

async function deleteCurrentAlarm() {

    if (!state.currentAlarm) {
        return;
    }


    if (
        !confirm(
            "Delete this alarm?"
        )
    ) {

        return;

    }


    try {

        const response =
            await apiFetch(
                "/api/state.alarms/" +
                encodeURIComponent(
                    state.currentAlarm.id
                ),
                {

                    method:
                        "DELETE"

                },

                1500
            );


        const result =
            await response.json();


        if (!result.ok) {

            throw new Error();

        }


        showMessage(
            "Alarm deleted"
        );


        closeAlarmEditor();


        await loadAlarms();


    } catch (e) {

        showMessage(
            "Delete error"
        );

    }
}
