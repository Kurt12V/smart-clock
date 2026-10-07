import { state } from "../state.js";
import { minutesToMs, secondsToMs, msToMinutes, msToSeconds } from "./alarm-utils.js";

function renderAlarmPhases() {

    const container =
        document.getElementById(
            "alarmPhases"
        );

    container.replaceChildren();

    state.currentAlarm.phases =
        state.currentAlarm.phases ||
        [];


    state.currentAlarm.phases.forEach(
        (
            phase,
            index
        ) => {

            renderPhase(
                container,
                phase,
                index
            );

        }
    );
}

function renderPhase(
    container,
    phase,
    index
) {

    const element =
        document.createElement(
            "div"
        );

    element.className =
        "phase";


    element.innerHTML = `

        <div class="phase-header">

            <div class="phase-title">
                Phase ${index + 1}
            </div>

            <button
                class="danger"
                onclick="removeAlarmPhase(${index})"
            >
                Remove
            </button>

        </div>


        <div class="phase-grid">


            <div class="phase-field">

                <label>
                    Start offset
                </label>

                <input
                    id="phase_${index}_offset"
                    type="number"
                    step="0.1"
                    value="${msToMinutes(
                        phase.startOffsetMs
                    )}"
                >

                <small>
                    minutes from alarm time
                </small>

            </div>


            <div class="phase-field">

                <label>
                    Duration
                </label>

                <input
                    id="phase_${index}_duration"
                    type="number"
                    step="0.1"
                    min="0"
                    value="${
                        phase.durationMs === 0
                            ? ""
                            : msToMinutes(
                                phase.durationMs
                            )
                    }"
                    placeholder="Until stopped"
                >

                <small>
                    empty = until stopped
                </small>

            </div>


            <div class="phase-field">

                <label>
                    Condition
                </label>

                <select
                    id="phase_${index}_condition"
                >

                    <option value="always">
                        Always
                    </option>

                    <option value="if_not_dismissed">
                        If not dismissed
                    </option>

                    <option value="if_not_snoozed">
                        If not snoozed
                    </option>

                    <option value="if_no_motion">
                        If no motion
                    </option>

                </select>

            </div>

        </div>


        <div class="device-block">

            <div class="device-title">
                Matrix
            </div>

            ${renderMatrixFields(
                phase,
                index
            )}

        </div>


        <div class="device-block">

            <div class="device-title">
                Audio
            </div>

            ${renderAudioFields(
                phase,
                index
            )}

        </div>


        <div class="device-block">

            <div class="device-title">
                COB
            </div>

            ${renderCobFields(
                phase,
                index
            )}

        </div>

    `;


    container.appendChild(
        element
    );


    document.getElementById(
        `phase_${index}_condition`
    ).value =
        phase.condition ||
        "always";


    document.getElementById(
        `matrix_${index}_effect`
    ).value =
        phase.matrix?.effectId ||
        "static";


    document.getElementById(
        `audio_${index}_effect`
    ).value =
        phase.audio?.effectId ||
        "melody";


    document.getElementById(
        `cob_${index}_effect`
    ).value =
        phase.cob?.effectId ||
        "static";
}

function renderMatrixFields(
    phase,
    index
) {

    const m =
        phase.matrix || {};

    return `

        <div class="device-row">

            <label>
                Enabled
            </label>

            <button
                id="matrix_${index}_enabled"
                class="toggle ${
                    m.enabled
                        ? "on"
                        : ""
                }"
                onclick="
                    togglePhaseDevice(
                        ${index},
                        'matrix'
                    )
                "
            >
                ${
                    m.enabled
                        ? "ON"
                        : "OFF"
                }
            </button>

        </div>


        <div class="device-row">

            <label>
                Effect
            </label>

            <select
                id="matrix_${index}_effect"
            >

                <option value="static">
                    Static
                </option>

                <option value="fade">
                    Fade
                </option>

                <option value="pulse">
                    Pulse
                </option>

            </select>

        </div>


        <div class="device-row">

            <label>
                Start
            </label>

            <input
                id="matrix_${index}_start"
                type="number"
                min="0"
                max="100"
                value="${m.start ?? 0}"
            >

        </div>


        <div class="device-row">

            <label>
                End
            </label>

            <input
                id="matrix_${index}_end"
                type="number"
                min="0"
                max="100"
                value="${m.end ?? 100}"
            >

        </div>


        <div class="device-row">

            <label>
                Speed
            </label>

            <input
                id="matrix_${index}_speed"
                type="number"
                min="0"
                step="0.1"
                value="${
                    msToSeconds(
                        m.speedMs
                    )
                }"
            >

        </div>

    `;
}

