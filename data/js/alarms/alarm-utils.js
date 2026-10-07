function formatAlarmTime(time) {

    if (!time) {
        return "--:--";
    }

    const hour =
        Math.max(
            0,
            Math.min(
                23,
                Number(
                    time.hour || 0
                )
            )
        );

    const minute =
        Math.max(
            0,
            Math.min(
                59,
                Number(
                    time.minute || 0
                )
            )
        );

    return (
        String(hour).padStart(
            2,
            "0"
        ) +
        ":" +
        String(minute).padStart(
            2,
            "0"
        )
    );
}
function formatAlarmTimeWithSeconds(time) {

    if (!time) {
        return "00:00:00";
    }

    const hour =
        Math.max(
            0,
            Math.min(
                23,
                Number(
                    time.hour || 0
                )
            )
        );

    const minute =
        Math.max(
            0,
            Math.min(
                59,
                Number(
                    time.minute || 0
                )
            )
        );

    const second =
        Math.max(
            0,
            Math.min(
                59,
                Number(
                    time.second || 0
                )
            )
        );

    return (
        String(hour).padStart(
            2,
            "0"
        ) +
        ":" +
        String(minute).padStart(
            2,
            "0"
        ) +
        ":" +
        String(second).padStart(
            2,
            "0"
        )
    );
}
function padAlarmTimePart(
    value
) {

    return String(
        Math.max(
            0,
            Number(
                value || 0
            )
        )
    ).padStart(
        2,
        "0"
    );
}
function clampAlarmTimePart(
    value,
    min,
    max
) {

    const digits =
        String(
            value || ""
        )
            .replace(
                /\D/g,
                ""
            )
            .slice(
                0,
                2
            );

    if (!digits) {
        return "";
    }

    const number =
        Math.min(
            max,
            Math.max(
                min,
                Number(digits)
            )
        );

    return padAlarmTimePart(
        number
    );
}
function setupAlarmTimeInput(
    id,
    min,
    max,
    nextId,
    prevId
) {

    const input =
        document.getElementById(
            id
        );

    if (!input) {
        return;
    }


    input.addEventListener(
        "focus",
        () => {

            input.select();

        }
    );


    input.addEventListener(
        "input",
        () => {

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
            ) {

                const number =
                    Number(value);

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


                if (nextId) {

                    document.getElementById(
                        nextId
                    ).focus();

                }

            }

        }
    );


    input.addEventListener(
        "blur",
        () => {

            if (
                input.value === ""
            ) {

                input.value =
                    padAlarmTimePart(
                        min
                    );

                return;

            }

            input.value =
                clampAlarmTimePart(
                    input.value,
                    min,
                    max
                );

        }
    );


    input.addEventListener(
        "keydown",
        event => {

            /* UP / DOWN */

            if (
                event.key === "ArrowUp" ||
                event.key === "ArrowDown"
            ) {

                event.preventDefault();

                let value =
                    Number(
                        input.value || min
                    );

                value +=
                    event.key === "ArrowUp"
                        ? 1
                        : -1;


                if (
                    value > max
                ) {

                    value = min;

                }

                if (
                    value < min
                ) {

                    value = max;

                }


                input.value =
                    padAlarmTimePart(
                        value
                    );

                return;

            }


            /* ENTER */

            if (
                event.key === "Enter"
            ) {

                event.preventDefault();

                if (nextId) {

                    document.getElementById(
                        nextId
                    ).focus();

                }

                return;

            }


            /* BACKSPACE */

            if (
                event.key === "Backspace" &&
                input.value === "" &&
                prevId
            ) {

                document.getElementById(
                    prevId
                ).focus();

            }

        }
    );

}
function getAlarmTime() {

    const hour =
        Number(
            document.getElementById(
                "alarm_hour"
            ).value
        );

    const minute =
        Number(
            document.getElementById(
                "alarm_minute"
            ).value
        );

    const second =
        Number(
            document.getElementById(
                "alarm_second"
            ).value
        );


    if (
        !Number.isInteger(hour) ||
        hour < 0 ||
        hour > 23 ||

        !Number.isInteger(minute) ||
        minute < 0 ||
        minute > 59 ||

        !Number.isInteger(second) ||
        second < 0 ||
        second > 59
    ) {

        return null;

    }


    return {

        hour,

        minute,

        second

    };

}
function formatAlarmDays(mask) {

    mask =
        Number(mask || 0);

    if (mask === 0) {
        return "Once";
    }

    if (mask === 127) {
        return "Every day";
    }

    if (mask === 31) {
        return "Mon – Fri";
    }

    const result = [];

    for (
        let i = 0;
        i < 7;
        ++i
    ) {

        if (
            mask &
            (1 << i)
        ) {

            result.push(
                DAY_NAMES[i]
            );

        }

    }

    return result.join(
        " · "
    );
}
function minutesToMs(value) {

    return Math.round(
        Number(value || 0) *
        60000
    );
}
function secondsToMs(value) {

    return Math.round(
        Number(value || 0) *
        1000
    );
}
function msToMinutes(value) {

    return Number(
        Number(value || 0) /
        60000
    ).toFixed(2);
}
function msToSeconds(value) {

    return Number(
        Number(value || 0) /
        1000
    ).toFixed(2);
}
function daysToMask() {

    let mask = 0;

    document
        .querySelectorAll(
            ".days button.selected"
        )
        .forEach(
            button => {

                const day =
                    Number(
                        button.dataset.day
                    );

                mask |=
                    1 << day;

            }
        );

    return mask;
}
function maskToDays(mask) {

    document
        .querySelectorAll(
            ".days button"
        )
        .forEach(
            button => {

                const day =
                    Number(
                        button.dataset.day
                    );

                button.classList.toggle(
                    "selected",
                    (
                        mask &
                        (1 << day)
                    ) !== 0
                );

            }
        );
}
