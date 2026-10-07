import { state } from "../state.js";

import {
    padAlarmTimePart,
    clampAlarmTimePart,
    maskToDays
} from "./alarm-utils.js";


// ============================================================
// INITIALIZATION
// ============================================================

let initialized =
    false;


// ============================================================
// SHOW EDITOR
// ============================================================

export function showAlarmEditor()
{
    const alarm =
        state.currentAlarm;

    if (!alarm)
        return;


    const alarmsTab =
        document.getElementById(
            "alarmsTab"
        );

    const editor =
        document.getElementById(
            "alarmEditor"
        );


    if (alarmsTab)
    {
        alarmsTab.style.display =
            "none";
    }

    if (editor)
    {
        editor.style.display =
            "block";
    }


    initializeEditor();

    renderAlarmEditor(
        alarm
    );
}


// ============================================================
// INITIALIZE
// ============================================================

function initializeEditor()
{
    if (initialized)
        return;

    initialized =
        true;


    setupTimeInput(
        "alarm_hour",
        0,
        23,
        "alarm_minute",
        null
    );

    setupTimeInput(
        "alarm_minute",
        0,
        59,
        "alarm_second",
        "alarm_hour"
    );

    setupTimeInput(
        "alarm_second",
        0,
        59,
        null,
        "alarm_minute"
    );


    setupDayButtons();

    ensureEffectControls();
}


// ============================================================
// RENDER EDITOR
// ============================================================

function renderAlarmEditor(
    alarm
)
{
    const name =
        document.getElementById(
            "alarm_name"
        );

    if (name)
    {
        name.value =
            alarm.name || "";
    }


    const time =
        alarm.time ||
        {
            hour: 0,
            minute: 0,
            second: 0
        };


    setInputValue(
        "alarm_hour",
        padAlarmTimePart(
            clampAlarmTimePart(
                time.hour,
                0,
                23
            )
        )
    );

    setInputValue(
        "alarm_minute",
        padAlarmTimePart(
            clampAlarmTimePart(
                time.minute,
                0,
                59
            )
        )
    );

    setInputValue(
        "alarm_second",
        padAlarmTimePart(
            clampAlarmTimePart(
                time.second,
                0,
                59
            )
        )
    );


    applyAlarmToggle(
        alarm.enabled
    );


    maskToDays(
        alarm.repeatMask
    );


    setInputValue(
        "alarm_matrix_effect",
        alarm.matrixEffect || ""
    );

    setInputValue(
        "alarm_cob_effect",
        alarm.cobEffect || ""
    );

    setInputValue(
        "alarm_audio_effect",
        alarm.audioEffect || ""
    );
}


// ============================================================
// ENABLE TOGGLE
// ============================================================

function applyAlarmToggle(
    value
)
{
    const button =
        document.getElementById(
            "alarm_enabled"
        );

    if (!button)
        return;

    const enabled =
        Boolean(
            value
        );

    button.classList.toggle(
        "on",
        enabled
    );

    button.textContent =
        enabled
            ? "ON"
            : "OFF";
}


// ============================================================
// TOGGLE ENABLED
// ============================================================

export function toggleAlarmEnabled()
{
    if (!state.currentAlarm)
        return;

    state.currentAlarm.enabled =
        !state.currentAlarm.enabled;

    applyAlarmToggle(
        state.currentAlarm.enabled
    );
}


// ============================================================
// CLOSE EDITOR
// ============================================================

export function closeAlarmEditor()
{
    const editor =
        document.getElementById(
            "alarmEditor"
        );

    const alarmsTab =
        document.getElementById(
            "alarmsTab"
        );


    if (editor)
    {
        editor.style.display =
            "none";
    }

    if (alarmsTab)
    {
        alarmsTab.style.display =
            "block";
    }

    state.currentAlarm =
        null;
}


// ============================================================
// DAY BUTTONS
// ============================================================

function setupDayButtons()
{
    document
        .querySelectorAll(
            ".days button"
        )
        .forEach(
            button =>
            {
                if (
                    button.dataset.alarmDaysInitialized
                )
                {
                    return;
                }

                button.dataset.alarmDaysInitialized =
                    "1";

                button.addEventListener(
                    "click",
                    event =>
                    {
                        event.preventDefault();

                        button.classList.toggle(
                            "selected"
                        );
                    }
                );
            }
        );
}


// ============================================================
// EFFECT CONTROLS
// ============================================================

