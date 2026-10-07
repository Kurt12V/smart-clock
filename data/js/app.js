import * as params from "./params.js";
import * as sd from "./sd.js";
import * as audio from "./audio.js";
import * as tabs from "./tabs.js";

import * as alarms from "./alarms/alarms.js";
import * as editor from "./alarms/alarm-editor.js";


// ============================================================
// GLOBAL FUNCTIONS FOR INLINE HTML
// ============================================================

// ------------------------------------------------------------
// TABS
// ------------------------------------------------------------

window.openTab =
    tabs.openTab;


// ------------------------------------------------------------
// SETTINGS
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
// AUDIO
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
// ALARMS
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
// ALARM EDITOR
// ------------------------------------------------------------

window.showAlarmEditor =
    editor.showAlarmEditor;

window.toggleAlarmEnabled =
    editor.toggleAlarmEnabled;

window.closeAlarmEditor =
    editor.closeAlarmEditor;


// ============================================================
// INITIALIZATION
// ============================================================

async function init()
{
    console.log(
        "[APP] SmartClock frontend initialization..."
    );


    try
    {
        await params.loadParams();

        params.renderTimezone();


        await Promise.all(
            [
                sd.loadSD(),
                audio.updatePlayerStatus()
            ]
        );


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
