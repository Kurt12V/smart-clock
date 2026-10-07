import { state } from "./state.js";
import { apiFetch, debounce, setParam } from "./api.js";
import { setConnection, setRange, applyToggle, showMessage } from "./ui.js";

async function loadParams() {

    if (state.paramsRequest) {
        return;
    }

    state.paramsRequest = true;

    try {

        const response =
            await apiFetch(
                "/api/params",
                {},
                1500
            );

        const p =
            await response.json();


        /* DISPLAY */

        setRange(
            "brightness",
            p.brightness,
            "brightness_value"
        );


        /* MATRIX */

        applyToggle(
            "mx_on",
            p.mx_on
        );

        setRange(
            "mx_br",
            p.mx_br,
            "mx_br_value"
        );

        const matrixEffect =
            document.getElementById(
                "mx_eff"
            );

        if (
            matrixEffect &&
            p.mx_eff !== undefined
        ) {

            matrixEffect.value =
                p.mx_eff;

        }

        setRange(
            "mx_spd",
            p.mx_spd,
            "mx_spd_value"
        );


        /* COB */

        applyToggle(
            "cob_on",
            p.cob_on
        );

        setRange(
            "cob_br",
            p.cob_br,
            "cob_br_value"
        );

        const effect =
            document.getElementById(
                "cob_eff"
            );

        if (
            effect &&
            p.cob_eff !== undefined
        ) {

            effect.value =
                p.cob_eff;

        }

        setRange(
            "cob_spd",
            p.cob_spd,
            "cob_spd_value"
        );


        /* VOLUME */

        setRange(
            "vol_media",
            p.vol_media,
            "vol_media_value"
        );

        setRange(
            "vol_alarm",
            p.vol_alarm,
            "vol_alarm_value"
        );

        setRange(
            "vol_system",
            p.vol_system,
            "vol_system_value"
        );


        /* MICROPHONE */

        applyToggle(
            "mic_on",
            p.mic_on
        );


        /* TIMEZONE */

        if (
            p.utc !== undefined
        ) {

            state.timezoneOffset =
                Number(p.utc);

            renderTimezone();

        }

        setConnection(true);

    } catch (e) {

        setConnection(false);

    } finally {

        state.paramsRequest = false;

    }
}

function renderTimezone() {

    const sign =
        state.timezoneOffset >= 0
            ? "+"
            : "";

    document.getElementById(
        "timezone"
    ).textContent =
        "UTC " +
        sign +
        state.timezoneOffset;
}

async function changeTimezone(delta) {

    let value =
        state.timezoneOffset + delta;

    if (value < -12) {
        value = -12;
    }

    if (value > 14) {
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

    if (!result) {

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

function changeBrightness(value) {

    document.getElementById(
        "brightness_value"
    ).textContent =
        value + "%";

    debounce(
        "brightness",
        () =>
            setParam(
                "brightness",
                value
            )
    );
}

async function toggleMatrix() {

    const el =
        document.getElementById(
            "mx_on"
        );

    const oldState =
        el.classList.contains(
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

    if (!result) {

        applyToggle(
            "mx_on",
            oldState
        );

    }
}

function changeMatrixBrightness(
    value
) {

    document.getElementById(
        "mx_br_value"
    ).textContent =
        value + "%";

    debounce(
        "mx_br",
        () =>
            setParam(
                "mx_br",
                value
            )
    );
}

function changeMatrixEffect(
    value
) {

    setParam(
        "mx_eff",
        value
    );
}

function changeMatrixSpeed(
    value
) {

    document.getElementById(
        "mx_spd_value"
    ).textContent =
        value + "%";

    debounce(
        "mx_spd",
        () =>
            setParam(
                "mx_spd",
                value
            )
    );
}

async function toggleCob() {

    const el =
        document.getElementById(
            "cob_on"
        );

    const oldState =
        el.classList.contains(
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

    if (!result) {

        applyToggle(
            "cob_on",
            oldState
        );

    }
}

function changeCobBrightness(
    value
) {

    document.getElementById(
        "cob_br_value"
    ).textContent =
        value + "%";

    debounce(
        "cob_br",
        () =>
            setParam(
                "cob_br",
                value
            )
    );
}

function changeCobEffect(
    value
) {

    setParam(
        "cob_eff",
        value
    );
}

function changeCobSpeed(
    value
) {

    document.getElementById(
        "cob_spd_value"
    ).textContent =
        value + "%";

    debounce(
        "cob_spd",
        () =>
            setParam(
                "cob_spd",
                value
            )
    );
}

function changeVolume(
    name,
    value
) {

    document.getElementById(
        name + "_value"
    ).textContent =
        value + "%";

    debounce(
        name,
        () =>
            setParam(
                name,
                value
            ),
        60
    );
}

async function toggleMic() {

    const el =
        document.getElementById(
            "mic_on"
        );

    const oldState =
        el.classList.contains(
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

    if (!result) {

        applyToggle(
            "mic_on",
            oldState
        );

    }
}
