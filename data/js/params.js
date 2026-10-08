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
// PARAMETER NORMALIZATION
// ============================================================
//
// Поддерживаем несколько вариантов ответа API:
//
// 1. Плоский:
// {
//     "brightness": 80,
//     "mx_on": 1,
//     ...
// }
//
// 2. Объект params:
// {
//     "ok": true,
//     "params": {
//         "brightness": 80,
//         "mx_on": 1,
//         ...
//     }
// }
//
// 3. Массив params:
//
// {
//     "ok": true,
//     "params": [
//         {
//             "name": "mx_on",
//             "value": 1
//         },
//         ...
//     ]
// }
//
// Нормализуем всё в:
//
// {
//     brightness: 80,
//     mx_on: 1,
//     ...
// }
//
// ВАЖНО:
// эта функция НИКОГДА ничего не отправляет на ESP32.
// ============================================================

function normalizeParameters(data)
{
    if (!data || typeof data !== "object")
    {
        return {};
    }


    // --------------------------------------------------------
    // Плоский объект
    // --------------------------------------------------------

    if (
        data.params === undefined &&
        !Array.isArray(data)
    )
    {
        return data;
    }


    const params =
        data.params;


    // --------------------------------------------------------
    // params: object
    // --------------------------------------------------------

    if (
        params &&
        typeof params === "object" &&
        !Array.isArray(params)
    )
    {
        return params;
    }


    // --------------------------------------------------------
    // params: array
    // --------------------------------------------------------

    if (Array.isArray(params))
    {
        const result = {};

        for (const item of params)
        {
            if (
                !item ||
                typeof item !== "object"
            )
            {
                continue;
            }


            const name =
                item.name;


            if (!name)
            {
                continue;
            }


            // API может использовать value
            if (
                item.value !== undefined
            )
            {
                result[name] =
                    item.value;

                continue;
            }


            // На случай другого формата
            if (
                item.current !== undefined
            )
            {
                result[name] =
                    item.current;

                continue;
            }


            if (
                item.currentValue !== undefined
            )
            {
                result[name] =
                    item.currentValue;

                continue;
            }
        }

        return result;
    }


    return {};
}


// ============================================================
// VALUE HELPERS
// ============================================================

function hasValue(
    object,
    name
)
{
    return (
        object &&
        Object.prototype.hasOwnProperty.call(
            object,
            name
        )
    );
}


function numberValue(
    object,
    name
)
{
    if (!hasValue(object, name))
    {
        return null;
    }

    const value =
        Number(object[name]);

    if (!Number.isFinite(value))
    {
        return null;
    }

    return value;
}


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


        const rawData =
            await response.json();


        console.log(
            "[PARAMS] Raw response:",
            rawData
        );


        // ----------------------------------------------------
        // NORMALIZE API RESPONSE
        // ----------------------------------------------------

        const data =
            normalizeParameters(
                rawData
            );


        console.log(
            "[PARAMS] Normalized:",
            data
        );


        // ----------------------------------------------------
        // VALIDATE RESPONSE
        // ----------------------------------------------------

        if (
            rawData &&
            rawData.ok === false
        )
        {
            throw new Error(
                rawData.error ||
                "Failed to load parameters"
            );
        }


        // ----------------------------------------------------
        // DISPLAY
        // ----------------------------------------------------

        const brightness =
            numberValue(
                data,
                "brightness"
            );

        if (brightness !== null)
        {
            setRange(
                "brightness",
                brightness,
                "brightness_value"
            );
        }


        // ----------------------------------------------------
        // MATRIX
        // ----------------------------------------------------

        if (
            hasValue(
                data,
                "mx_on"
            )
        )
        {
            applyToggle(
                "mx_on",
                data.mx_on
            );
        }


        const matrixBrightness =
            numberValue(
                data,
                "mx_br"
            );

        if (matrixBrightness !== null)
        {
            setRange(
                "mx_br",
                matrixBrightness,
                "mx_br_value"
            );
        }


        if (
            hasValue(
                data,
                "mx_eff"
            )
        )
        {
            const matrixEffect =
                document.getElementById(
                    "mx_eff"
                );

            if (matrixEffect)
            {
                matrixEffect.value =
                    String(data.mx_eff);
            }
        }


        const matrixSpeed =
            numberValue(
                data,
                "mx_spd"
            );

        if (matrixSpeed !== null)
        {
            setRange(
                "mx_spd",
                matrixSpeed,
                "mx_spd_value"
            );
        }


        // ----------------------------------------------------
        // COB
        // ----------------------------------------------------

        if (
            hasValue(
                data,
                "cob_on"
            )
        )
        {
            applyToggle(
                "cob_on",
                data.cob_on
            );
        }


        const cobBrightness =
            numberValue(
                data,
                "cob_br"
            );

        if (cobBrightness !== null)
        {
            setRange(
                "cob_br",
                cobBrightness,
                "cob_br_value"
            );
        }


        if (
            hasValue(
                data,
                "cob_eff"
            )
        )
        {
            const cobEffect =
                document.getElementById(
                    "cob_eff"
                );

            if (cobEffect)
            {
                cobEffect.value =
                    String(data.cob_eff);
            }
        }


        const cobSpeed =
            numberValue(
                data,
                "cob_spd"
            );

        if (cobSpeed !== null)
        {
            setRange(
                "cob_spd",
                cobSpeed,
                "cob_spd_value"
            );
        }


        // ----------------------------------------------------
        // VOLUME
        // ----------------------------------------------------

        const mediaVolume =
            numberValue(
                data,
                "vol_media"
            );

        if (mediaVolume !== null)
        {
            setRange(
                "vol_media",
                mediaVolume,
                "vol_media_value"
            );
        }


        const alarmVolume =
            numberValue(
                data,
                "vol_alarm"
            );

        if (alarmVolume !== null)
        {
            setRange(
                "vol_alarm",
                alarmVolume,
                "vol_alarm_value"
            );
        }


        const systemVolume =
            numberValue(
                data,
                "vol_system"
            );

        if (systemVolume !== null)
        {
            setRange(
                "vol_system",
                systemVolume,
                "vol_system_value"
            );
        }


        // ----------------------------------------------------
        // MICROPHONE
        // ----------------------------------------------------

        if (
            hasValue(
                data,
                "mic_on"
            )
        )
        {
            applyToggle(
                "mic_on",
                data.mic_on
            );
        }


        // ----------------------------------------------------
        // TIMEZONE
        // ----------------------------------------------------

        const utc =
            numberValue(
                data,
                "utc"
            );

        if (utc !== null)
        {
            state.timezoneOffset =
                utc;

            renderTimezone();
        }


        // ----------------------------------------------------
        // SUCCESS
        // ----------------------------------------------------

        setConnection(true);

        console.log(
            "[PARAMS] Parameters loaded successfully"
        );
    }
    catch (error)
    {
        console.error(
            "[PARAMS] Load failed:",
            error
        );

        setConnection(false);

        showMessage(
            "Failed to load settings"
        );
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
    value =
        Number(value);


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
        oldState
            ? 0
            : 1;


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
    value =
        Number(value);


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
        Number(value)
    );
}


// ============================================================
// MATRIX SPEED
// ============================================================

export function changeMatrixSpeed(
    value
)
{
    value =
        Number(value);


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
        oldState
            ? 0
            : 1;


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
    value =
        Number(value);


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
        Number(value)
    );
}


// ============================================================
// COB SPEED
// ============================================================

export function changeCobSpeed(
    value
)
{
    value =
        Number(value);


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
    value =
        Number(value);


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
        oldState
            ? 0
            : 1;


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