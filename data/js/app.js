// ============================================================
// SMARTCLOCK APP
// ============================================================

import * as params from "./params.js";
import * as sd from "./sd.js";
import * as audio from "./audio.js";
import * as tabs from "./tabs.js";

import * as alarms from "./alarms/alarms.js";
import * as editor from "./alarms/alarm-editor.js";
import * as phases from "./alarms/alarm-phases.js";


// ============================================================
// GLOBAL HTML HANDLERS
//
// The HTML currently uses onclick/oninput attributes.
// ES modules do not expose their functions to window
// automatically, so we explicitly export them here.
// ============================================================


// ------------------------------------------------------------
// Tabs
// ------------------------------------------------------------

window.openTab =
    tabs.openTab;


// ------------------------------------------------------------
// Settings
// ------------------------------------------------------------

window.changeTimezone =
    params.changeTimezone;

window.changeBrightness =
    params.changeBrightness;

window.toggleMatrix =
    params.toggleMatrix;

window.changeMatrixBrightness =
    params.changeMatrixBrightness;

window.changeMatrixEffect =
    params.changeMatrixEffect;

window.changeMatrixSpeed =
    params.changeMatrixSpeed;

window.toggleCob =
    params.toggleCob;

window.changeCobBrightness =
    params.changeCobBrightness;

window.changeCobEffect =
    params.changeCobEffect;

window.changeCobSpeed =
    params.changeCobSpeed;

window.changeVolume =
    params.changeVolume;

window.toggleMic =
    params.toggleMic;


// ------------------------------------------------------------
// SD
// ------------------------------------------------------------

window.loadSD =
    sd.loadSD;

window.filterFiles =
    sd.filterFiles;


// ------------------------------------------------------------
// Audio
// ------------------------------------------------------------

window.playSound =
    audio.playSound;

window.playSelectedSound =
    audio.playSelectedSound;

window.pauseSound =
    audio.pauseSound;

window.resumeSound =
    audio.resumeSound;

window.stopSound =
    audio.stopSound;

window.updatePlayerStatus =
    audio.updatePlayerStatus;


// ------------------------------------------------------------
// Alarms
// ------------------------------------------------------------

window.loadAlarms =
    alarms.loadAlarms;

window.createAlarm =
    alarms.createAlarm;

window.openAlarmEditor =
    alarms.openAlarmEditor;

window.saveCurrentAlarm =
    alarms.saveCurrentAlarm;

window.setAlarmEnabled =
    alarms.setAlarmEnabled;

window.deleteCurrentAlarm =
    alarms.deleteCurrentAlarm;


// ------------------------------------------------------------
// Alarm editor
// ------------------------------------------------------------

window.showAlarmEditor =
    editor.showAlarmEditor;

window.toggleAlarmEnabled =
    editor.toggleAlarmEnabled;

window.closeAlarmEditor =
    editor.closeAlarmEditor;


// ------------------------------------------------------------
// Alarm phases
// ------------------------------------------------------------

window.renderAlarmPhases =
    phases.renderAlarmPhases;

window.togglePhaseDevice =
    phases.togglePhaseDevice;

window.togglePhaseLoop =
    phases.togglePhaseLoop;

window.addAlarmPhase =
    phases.addAlarmPhase;

window.removeAlarmPhase =
    phases.removeAlarmPhase;


// ============================================================
// APPLICATION INITIALIZATION
// ============================================================

async function init()
{
    console.log(
        "[APP] SmartClock frontend initialization..."
    );

    try
    {
        // ----------------------------------------------------
        // Settings
        // ----------------------------------------------------

        await params.loadParams();

        params.renderTimezone();


        // ----------------------------------------------------
        // SD + Audio
        // ----------------------------------------------------

        await Promise.all([
            sd.loadSD(),
            audio.updatePlayerStatus()
        ]);


        // ----------------------------------------------------
        // Alarms
        // ----------------------------------------------------

        await alarms.loadAlarms();


        console.log(
            "[APP] SmartClock frontend READY"
        );
    }
    catch (error)
    {
        console.error(
            "[APP] Initialization failed:",
            error
        );
    }
}


// ============================================================
// AUDIO STATUS POLLING
// ============================================================

setInterval(
    () =>
    {
        audio.updatePlayerStatus();
    },
    1000
);


// ============================================================
// START
// ============================================================

init();