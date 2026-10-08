"use strict";

/* ============================================================
   ELEMENTS
   ============================================================ */

const scanButton = document.getElementById("scanButton");
const scanIcon = document.getElementById("scanIcon");
const scanText = document.getElementById("scanText");

const networkList = document.getElementById("networkList");

const connectionCard =
    document.getElementById("connectionCard");

const successCard =
    document.getElementById("successCard");

const wifiForm =
    document.getElementById("wifiForm");

const ssidInput =
    document.getElementById("ssid");

const passwordInput =
    document.getElementById("password");

const showPasswordButton =
    document.getElementById("showPassword");

const connectButton =
    document.getElementById("connectButton");

const connectText =
    document.getElementById("connectText");

const scanError =
    document.getElementById("scanError");

const formError =
    document.getElementById("formError");

const statusText =
    document.getElementById("statusText");

const statusDot =
    document.getElementById("statusDot");

const countdown =
    document.getElementById("countdown");


/* ============================================================
   STATE
   ============================================================ */

let selectedSSID = "";


/* ============================================================
   INIT
   ============================================================ */

document.addEventListener(
    "DOMContentLoaded",
    () => {

        loadStatus();

        scanNetworks();

    }
);


/* ============================================================
   STATUS
   ============================================================ */

async function loadStatus() {

    try {

        const response =
            await fetch("/api/wifi", {
                cache: "no-store"
            });

        if (!response.ok) {
            throw new Error(
                "HTTP " + response.status
            );
        }

        const data =
            await response.json();

        if (data.setupMode) {

            statusText.textContent =
                "Режим настройки";

            statusDot.style.background =
                "#d6a84f";

        } else if (data.connected) {

            statusText.textContent =
                "Подключено";

            statusDot.style.background =
                "#75b892";

        } else {

            statusText.textContent =
                "Не подключено";

        }

    } catch (error) {

        statusText.textContent =
            "Нет связи";

    }

}


/* ============================================================
   SCAN
   ============================================================ */

scanButton.addEventListener(
    "click",
    scanNetworks
);


async function scanNetworks() {

    setScanLoading(true);

    hideMessage(scanError);

    try {

        const response =
            await fetch(
                "/api/wifi/scan",
                {
                    method: "GET",
                    cache: "no-store"
                }
            );

        if (!response.ok) {

            throw new Error(
                "HTTP " + response.status
            );

        }

        const data =
            await response.json();

        if (!data.ok) {

            throw new Error(
                data.message ||
                "Не удалось выполнить сканирование"
            );

        }

        renderNetworks(
            data.networks || []
        );

    } catch (error) {

        showMessage(
            scanError,
            error.message ||
            "Не удалось найти Wi-Fi сети."
        );

        renderEmpty(
            "Ошибка сканирования"
        );

    } finally {

        setScanLoading(false);

    }

}


/* ============================================================
   RENDER NETWORKS
   ============================================================ */

function renderNetworks(networks) {

    if (!networks.length) {

        renderEmpty(
            "Wi-Fi сети не найдены"
        );

        return;
    }


    networkList.innerHTML = "";


    networks.forEach(
        network => {

            const item =
                document.createElement("button");

            item.type = "button";

            item.className =
                "network";

            const signal =
                getSignalLevel(
                    Number(network.rssi)
                );

            const encrypted =
                Boolean(network.encrypted);

            item.innerHTML = `

                <div class="network-icon">
                    ${signal.icon}
                </div>

                <div class="network-info">

                    <div class="network-name">
                        ${escapeHtml(
                            network.ssid ||
                            "Без имени"
                        )}
                    </div>

                    <div class="network-meta">
                        ${network.rssi ?? "?"} dBm
                        ${encrypted
                            ? " • Защищена"
                            : " • Открытая"}
                    </div>

                </div>

                <div class="network-signal">
                    ${signal.text}
                </div>

                ${
                    encrypted
                        ? `<div class="network-lock">●</div>`
                        : ""
                }

            `;


            item.addEventListener(
                "click",
                () => {

                    selectNetwork(
                        network.ssid,
                        item
                    );

                }
            );


            networkList.appendChild(item);

        }
    );

}


/* ============================================================
   SELECT NETWORK
   ============================================================ */

