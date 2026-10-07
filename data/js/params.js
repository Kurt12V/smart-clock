// ============================================================
// SMARTCLOCK PARAMETERS
// ============================================================

import { state } from "./state.js";

import {
    apiFetch,
    debounce,
    setParam
} from "./api.js";

import {
    setConnection,
    setRange,
    applyToggle,
    showMessage
} from "./ui.js";


// ============================================================
// LOAD ALL PARAMETERS
// ============================================================

export async function loadParams()
{
    if (state.paramsRequest)
    {
        return;
    }

    state.paramsRequest = true;

    try
    {
        console.log(
            "[PARAMS] Loading parameters..."
        );

        const response =
            await apiFetch(
                "/api/params"
            );

        const data =
            await response.json();

        console.log(
            "[PARAMS] Received:",
            data
        );


        // ----------------------------------------------------
        // DISPLAY
        // ----------------------------------------------------

        setRange(
            "brightness",
            data.brightness,
            "brightness_value"
        );


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        applyToggle(
            "mx_on",
            data.mx_on
        );

        setRange(
            "mx_br",
            data.mx_br,
            "mx_br_value"
        );

        const matrixEffect =
            document.getElementById(
                "mx_eff"
            );

        if (
            matrixEffect &&
            data.mx_eff !== undefined
        )
        {
            matrixEffect.value =
                data.mx_eff;
        }

        setRange(
            "mx_spd",
            data.mx_spd,
            "mx_spd_value"
        );


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        applyToggle(
            "cob_on",
            data.cob_on
        );

        setRange(
            "cob_br",
            data.cob_br,
            "cob_br_value"
        );

        const cobEffect =
            document.getElementById(
                "cob_eff"
            );

        if (
            cobEffect &&
            data.cob_eff !== undefined
        )
        {
            cobEffect.value =
                data.cob_eff;
        }

        setRange(
            "cob_spd",
            data.cob_spd,
            "cob_spd_value"
        );


        // ----------------------------------------------------
        // VOLUME
        // ----------------------------------------------------

        setRange(
            "vol_media",
            data.vol_media,
            "vol_media_value"
        );

        setRange(
            "vol_alarm",
            data.vol_alarm,
            "vol_alarm_value"
        );

        setRange(
            "vol_system",
            data.vol_system,
            "vol_system_value"
        );


        // ----------------------------------------------------
        // MICROPHONE
        // ----------------------------------------------------

        applyToggle(
            "mic_on",
            data.mic_on
        );


        // ----------------------------------------------------
        // TIMEZONE
        // ----------------------------------------------------

        if (
            data.utc !== undefined
        )
        {
            state.timezoneOffset =
                Number(data.utc);

            renderTimezone();
        }


        setConnection(true);

        console.log(
            "[PARAMS] Parameters loaded"
        );
    }
    catch (error)
    {
        console.error(
            "[PARAMS] Load failed:",
            error
        );

        setConnection(false);
    }
    finally
    {
        state.paramsRequest = false;
    }
}


// ============================================================
// TIMEZONE DISPLAY
// ============================================================

export function renderTimezone()
{
    const element =
        document.getElementById(
            "timezone"
        );

    if (!element)
    {
        return;
    }

    const sign =
        state.timezoneOffset >= 0
            ? "+"
            : "";

    element.textContent =
        "UTC " +
        sign +
        state.timezoneOffset;
}


// ============================================================
// TIMEZONE
// ============================================================

export async function changeTimezone(
    delta
)
{
    let value =
        state.timezoneOffset +
        Number(delta);

    if (value < -12)
    {
        value = -12;
    }

    if (value > 14)
    {
        value = 14;
    }

    const oldValue =
        state.timezoneOffset;

    state.timezoneOffset =
        value;

    renderTimezone();

    const result =
        await setParam(
            "utc",
            value,
            false
        );

    if (!result)
    {
        state.timezoneOffset =
            oldValue;

        renderTimezone();

        showMessage(
            "Timezone error"
        );

        return;
    }

    showMessage(
        "Timezone changed"
    );
}


// ============================================================
// DISPLAY BRIGHTNESS
// ============================================================

export function changeBrightness(
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            "brightness_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        "brightness",
        () =>
        {
            setParam(
                "brightness",
                value
            );
        }
    );
}


// ============================================================
// MATRIX TOGGLE
// ============================================================

export async function toggleMatrix()
{
    const element =
        document.getElementById(
            "mx_on"
        );

    if (!element)
    {
        return;
    }

    const oldState =
        element.classList.contains(
            "on"
        );

    const newState =
        oldState ? 0 : 1;

    applyToggle(
        "mx_on",
        newState
    );

    const result =
        await setParam(
            "mx_on",
            newState,
            false
        );

    if (!result)
    {
        applyToggle(
            "mx_on",
            oldState
        );
    }
}


// ============================================================
// MATRIX BRIGHTNESS
// ============================================================

export function changeMatrixBrightness(
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            "mx_br_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        "mx_br",
        () =>
        {
            setParam(
                "mx_br",
                value
            );
        }
    );
}


// ============================================================
// MATRIX EFFECT
// ============================================================

export function changeMatrixEffect(
    value
)
{
    setParam(
        "mx_eff",
        value
    );
}


// ============================================================
// MATRIX SPEED
// ============================================================

export function changeMatrixSpeed(
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            "mx_spd_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        "mx_spd",
        () =>
        {
            setParam(
                "mx_spd",
                value
            );
        }
    );
}


// ============================================================
// COB TOGGLE
// ============================================================

export async function toggleCob()
{
    const element =
        document.getElementById(
            "cob_on"
        );

    if (!element)
    {
        return;
    }

    const oldState =
        element.classList.contains(
            "on"
        );

    const newState =
        oldState ? 0 : 1;

    applyToggle(
        "cob_on",
        newState
    );

    const result =
        await setParam(
            "cob_on",
            newState,
            false
        );

    if (!result)
    {
        applyToggle(
            "cob_on",
            oldState
        );
    }
}


// ============================================================
// COB BRIGHTNESS
// ============================================================

export function changeCobBrightness(
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            "cob_br_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        "cob_br",
        () =>
        {
            setParam(
                "cob_br",
                value
            );
        }
    );
}


// ============================================================
// COB EFFECT
// ============================================================

export function changeCobEffect(
    value
)
{
    setParam(
        "cob_eff",
        value
    );
}


// ============================================================
// COB SPEED
// ============================================================

export function changeCobSpeed(
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            "cob_spd_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        "cob_spd",
        () =>
        {
            setParam(
                "cob_spd",
                value
            );
        }
    );
}


// ============================================================
// VOLUME
// ============================================================

export function changeVolume(
    name,
    value
)
{
    value = Number(value);

    const element =
        document.getElementById(
            name + "_value"
        );

    if (element)
    {
        element.textContent =
            value + "%";
    }

    debounce(
        name,
        () =>
        {
            setParam(
                name,
                value
            );
        },
        60
    );
}


// ============================================================
// MICROPHONE TOGGLE
// ============================================================

export async function toggleMic()
{
    const element =
        document.getElementById(
            "mic_on"
        );

    if (!element)
    {
        return;
    }

    const oldState =
        element.classList.contains(
            "on"
        );

    const newState =
        oldState ? 0 : 1;

    applyToggle(
        "mic_on",
        newState
    );

    const result =
        await setParam(
            "mic_on",
            newState,
            false
        );

    if (!result)
    {
        applyToggle(
            "mic_on",
            oldState
        );
    }
}