// ============================================================
// SMARTCLOCK TABS
// ============================================================

import {
    loadAlarms
} from "./alarms/alarms.js";


// ============================================================
// OPEN TAB
// ============================================================

export function openTab(tab)
{
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


    if (
        !main ||
        !alarmsTab ||
        !editor ||
        !mainButton ||
        !alarmButton
    )
    {
        console.warn(
            "[TABS] Required elements not found"
        );

        return;
    }


    // --------------------------------------------------------
    // ALARMS
    // --------------------------------------------------------

    if (tab === "alarms")
    {
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

        return;
    }


    // --------------------------------------------------------
    // MAIN
    // --------------------------------------------------------

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