function selectNetwork(
    ssid,
    element
) {

    if (!ssid) {
        return;
    }

    selectedSSID = ssid;

    ssidInput.value =
        ssid;

    passwordInput.value =
        "";

    document
        .querySelectorAll(".network")
        .forEach(
            item => item.classList.remove(
                "selected"
            )
        );

    element.classList.add(
        "selected"
    );

    connectionCard.classList.remove(
        "hidden"
    );

    hideMessage(formError);

    setTimeout(
        () => {

            passwordInput.focus();

            connectionCard.scrollIntoView({
                behavior: "smooth",
                block: "center"
            });

        },
        50
    );

}


/* ============================================================
   PASSWORD VISIBILITY
   ============================================================ */

showPasswordButton.addEventListener(
    "click",
    () => {

        const visible =
            passwordInput.type === "text";

        passwordInput.type =
            visible
                ? "password"
                : "text";

        showPasswordButton.textContent =
            visible
                ? "◉"
                : "○";

    }
);


/* ============================================================
   CONNECT
   ============================================================ */

wifiForm.addEventListener(
    "submit",
    connectToWiFi
);


async function connectToWiFi(
    event
) {

    event.preventDefault();

    hideMessage(formError);


    const ssid =
        ssidInput.value.trim();

    const password =
        passwordInput.value;


    if (!ssid) {

        showMessage(
            formError,
            "Выберите Wi-Fi сеть."
        );

        return;

    }


    setConnectLoading(true);


    try {

        const response =
            await fetch(
                "/api/wifi/connect",
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({
                        ssid: ssid,
                        password: password
                    })
                }
            );


        const data =
            await response.json();


        if (!response.ok || !data.ok) {

            throw new Error(
                data.message ||
                "Не удалось сохранить настройки Wi-Fi."
            );

        }


        showSuccess();

    } catch (error) {

        showMessage(
            formError,
            error.message ||
            "Ошибка подключения."
        );

        setConnectLoading(false);

    }

}


/* ============================================================
   SUCCESS
   ============================================================ */

function showSuccess() {

    connectionCard.classList.add(
        "hidden"
    );

    successCard.classList.remove(
        "hidden"
    );

    successCard.scrollIntoView({
        behavior: "smooth",
        block: "center"
    });


    let value = 3;

    countdown.textContent =
        value;


    const timer =
        setInterval(
            () => {

                value--;

                countdown.textContent =
                    value;


                if (value <= 0) {

                    clearInterval(timer);

                }

            },
            1000
        );

}


/* ============================================================
   SCAN LOADING
   ============================================================ */

function setScanLoading(
    loading
) {

    scanButton.disabled =
        loading;

    scanButton.classList.toggle(
        "loading",
        loading
    );


    if (loading) {

        scanText.textContent =
            "Сканирование...";

    } else {

        scanText.textContent =
            "Сканировать";

    }

}


/* ============================================================
   CONNECT LOADING
   ============================================================ */

function setConnectLoading(
    loading
) {

    connectButton.disabled =
        loading;


    if (loading) {

        connectText.textContent =
            "Сохранение...";

    } else {

        connectText.textContent =
            "Подключить SmartClock";

    }

}


/* ============================================================
   EMPTY STATE
   ============================================================ */

function renderEmpty(
    title
) {

    networkList.innerHTML = `

        <div class="empty-state">

            <div class="empty-icon">
                Wi-Fi
            </div>

            <div>
                ${escapeHtml(title)}
            </div>

            <small>
                Попробуйте выполнить сканирование ещё раз
            </small>

        </div>

    `;

}


/* ============================================================
   SIGNAL
   ============================================================ */

function getSignalLevel(
    rssi
) {

    if (rssi >= -55) {

        return {
            icon: "▰▰▰",
            text: "Сильный"
        };

    }

    if (rssi >= -70) {

        return {
            icon: "▰▰▱",
            text: "Хороший"
        };

    }

    if (rssi >= -80) {

        return {
            icon: "▰▱▱",
            text: "Слабый"
        };

    }

    return {
        icon: "▱▱▱",
        text: "Очень слабый"
    };

}


/* ============================================================
   MESSAGE
   ============================================================ */

function showMessage(
    element,
    message
) {

    element.textContent =
        message;

    element.classList.remove(
        "hidden"
    );

}


function hideMessage(
    element
) {

    element.textContent = "";

    element.classList.add(
        "hidden"
    );

}


/* ============================================================
   HTML ESCAPE
   ============================================================ */

function escapeHtml(
    value
) {

    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");

}