function ensureEffectControls()
{
    const editor =
        document.getElementById(
            "alarmEditor"
        );

    if (!editor)
        return;


    /*
     * If the HTML already contains these
     * controls, do not create them.
     */

    const existing =
        document.getElementById(
            "alarmEffects"
        );

    if (existing)
        return;


    const section =
        document.createElement(
            "section"
        );

    section.className =
        "section";

    section.id =
        "alarmEffects";


    const title =
        document.createElement(
            "div"
        );

    title.className =
        "section-title";

    title.textContent =
        "Effects";


    section.appendChild(
        title
    );


    section.appendChild(
        createEffectField(
            "alarm_matrix_effect",
            "Matrix effect",
            "matrix effect"
        )
    );

    section.appendChild(
        createEffectField(
            "alarm_cob_effect",
            "COB effect",
            "COB effect"
        )
    );

    section.appendChild(
        createEffectField(
            "alarm_audio_effect",
            "Audio effect",
            "audio file or effect"
        )
    );


    /*
     * Insert before the save area.
     */

    const save =
        editor.querySelector(
            ".alarm-save"
        );

    if (save)
    {
        editor.insertBefore(
            section,
            save
        );
    }
    else
    {
        editor.appendChild(
            section
        );
    }
}


// ============================================================
// CREATE EFFECT FIELD
// ============================================================

function createEffectField(
    id,
    labelText,
    placeholder
)
{
    const row =
        document.createElement(
            "div"
        );

    row.className =
        "form-row";


    const label =
        document.createElement(
            "label"
        );

    label.textContent =
        labelText;


    const input =
        document.createElement(
            "input"
        );

    input.id =
        id;

    input.type =
        "text";

    input.placeholder =
        placeholder;

    input.autocomplete =
        "off";


    row.appendChild(
        label
    );

    row.appendChild(
        input
    );


    return row;
}


// ============================================================
// TIME INPUT
// ============================================================

function setupTimeInput(
    id,
    min,
    max,
    nextId,
    prevId
)
{
    const input =
        document.getElementById(
            id
        );

    if (!input)
        return;


    if (
        input.dataset.timeInitialized
    )
    {
        return;
    }

    input.dataset.timeInitialized =
        "1";


    input.addEventListener(
        "input",
        () =>
        {
            let value =
                input.value
                    .replace(
                        /\D/g,
                        ""
                    )
                    .slice(
                        0,
                        2
                    );

            input.value =
                value;

            if (
                value.length === 2
            )
            {
                const number =
                    Number(
                        value
                    );

                input.value =
                    padAlarmTimePart(
                        Math.min(
                            max,
                            Math.max(
                                min,
                                number
                            )
                        )
                    );

                if (nextId)
                {
                    document
                        .getElementById(
                            nextId
                        )
                        ?.focus();
                }
            }
        }
    );


    input.addEventListener(
        "blur",
        () =>
        {
            if (
                input.value === ""
            )
            {
                input.value =
                    padAlarmTimePart(
                        min
                    );

                return;
            }

            input.value =
                padAlarmTimePart(
                    clampAlarmTimePart(
                        input.value,
                        min,
                        max
                    )
                );
        }
    );


    input.addEventListener(
        "keydown",
        event =>
        {
            if (
                event.key ===
                    "ArrowUp" ||
                event.key ===
                    "ArrowDown"
            )
            {
                event.preventDefault();

                let value =
                    Number(
                        input.value ||
                        min
                    );

                value +=
                    event.key ===
                        "ArrowUp"
                        ? 1
                        : -1;

                if (
                    value > max
                )
                {
                    value =
                        min;
                }

                if (
                    value < min
                )
                {
                    value =
                        max;
                }

                input.value =
                    padAlarmTimePart(
                        value
                    );

                return;
            }


            if (
                event.key ===
                "Enter"
            )
            {
                event.preventDefault();

                if (nextId)
                {
                    document
                        .getElementById(
                            nextId
                        )
                        ?.focus();
                }

                return;
            }


            if (
                event.key ===
                    "Backspace" &&
                input.value === "" &&
                prevId
            )
            {
                document
                    .getElementById(
                        prevId
                    )
                    ?.focus();
            }
        }
    );
}


// ============================================================
// HELPERS
// ============================================================

function setInputValue(
    id,
    value
)
{
    const element =
        document.getElementById(
            id
        );

    if (element)
    {
        element.value =
            value;
    }
}
