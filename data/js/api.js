import { showMessage, setConnection } from "./ui.js";
import { state } from "./state.js";

async function apiFetch(
    url,
    options = {}

function debounce(
    name,
    callback,
    delay = 80
) {

    clearTimeout(
        state.debounceTimers[name]
    );

    state.debounceTimers[name] =
        setTimeout(
            callback,
            delay
        );
}

async function setParam(
    name,
    value,
    silent = true
) {

    try {

        const response =
            await apiFetch(
                "/api/param",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify({
                            param: name,
                            value:
                                Number(value)
                        })
                },

                1000
            );

        const result =
            await response.json();

        if (!result.ok) {

            throw new Error(
                result.error ||
                "Parameter error"
            );

        }

        setConnection(true);

        return result;

    } catch (e) {

        setConnection(false);

        if (!silent) {

            showMessage(
                "Error: " + name
            );

        }

        return null;

    }
}
