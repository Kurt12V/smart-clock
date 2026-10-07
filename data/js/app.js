import { loadParams, renderTimezone } from "./params.js";
import * as params from "./params.js";
import * as sd from "./sd.js";
import * as audio from "./audio.js";
import * as tabs from "./tabs.js";
import * as alarms from "./alarms/alarms.js";
import * as editor from "./alarms/alarm-editor.js";
import * as phases from "./alarms/alarm-phases.js";

window.openTab = tabs.openTab;
window.changeTimezone = params.changeTimezone;
window.changeBrightness = params.changeBrightness;
window.toggleMatrix = params.toggleMatrix;
window.changeMatrixBrightness = params.changeMatrixBrightness;
window.changeMatrixEffect = params.changeMatrixEffect;
window.changeMatrixSpeed = params.changeMatrixSpeed;
window.toggleCob = params.toggleCob;
window.changeCobBrightness = params.changeCobBrightness;
window.changeCobEffect = params.changeCobEffect;
window.changeCobSpeed = params.changeCobSpeed;
window.changeVolume = params.changeVolume;
window.toggleMic = params.toggleMic;
window.loadSD = sd.loadSD;
window.filterFiles = sd.filterFiles;
window.playSound = audio.playSound;
window.playSelectedSound = audio.playSelectedSound;
window.pauseSound = audio.pauseSound;
window.resumeSound = audio.resumeSound;
window.stopSound = audio.stopSound;
window.updatePlayerStatus = audio.updatePlayerStatus;
window.loadAlarms = alarms.loadAlarms;
window.createAlarm = alarms.createAlarm;
window.openAlarmEditor = alarms.openAlarmEditor;
window.showAlarmEditor = editor.showAlarmEditor;
window.toggleAlarmEnabled = editor.toggleAlarmEnabled;
window.renderAlarmPhases = phases.renderAlarmPhases;
window.togglePhaseDevice = phases.togglePhaseDevice;
window.togglePhaseLoop = phases.togglePhaseLoop;
window.addAlarmPhase = phases.addAlarmPhase;
window.removeAlarmPhase = phases.removeAlarmPhase;
window.saveCurrentAlarm = alarms.saveCurrentAlarm;
window.setAlarmEnabled = alarms.setAlarmEnabled;
window.deleteCurrentAlarm = alarms.deleteCurrentAlarm;
window.closeAlarmEditor = editor.closeAlarmEditor;

async function init() {
    params.renderTimezone();
    await params.loadParams();
    await Promise.all([
        sd.loadSD(),
        audio.updatePlayerStatus()
    ]);
}

setInterval(audio.updatePlayerStatus, 1000);

init();
