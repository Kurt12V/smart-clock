// ============================================================
// SMARTCLOCK AUDIO
// ============================================================

import { state } from "./state.js";
import { apiFetch } from "./api.js";
import { setConnection, showMessage } from "./ui.js";


// ============================================================
// RENDER AUDIO FILES
// ============================================================

export function renderAudioFiles()
{
    const list =
        document.getElementById("audioList");

    if (!list)
    {
        console.warn(
            "[AUDIO] #audioList not found"
        );

        return;
    }

    const audioFiles =
        state.files.filter(
            file =>
            {
                if (file.type === "dir")
                {
                    return false;
                }

                const path =
                    String(file.path || "")
                        .toLowerCase();

                return (
                    path.endsWith(".wav") ||
                    path.endsWith(".mp3")
                );
            }
        );


    if (!audioFiles.length)
    {
        list.innerHTML =
            '<div class="empty">No sounds</div>';

        return;
    }


    const fragment =
        document.createDocumentFragment();


    audioFiles.forEach(
        file =>
        {
            const row =
                document.createElement("div");

            row.className =
                "audio-row";


            const name =
                document.createElement("div");

            name.className =
                "audio-name";

            name.textContent =
                file.path;


            const button =
                document.createElement("button");

            button.className =
                "audio-play";

            button.textContent =
                "▶";


            button.addEventListener(
                "click",
                () =>
                {
                    playSound(file.path);
                }
            );


            row.appendChild(name);
            row.appendChild(button);

            fragment.appendChild(row);
        }
    );


    list.replaceChildren(fragment);
}


// ============================================================
// PLAY
// ============================================================

export async function playSound(
    path,
    stream = 0
)
{
    if (!path)
    {
        return;
    }

    state.selectedSound =
        path;


    try
    {
        await apiFetch(
            "/api/audio/play",
            {
                method: "POST",

                headers:
                {
                    "Content-Type":
                        "application/json"
                },

                body:
                    JSON.stringify(
                        {
                            path: path,
                            stream: stream,
                            localPercent: 100,
                            fadeInMs: 0,
                            fadeOutMs: 0,
                            curve: 0
                        }
                    )
            }
        );


        setConnection(true);

        await updatePlayerStatus();

        showMessage("Playing");
    }
    catch (error)
    {
        console.error(
            "[AUDIO][PLAY]",
            error
        );

        showMessage(
            "Playback error"
        );
    }
}


// ============================================================
// PLAY SELECTED
// ============================================================

export async function playSelectedSound()
{
    if (state.selectedSound)
    {
        await playSound(
            state.selectedSound
        );

        return;
    }


    const wav =
        state.files.find(
            file =>
                file.type !== "dir" &&
                String(file.path || "")
                    .toLowerCase()
                    .endsWith(".wav")
        );


    if (!wav)
    {
        showMessage(
            "No WAV files"
        );

        return;
    }


    await playSound(
        wav.path
    );
}


// ============================================================
// PAUSE
// ============================================================

export async function pauseSound()
{
    try
    {
        await apiFetch(
            "/api/audio/pause",
            {
                method: "POST"
            }
        );


        await updatePlayerStatus();

        showMessage("Paused");
    }
    catch (error)
    {
        console.error(
            "[AUDIO][PAUSE]",
            error
        );

        showMessage(
            "Pause error"
        );
    }
}


// ============================================================
// RESUME
// ============================================================

export async function resumeSound()
{
    try
    {
        await apiFetch(
            "/api/audio/resume",
            {
                method: "POST"
            }
        );


        await updatePlayerStatus();

        showMessage("Resumed");
    }
    catch (error)
    {
        console.error(
            "[AUDIO][RESUME]",
            error
        );

        showMessage(
            "Resume error"
        );
    }
}


// ============================================================
// STOP
// ============================================================

export async function stopSound()
{
    try
    {
        await apiFetch(
            "/api/audio/stop",
            {
                method: "POST",

                headers:
                {
                    "Content-Type":
                        "application/json"
                },

                body:
                    JSON.stringify(
                        {
                            fadeOutMs: 0
                        }
                    )
            }
        );


        await updatePlayerStatus();

        showMessage(
            "Playback stopped"
        );
    }
    catch (error)
    {
        console.error(
            "[AUDIO][STOP]",
            error
        );

        showMessage(
            "Stop error"
        );
    }
}


// ============================================================
// FORMAT TIME
// ============================================================

function formatTime(ms)
{
    const total =
        Math.floor(
            Number(ms || 0) / 1000
        );


    const minutes =
        Math.floor(total / 60);


    const seconds =
        total % 60;


    return (
        minutes +
        ":" +
        String(seconds)
            .padStart(2, "0")
    );
}


// ============================================================
// UPDATE PLAYER STATUS
// ============================================================

export async function updatePlayerStatus()
{
    if (state.playerRequest)
    {
        return;
    }


    state.playerRequest = true;


    try
    {
        const response =
            await apiFetch(
                "/api/audio/status"
            );


        const data =
            await response.json();


        // ----------------------------------------------------
        // TRACK
        // ----------------------------------------------------

        const track =
            document.getElementById(
                "playerTrack"
            );

        if (track)
        {
            track.textContent =
                data.path &&
                data.path.length
                    ? data.path
                    : "—";
        }


        // ----------------------------------------------------
        // TIME
        // ----------------------------------------------------

        const time =
            document.getElementById(
                "playerTime"
            );

        if (time)
        {
            time.textContent =
                formatTime(
                    data.positionMs
                ) +
                " / " +
                formatTime(
                    data.durationMs
                );
        }


        // ----------------------------------------------------
        // PLAYER STATE
        // ----------------------------------------------------

        const playerState =
            data.state ||
            "stopped";


        const stateElement =
            document.getElementById(
                "playerState"
            );


        if (stateElement)
        {
            stateElement.textContent =
                playerState;

            stateElement.className =
                "player-state " +
                playerState;
        }


        setConnection(true);
    }
    catch (error)
    {
        // Audio polling should not spam
        // the user with errors every second.

        console.debug(
            "[AUDIO][STATUS]",
            error.message
        );
    }
    finally
    {
        state.playerRequest =
            false;
    }
}
