import { loadAlarms } from "./alarms/alarms.js";

function openTab(tab) {

    const main =
        document.getElementById(
            "mainTab"
        );

    const alarmsTab =
        document.getElementById(
            "alarmsTab"
        );

    const editor =
        document.getElementById(
            "alarmEditor"
        );

    const mainButton =
        document.getElementById(
            "tabMain"
        );

    const alarmButton =
        document.getElementById(
            "tabAlarms"
        );


    if (tab === "state.alarms") {

        main.style.display =
            "none";

        alarmsTab.style.display =
            "block";

        editor.style.display =
            "none";

        mainButton.classList.remove(
            "active"
        );

        alarmButton.classList.add(
            "active"
        );

        loadAlarms();

    } else {

        main.style.display =
            "block";

        alarmsTab.style.display =
            "none";

        editor.style.display =
            "none";

        mainButton.classList.add(
            "active"
        );

        alarmButton.classList.remove(
            "active"
        );

    }
}
