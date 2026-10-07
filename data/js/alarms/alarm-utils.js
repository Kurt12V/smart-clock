// ============================================================
// DAY NAMES
// ============================================================

export const DAY_NAMES =
[
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun"
];


// ============================================================
// FORMAT TIME
// ============================================================

export function formatAlarmTime(
    time
)
{
    if (!time)
        return "--:--:--";

    const hour =
        padAlarmTimePart(
            time.hour
        );

    const minute =
        padAlarmTimePart(
            time.minute
        );

    const second =
        padAlarmTimePart(
            time.second
        );

    return `${hour}:${minute}:${second}`;
}


// ============================================================
// FORMAT TIME WITH OPTIONAL SECONDS
// ============================================================

export function formatAlarmTimeWithSeconds(
    time
)
{
    return formatAlarmTime(
        time
    );
}


// ============================================================
// PAD TIME PART
// ============================================================

export function padAlarmTimePart(
    value
)
{
    const number =
        Number(
            value
        );

    if (!Number.isFinite(number))
        return "00";

    return String(
        Math.trunc(
            number
        )
    ).padStart(
        2,
        "0"
    );
}


// ============================================================
// CLAMP TIME PART
// ============================================================

export function clampAlarmTimePart(
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


// ============================================================
// GET TIME FROM EDITOR
// ============================================================

export function getAlarmTime()
{
    const hourElement =
        document.getElementById(
            "alarm_hour"
        );

    const minuteElement =
        document.getElementById(
            "alarm_minute"
        );

    const secondElement =
        document.getElementById(
            "alarm_second"
        );


    if (
        !hourElement ||
        !minuteElement ||
        !secondElement
    )
    {
        return null;
    }


    const hour =
        Number(
            hourElement.value
        );

    const minute =
        Number(
            minuteElement.value
        );

    const second =
        Number(
            secondElement.value
        );


    if (
        !Number.isInteger(hour) ||
        hour < 0 ||
        hour > 23
    )
    {
        return null;
    }

    if (
        !Number.isInteger(minute) ||
        minute < 0 ||
        minute > 59
    )
    {
        return null;
    }

    if (
        !Number.isInteger(second) ||
        second < 0 ||
        second > 59
    )
    {
        return null;
    }


    return {
        hour,
        minute,
        second
    };
}


// ============================================================
// FORMAT DAYS
// ============================================================

export function formatAlarmDays(
    mask
)
{
    mask =
        Number(
            mask || 0
        );


    if (mask === 0)
    {
        return "Once";
    }


    if (mask === 127)
    {
        return "Every day";
    }


    if (mask === 31)
    {
        return "Mon – Fri";
    }


    const result =
        [];


    for (
        let i = 0;
        i < 7;
        ++i
    )
    {
        if (
            mask &
            (1 << i)
        )
        {
            result.push(
                DAY_NAMES[i]
            );
        }
    }


    return result.length
        ? result.join(" · ")
        : "Once";
}


// ============================================================
// DAYS -> MASK
// ============================================================

export function daysToMask()
{
    let mask =
        0;


    document
        .querySelectorAll(
            ".days button.selected"
        )
        .forEach(
            button =>
            {
                const day =
                    Number(
                        button.dataset.day
                    );

                if (
                    Number.isInteger(
                        day
                    ) &&
                    day >= 0 &&
                    day <= 6
                )
                {
                    mask |=
                        1 << day;
                }
            }
        );


    return mask;
}


// ============================================================
// MASK -> DAYS
// ============================================================

export function maskToDays(
    mask
)
{
    mask =
        Number(
            mask || 0
        );


    document
        .querySelectorAll(
            ".days button"
        )
        .forEach(
            button =>
            {
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