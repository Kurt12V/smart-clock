import { state } from "./state.js";
import { apiFetch } from "./api.js";
import { setConnection, showMessage } from "./ui.js";

function renderAudioFiles() {

    const list =
        document.getElementById(
            "audioList"
        );

    const audioFiles =
        state.files.filter(
            f => {

                if (
                    f.type === "dir"
                ) {

                    return false;

                }

                const path =
                    String(
                        f.path
                    ).toLowerCase();

                return (
                    path.endsWith(
                        ".wav"
                    ) ||
                    path.endsWith(
                        ".mp3"
                    )
                );

            }
        );

    if (!audioFiles.length) {

        list.innerHTML =
            '<div class="empty">No sounds</div>';

        return;

    }

    const fragment =
        document.createDocumentFragment();

    audioFiles.forEach(
        f => {

            const row =
                document.createElement(
                    "div"
                );

            row.className =
                "audio-row";

            const name =
                document.createElement(
                    "div"
                );

            name.className =
                "audio-name";

            name.textContent =
                f.path;

            const button =
                document.createElement(
                    "button"
                );

            button.className =
                "audio-play";

            button.textContent =
                "▶";

            button.onclick =
                () =>
                    playSound(
                        f.path
                    );

            row.appendChild(
                name
            );

            row.appendChild(
                button
            );

            fragment.appendChild(
                row
            );

        }
    );

    list.replaceChildren(
        fragment
    );
}

async function playSound(
    path,
    stream = 0
) {

    state.selectedSound =
        path;

    try {

        await apiFetch(
            "/api/audio/play",
            {

                method: "POST",

                headers: {
                    "Content-Type":
                        "application/json"
                },

                body:
                    JSON.stringify({

                        path: path,

                        stream: stream,

                        localPercent: 100,

                        fadeInMs: 0,

                        fadeOutMs: 0,

                        curve: 0

                    })

            },

            1200
        );

        setConnection(true);

        await updatePlayerStatus();

        showMessage(
            "Playing"
        );

    } catch (e) {

        showMessage(
            "Playback error"
        );

    }
}

async function playSelectedSound() {

    if (state.selectedSound) {

        await playSound(
            state.selectedSound
        );

        return;

    }

    const wav =
        state.files.find(
            f =>
                f.type !== "dir" &&
                String(
                    f.path
                )
                    .toLowerCase()
                    .endsWith(".wav")
        );

    if (!wav) {

        showMessage(
            "No WAV state.files"
        );

        return;

    }

    await playSound(
        wav.path
    );
}

async function pauseSound() {

    try {

        await apiFetch(
            "/api/audio/pause",
            {
                method: "POST"
            },
            1000
        );

        await updatePlayerStatus();

        showMessage(
            "Paused"
        );

    } catch (e) {

        showMessage(
            "Pause error"
        );

    }
}

async function resumeSound() {

    try {

        await apiFetch(
            "/api/audio/resume",
            {
                method: "POST"
            },
            1000
        );

        await updatePlayerStatus();

        showMessage(
            "Resumed"
        );

    } catch (e) {

        showMessage(
            "Resume error"
        );

    }
}

async function stopSound() {

    try {

        await apiFetch(
            "/api/audio/stop",
            {

                method: "POST",

                headers: {
                    "Content-Type":
                        "application/json"
                },

                body:
                    JSON.stringify({
                        fadeOutMs: 0
                    })

            },
            1000
        );

        await updatePlayerStatus();

        showMessage(
            "Playback stopped"
        );

    } catch (e) {

        showMessage(
            "Stop error"
        );

    }
}

function formatTime(ms) {

    const total =
        Math.floor(
            Number(ms || 0) /
            1000
        );

    const minutes =
        Math.floor(
            total / 60
        );

    const seconds =
        total %
        60;

    return (
        minutes +
        ":" +
        String(seconds)
            .padStart(2, "0")
    );
}

async function updatePlayerStatus() {

    if (state.playerRequest) {
        return;
    }

    state.playerRequest = true;

    try {

        const response =
            await apiFetch(
                "/api/audio/status",
                {},
                700
            );

        const s =
            await response.json();


        document.getElementById(
            "playerTrack"
        ).textContent =
            s.path &&
            s.path.length
                ? s.path
                : "—";


        document.getElementById(
            "playerTime"
        ).textContent =
            formatTime(
                s.positionMs
            ) +
            " / " +
            formatTime(
                s.durationMs
            );


        const state =
            s.state ||
            "stopped";

        const stateEl =
            document.getElementById(
                "playerState"
            );

        stateEl.textContent =
            state;

        stateEl.className =
            "player-state " +
            state;

    } catch (e) {

        /* silent */

    } finally {

        state.playerRequest = false;

    }
}
