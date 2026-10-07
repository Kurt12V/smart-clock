import { state } from "./state.js";

function showMessage(text) {

    const el =
        document.getElementById(
            "message"
        );

    el.textContent = text;

    el.classList.add("show");

    clearTimeout(
        state.messageTimer
    );

    state.messageTimer =
        setTimeout(
            () => {

                el.classList.remove(
                    "show"
                );

            },
            1200
        );
}

function setConnection(online) {

    const dot =
        document.getElementById(
            "connectionDot"
        );

    const text =
        document.getElementById(
            "connectionText"
        );

    dot.classList.toggle(
        "online",
        online
    );

    text.textContent =
        online
            ? "Online"
            : "Offline";
}

function setRange(
    id,
    value,
    valueId
) {

    const el =
        document.getElementById(id);

    if (
        !el ||
        value === undefined ||
        value === null
    ) {

        return;

    }

    el.value = value;

    if (valueId) {

        const valueEl =
            document.getElementById(
                valueId
            );

        if (valueEl) {

            valueEl.textContent =
                value + "%";

        }

    }
}

function applyToggle(
    id,
    value
) {

    const el =
        document.getElementById(id);

    if (!el) {
        return;
    }

    const on =
        value === 1 ||
        value === true ||
        value === "1";

    el.classList.toggle(
        "on",
        on
    );

    el.textContent =
        on
            ? "ON"
            : "OFF";
}
