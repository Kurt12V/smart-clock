import { showMessage, setConnection } from "./ui.js";
import { state } from "./state.js";


// ============================================================
// API FETCH
// ============================================================

export async function apiFetch(url, options = {}) {

    try {

        const response = await fetch(url, {
            ...options,
            cache: "no-store"
        });

        setConnection(true);

        if (!response.ok) {

            let message = `HTTP ${response.status}`;

            try {

                const data = await response.json();

                if (data?.error) {
                    message = data.error;
                }

            } catch (_) {
                // Response is not JSON
            }

            throw new Error(message);
        }

        return response;

    } catch (error) {

        setConnection(false);

        console.error(
            "[API]",
            url,
            error
        );

        throw error;
    }
}


// ============================================================
// DEBOUNCE
// ============================================================

export function debounce(
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


// ============================================================
// SET PARAMETER
// ============================================================

export async function setParam(
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
                            name: name,
                            value: Number(value)
                        })
                }
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

        console.log(
            "[API][PARAM]",
            name,
            "=",
            result.value
        );

        return result;

    } catch (error) {

        setConnection(false);

        console.error(
            "[API][PARAM]",
            name,
            error
        );

        if (!silent) {

            showMessage(
                "Error: " +
                name +
                " — " +
                error.message
            );
        }

        return null;
    }
}


// ============================================================
// GET PARAMETER
// ============================================================

export async function getParam(name) {

    try {

        const response =
            await apiFetch(
                "/api/param?name=" +
                encodeURIComponent(name)
            );

        const result =
            await response.json();

        return result;

    } catch (error) {

        console.error(
            "[API][GET PARAM]",
            name,
            error
        );

        return null;
    }
}


// ============================================================
// GET ALL PARAMETERS
// ============================================================

export async function getParams() {

    try {

        const response =
            await apiFetch(
                "/api/params"
            );

        const result =
            await response.json();

        return result;

    } catch (error) {

        console.error(
            "[API][GET PARAMS]",
            error
        );

        return null;
    }
}


// ============================================================
// RESET SETTINGS
// ============================================================

export async function resetSettings() {

    try {

        const response =
            await apiFetch(
                "/api/reset",
                {
                    method: "POST",
                    headers: {
                        "Content-Type":
                            "application/json"
                    },
                    body: "{}"
                }
            );

        const result =
            await response.json();

        if (!result.ok) {

            throw new Error(
                result.error ||
                "Reset failed"
            );
        }

        return result;

    } catch (error) {

        console.error(
            "[API][RESET]",
            error
        );

        showMessage(
            "Reset error: " +
            error.message
        );

        return null;
    }
}
