import { state } from "../state.js";
import { padAlarmTimePart, maskToDays } from "./alarm-utils.js";
import { renderAlarmPhases } from "./alarm-phases.js";

function showAlarmEditor() {

    document.getElementById(
        "alarmsTab"
    ).style.display =
        "none";

    document.getElementById(
        "alarmEditor"
    ).style.display =
        "block";


    document.getElementById(
        "alarm_name"
    ).value =
        state.currentAlarm.name ||
        "";


    const time =
        state.currentAlarm.time || {

            hour: 0,

            minute: 0,

            second: 0

        };


    document.getElementById(
        "alarm_hour"
    ).value =
        padAlarmTimePart(
            Math.max(
                0,
                Math.min(
                    23,
                    Number(
                        time.hour || 0
                    )
                )
            )
        );


    document.getElementById(
        "alarm_minute"
    ).value =
        padAlarmTimePart(
            Math.max(
                0,
                Math.min(
                    59,
                    Number(
                        time.minute || 0
                    )
                )
            )
        );


    document.getElementById(
        "alarm_second"
    ).value =
        padAlarmTimePart(
            Math.max(
                0,
                Math.min(
                    59,
                    Number(
                        time.second || 0
                    )
                )
            )
        );


    applyAlarmToggle(
        state.currentAlarm.enabled
    );


    maskToDays(
        state.currentAlarm.repeatMask
    );


    renderAlarmPhases();

}

function applyAlarmToggle(
    value
) {

    const button =
        document.getElementById(
            "alarm_enabled"
        );

    const enabled =
        Boolean(value);

    button.classList.toggle(
        "on",
        enabled
    );

    button.textContent =
        enabled
            ? "ON"
            : "OFF";
}

function toggleAlarmEnabled() {

    state.currentAlarm.enabled =
        !state.currentAlarm.enabled;

    applyAlarmToggle(
        state.currentAlarm.enabled
    );
}

function closeAlarmEditor() {

    state.currentAlarm = null;

    document.getElementById(
        "alarmEditor"
    ).style.display =
        "none";

    document.getElementById(
        "alarmsTab"
    ).style.display =
        "block";
}