function renderAudioFields(
    phase,
    index
) {

    const a =
        phase.audio || {};

    return `

        <div class="device-row">

            <label>
                Enabled
            </label>

            <button
                id="audio_${index}_enabled"
                class="toggle ${
                    a.enabled
                        ? "on"
                        : ""
                }"
                onclick="
                    togglePhaseDevice(
                        ${index},
                        'audio'
                    )
                "
            >
                ${
                    a.enabled
                        ? "ON"
                        : "OFF"
                }
            </button>

        </div>


        <div class="device-row">

            <label>
                Effect
            </label>

            <select
                id="audio_${index}_effect"
            >

                <option value="melody">
                    Melody
                </option>

            </select>

        </div>


        <div class="device-row">

            <label>
                Start
            </label>

            <input
                id="audio_${index}_start"
                type="number"
                min="0"
                max="100"
                value="${a.start ?? 0}"
            >

        </div>


        <div class="device-row">

            <label>
                End
            </label>

            <input
                id="audio_${index}_end"
                type="number"
                min="0"
                max="100"
                value="${a.end ?? 100}"
            >

        </div>


        <div class="device-row">

            <label>
                Loop
            </label>

            <button
                id="audio_${index}_loop"
                class="toggle ${
                    a.loop
                        ? "on"
                        : ""
                }"
                onclick="
                    togglePhaseLoop(
                        ${index}
                    )
                "
            >
                ${
                    a.loop
                        ? "ON"
                        : "OFF"
                }
            </button>

        </div>

    `;
}

function renderCobFields(
    phase,
    index
) {

    const c =
        phase.cob || {};

    return `

        <div class="device-row">

            <label>
                Enabled
            </label>

            <button
                id="cob_${index}_enabled"
                class="toggle ${
                    c.enabled
                        ? "on"
                        : ""
                }"
                onclick="
                    togglePhaseDevice(
                        ${index},
                        'cob'
                    )
                "
            >
                ${
                    c.enabled
                        ? "ON"
                        : "OFF"
                }
            </button>

        </div>


        <div class="device-row">

            <label>
                Effect
            </label>

            <select
                id="cob_${index}_effect"
            >

                <option value="static">
                    Static
                </option>

                <option value="blink">
                    Blink
                </option>

            </select>

        </div>


        <div class="device-row">

            <label>
                Start
            </label>

            <input
                id="cob_${index}_start"
                type="number"
                min="0"
                max="100"
                value="${c.start ?? 0}"
            >

        </div>


        <div class="device-row">

            <label>
                End
            </label>

            <input
                id="cob_${index}_end"
                type="number"
                min="0"
                max="100"
                value="${c.end ?? 100}"
            >

        </div>


        <div class="device-row">

            <label>
                Max duration
            </label>

            <input
                id="cob_${index}_maxDuration"
                type="number"
                min="0"
                step="0.1"
                value="${
                    c.maxDurationMs
                        ? msToMinutes(
                            c.maxDurationMs
                        )
                        : ""
                }"
                placeholder="Unlimited"
            >

        </div>

    `;
}

function togglePhaseDevice(
    index,
    device
) {

    const phase =
        state.currentAlarm.phases[index];

    phase[device] =
        phase[device] || {};

    phase[device].enabled =
        !phase[device].enabled;

    renderAlarmPhases();
}

function togglePhaseLoop(
    index
) {

    const phase =
        state.currentAlarm.phases[index];

    phase.audio =
        phase.audio || {};

    phase.audio.loop =
        !phase.audio.loop;

    renderAlarmPhases();
}

function addAlarmPhase() {

    state.currentAlarm.phases.push({

        startOffsetMs:
            0,

        durationMs:
            30000,

        condition:
            "if_not_dismissed",

        matrix: {

            enabled:
                false,

            effectId:
                "static",

            start:
                0,

            end:
                100,

            speedMs:
                0,

            durationMs:
                30000

        },

        audio: {

            enabled:
                false,

            effectId:
                "melody",

            start:
                0,

            end:
                100,

            speedMs:
                0,

            durationMs:
                30000,

            loop:
                false

        },

        cob: {

            enabled:
                false,

            effectId:
                "static",

            start:
                0,

            end:
                100,

            speedMs:
                0,

            durationMs:
                30000,

            maxDurationMs:
                0

        }

    });

    renderAlarmPhases();
}

function removeAlarmPhase(
    index
) {

    state.currentAlarm.phases.splice(
        index,
        1
    );

    renderAlarmPhases();
}

function readPhase(
    phase,
    index
) {

    phase.startOffsetMs =
        minutesToMs(
            document.getElementById(
                `phase_${index}_offset`
            ).value
        );


    const duration =
        document.getElementById(
            `phase_${index}_duration`
        ).value;


    phase.durationMs =
        duration === ""
            ? 0
            : minutesToMs(
                duration
            );


    phase.condition =
        document.getElementById(
            `phase_${index}_condition`
        ).value;


    /* MATRIX */

    phase.matrix =
        phase.matrix || {};

    phase.matrix.enabled =
        document.getElementById(
            `matrix_${index}_enabled`
        ).classList.contains(
            "on"
        );

    phase.matrix.effectId =
        document.getElementById(
            `matrix_${index}_effect`
        ).value;

    phase.matrix.start =
        Number(
            document.getElementById(
                `matrix_${index}_start`
            ).value
        );

    phase.matrix.end =
        Number(
            document.getElementById(
                `matrix_${index}_end`
            ).value
        );

    phase.matrix.speedMs =
        secondsToMs(
            document.getElementById(
                `matrix_${index}_speed`
            ).value
        );

    phase.matrix.durationMs =
        phase.durationMs;


    /* AUDIO */

    phase.audio =
        phase.audio || {};

    phase.audio.enabled =
        document.getElementById(
            `audio_${index}_enabled`
        ).classList.contains(
            "on"
        );

    phase.audio.effectId =
        document.getElementById(
            `audio_${index}_effect`
        ).value;

    phase.audio.start =
        Number(
            document.getElementById(
                `audio_${index}_start`
            ).value
        );

    phase.audio.end =
        Number(
            document.getElementById(
                `audio_${index}_end`
            ).value
        );

    phase.audio.loop =
        document.getElementById(
            `audio_${index}_loop`
        ).classList.contains(
            "on"
        );

    phase.audio.speedMs =
        0;

    phase.audio.durationMs =
        phase.durationMs;


    /* COB */

    phase.cob =
        phase.cob || {};

    phase.cob.enabled =
        document.getElementById(
            `cob_${index}_enabled`
        ).classList.contains(
            "on"
        );

    phase.cob.effectId =
        document.getElementById(
            `cob_${index}_effect`
        ).value;

    phase.cob.start =
        Number(
            document.getElementById(
                `cob_${index}_start`
            ).value
        );

    phase.cob.end =
        Number(
            document.getElementById(
                `cob_${index}_end`
            ).value
        );


    const maxDuration =
        document.getElementById(
            `cob_${index}_maxDuration`
        ).value;


    phase.cob.maxDurationMs =
        maxDuration === ""
            ? 0
            : minutesToMs(
                maxDuration
            );

}
