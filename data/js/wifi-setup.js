"use strict";

/* ============================================================
   ELEMENTS
   ============================================================ */

const scanButton = document.getElementById("scanButton");
const scanIcon = document.getElementById("scanIcon");
const scanText = document.getElementById("scanText");

const networkList = document.getElementById("networkList");

const connectionCard = document.getElementById("connectionCard");
const successCard = document.getElementById("successCard");

const wifiForm = document.getElementById("wifiForm");
const ssidInput = document.getElementById("ssid");
const passwordInput = document.getElementById("password");

const showPasswordButton = document.getElementById("showPassword");
const connectButton = document.getElementById("connectButton");
const connectText = document.getElementById("connectText");

const scanError = document.getElementById("scanError");
const formError = document.getElementById("formError");

const statusText = document.getElementById("statusText");
const statusDot = document.getElementById("statusDot");
const countdown = document.getElementById("countdown");

/* ============================================================
   TIME SETTINGS ELEMENTS
   ============================================================ */

const utcOffsetInput = document.getElementById("utcOffset");
const utcDateInput = document.getElementById("utcDate");
const utcTimeInput = document.getElementById("utcTime");
const localTimePreview = document.getElementById("localTimePreview");

/* ============================================================
   STATE
   ============================================================ */

let selectedSSID = "";
let reconnectCountdownTimer = null;

/* ============================================================
   INIT
   ============================================================ */

document.addEventListener("DOMContentLoaded", () => {
    initializeTimeSettings();
    loadStatus();
    scanNetworks();
});

/* ============================================================
   TIME SETTINGS INITIALIZATION
   ============================================================ */

function initializeTimeSettings() {
    /*
     * Используем 24-часовой формат.
     * type="time" хранит значение в формате HH:mm
     * или HH:mm:ss, если задан step="1".
     */

    if (utcTimeInput) {
        utcTimeInput.setAttribute("step", "1");
        utcTimeInput.setAttribute("lang", "ru");
    }

    if (utcDateInput) {
        utcDateInput.setAttribute("lang", "ru");
    }

    /*
     * Заполняем дату и время текущим временем UTC
     * браузера, чтобы пользователь мог скорректировать
     * их при первоначальной настройке.
     *
     * RTC часов в этот момент ещё не изменяется.
     */

    const now = new Date();

    if (utcDateInput && !utcDateInput.value) {
        utcDateInput.value = formatUTCDate(now);
    }

    if (utcTimeInput && !utcTimeInput.value) {
        utcTimeInput.value = formatUTCTime(now);
    }

    if (utcOffsetInput && !utcOffsetInput.value) {
        utcOffsetInput.value = "3";
    }

    updateLocalTimePreview();

    utcOffsetInput?.addEventListener(
        "change",
        updateLocalTimePreview
    );

    utcDateInput?.addEventListener(
        "change",
        updateLocalTimePreview
    );

    utcTimeInput?.addEventListener(
        "input",
        updateLocalTimePreview
    );

    utcTimeInput?.addEventListener(
        "change",
        updateLocalTimePreview
    );
}

/* ============================================================
   UTC DATE / TIME FORMATTING
   ============================================================ */

function pad2(value) {
    return String(value).padStart(2, "0");
}

function formatUTCDate(date) {
    return [
        date.getUTCFullYear(),
        pad2(date.getUTCMonth() + 1),
        pad2(date.getUTCDate())
    ].join("-");
}

function formatUTCTime(date) {
    return [
        pad2(date.getUTCHours()),
        pad2(date.getUTCMinutes()),
        pad2(date.getUTCSeconds())
    ].join(":");
}

/* ============================================================
   LOCAL TIME PREVIEW
   ============================================================ */

function updateLocalTimePreview() {
    if (!localTimePreview) {
        return;
    }

    const dateValue = utcDateInput?.value;
    const timeValue = utcTimeInput?.value;
    const offsetValue = utcOffsetInput?.value;

    if (!dateValue || !timeValue || offsetValue === undefined) {
        localTimePreview.textContent = "";
        return;
    }

    const offset = Number(offsetValue);

    if (!Number.isInteger(offset) || offset < -12 || offset > 14) {
        localTimePreview.textContent = "Некорректное смещение UTC.";
        return;
    }

    /*
     * Создаём дату как UTC.
     * Часовой пояс устройства не используется.
     */

    const utcDateTime = parseUTCDateTime(dateValue, timeValue);

    if (!utcDateTime) {
        localTimePreview.textContent = "Проверьте дату и время UTC.";
        return;
    }

    const localDateTime = new Date(
        utcDateTime.getTime() + offset * 60 * 60 * 1000
    );

    const dateText = [
        pad2(localDateTime.getUTCDate()),
        pad2(localDateTime.getUTCMonth() + 1),
        localDateTime.getUTCFullYear()
    ].join(".");

    const timeText = [
        pad2(localDateTime.getUTCHours()),
        pad2(localDateTime.getUTCMinutes()),
        pad2(localDateTime.getUTCSeconds())
    ].join(":");

    const offsetText =
        offset >= 0
            ? `UTC+${offset}`
            : `UTC${offset}`;

    localTimePreview.textContent =
        `Локальное время (${offsetText}): ${dateText} ${timeText}`;
}

/* ============================================================
   PARSE UTC DATE / TIME
   ============================================================ */

function parseUTCDateTime(dateValue, timeValue) {
    const dateMatch = /^(\d{4})-(\d{2})-(\d{2})$/.exec(dateValue);

    const timeMatch =
        /^(\d{2}):(\d{2})(?::(\d{2}))?$/.exec(timeValue);

    if (!dateMatch || !timeMatch) {
        return null;
    }

    const year = Number(dateMatch[1]);
    const month = Number(dateMatch[2]);
    const day = Number(dateMatch[3]);

    const hour = Number(timeMatch[1]);
    const minute = Number(timeMatch[2]);
    const second = Number(timeMatch[3] || 0);

    if (
        month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59
    ) {
        return null;
    }

    const date = new Date(0);

    date.setUTCFullYear(year, month - 1, day);
    date.setUTCHours(hour, minute, second, 0);

    /*
     * Проверяем, что календарная дата существует.
     * Например, 31 февраля не будет принято.
     */

    if (
        date.getUTCFullYear() !== year ||
        date.getUTCMonth() !== month - 1 ||
        date.getUTCDate() !== day
    ) {
        return null;
    }

    return date;
}

/* ============================================================
   STATUS
   ============================================================ */

async function loadStatus() {
    try {
        const response = await fetch("/api/wifi", {
            cache: "no-store"
        });

        if (!response.ok) {
            throw new Error("HTTP " + response.status);
        }

        const data = await response.json();

        if (data.setupMode) {
            statusText.textContent = "Режим настройки";
            statusDot.style.background = "#d6a84f";
        } else if (data.connected) {
            statusText.textContent = "Подключено";
            statusDot.style.background = "#75b892";
        } else {
            statusText.textContent = "Не подключено";
            statusDot.style.background = "#d6a84f";
        }
    } catch (error) {
        statusText.textContent = "Нет связи";
        statusDot.style.background = "#d66a6a";
    }
}

/* ============================================================
   SCAN
   ============================================================ */

scanButton.addEventListener("click", scanNetworks);

async function scanNetworks() {
    setScanLoading(true);
    hideMessage(scanError);

    try {
        const response = await fetch("/api/wifi/scan", {
            method: "GET",
            cache: "no-store"
        });

        if (!response.ok) {
            throw new Error("HTTP " + response.status);
        }

        const data = await response.json();

        if (!data.ok) {
            throw new Error(
                data.message || "Не удалось выполнить сканирование."
            );
        }

        renderNetworks(data.networks || []);
    } catch (error) {
        showMessage(
            scanError,
            error.message || "Не удалось найти Wi-Fi сети."
        );

        renderEmpty("Ошибка сканирования");
    } finally {
        setScanLoading(false);
    }
}

/* ============================================================
   RENDER NETWORKS
   ============================================================ */

function renderNetworks(networks) {
    if (!networks.length) {
        renderEmpty("Wi-Fi сети не найдены");
        return;
    }

    networkList.innerHTML = "";

    networks.forEach(network => {
        const item = document.createElement("button");

        item.type = "button";
        item.className = "network";

        const signal = getSignalLevel(Number(network.rssi));
        const encrypted = Boolean(network.encrypted);

        item.innerHTML = `
            <div class="network-icon">
                ${signal.icon}
            </div>

            <div class="network-info">
                <div class="network-name">
                    ${escapeHtml(network.ssid || "Без имени")}
                </div>

                <div class="network-meta">
                    ${network.rssi ?? "?"} dBm
                    ${encrypted ? " • Защищена" : " • Открытая"}
                </div>
            </div>

            <div class="network-signal">
                ${signal.text}
            </div>

            ${encrypted ? `<div class="network-lock">●</div>` : ""}
        `;

        item.addEventListener("click", () => {
            selectNetwork(network.ssid, item);
        });

        networkList.appendChild(item);
    });
}

/* ============================================================
   SELECT NETWORK
   ============================================================ */

function selectNetwork(ssid, element) {
    if (!ssid) {
        return;
    }

    selectedSSID = ssid;
    ssidInput.value = ssid;
    passwordInput.value = "";

    document.querySelectorAll(".network").forEach(item => {
        item.classList.remove("selected");
    });

    element.classList.add("selected");

    connectionCard.classList.remove("hidden");

    hideMessage(formError);

    setTimeout(() => {
        passwordInput.focus();

        connectionCard.scrollIntoView({
            behavior: "smooth",
            block: "center"
        });
    }, 50);
}

/* ============================================================
   PASSWORD VISIBILITY
   ============================================================ */

showPasswordButton.addEventListener("click", () => {
    const visible = passwordInput.type === "text";

    passwordInput.type = visible ? "password" : "text";
    showPasswordButton.textContent = visible ? "◉" : "○";
});

/* ============================================================
   CONNECT
   ============================================================ */

wifiForm.addEventListener("submit", connectToWiFi);

async function connectToWiFi(event) {
    event.preventDefault();

    hideMessage(formError);

    const ssid = ssidInput.value.trim();
    const password = passwordInput.value;

    if (!ssid) {
        showMessage(formError, "Выберите Wi-Fi сеть.");
        return;
    }

    /*
     * Получаем настройки времени.
     */

    const dateValue = utcDateInput?.value;
    const timeValue = utcTimeInput?.value;
    const offsetValue = utcOffsetInput?.value;

    if (!dateValue || !timeValue || offsetValue === undefined) {
        showMessage(
            formError,
            "Укажите дату, время UTC и часовой пояс."
        );
        return;
    }

    const utcDateTime = parseUTCDateTime(dateValue, timeValue);

    if (!utcDateTime) {
        showMessage(
            formError,
            "Введите корректную дату и время UTC."
        );
        return;
    }

    const utcOffset = Number(offsetValue);

    if (
        !Number.isInteger(utcOffset) ||
        utcOffset < -12 ||
        utcOffset > 14
    ) {
        showMessage(
            formError,
            "Выберите корректное смещение UTC."
        );
        return;
    }

    setConnectLoading(true);

    /*
     * Данные времени отправляются отдельно по компонентам.
     * Сервер должен записать дату и время именно как UTC.
     * UTC_OFFSET применяется только для локального отображения.
     */

    const payload = {
        ssid,
        password,

        utcOffset,

        utcYear: utcDateTime.getUTCFullYear(),
        utcMonth: utcDateTime.getUTCMonth() + 1,
        utcDay: utcDateTime.getUTCDate(),

        utcHour: utcDateTime.getUTCHours(),
        utcMinute: utcDateTime.getUTCMinutes(),
        utcSecond: utcDateTime.getUTCSeconds()
    };

    try {
        const response = await fetch("/api/wifi/connect", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify(payload)
        });

        const data = await response.json();

        if (!response.ok || !data.ok) {
            throw new Error(
                data.message ||
                "Не удалось сохранить настройки Wi-Fi и времени."
            );
        }

        showSuccess();
    } catch (error) {
        showMessage(
            formError,
            error.message || "Ошибка сохранения настроек."
        );

        setConnectLoading(false);
    }
}

/* ============================================================
   SUCCESS
   ============================================================ */

function showSuccess() {
    connectionCard.classList.add("hidden");
    successCard.classList.remove("hidden");

    successCard.scrollIntoView({
        behavior: "smooth",
        block: "center"
    });

    if (reconnectCountdownTimer !== null) {
        clearInterval(reconnectCountdownTimer);
    }

    let value = 3;

    countdown.textContent = String(value);

    reconnectCountdownTimer = setInterval(() => {
        value--;

        countdown.textContent = String(Math.max(value, 0));

        if (value <= 0) {
            clearInterval(reconnectCountdownTimer);
            reconnectCountdownTimer = null;
        }
    }, 1000);
}

/* ============================================================
   SCAN LOADING
   ============================================================ */

function setScanLoading(loading) {
    scanButton.disabled = loading;

    scanButton.classList.toggle("loading", loading);

    scanText.textContent = loading
        ? "Сканирование..."
        : "Сканировать";
}

/* ============================================================
   CONNECT LOADING
   ============================================================ */

function setConnectLoading(loading) {
    connectButton.disabled = loading;

    connectText.textContent = loading
        ? "Сохранение..."
        : "Подключить SmartClock";
}

/* ============================================================
   EMPTY STATE
   ============================================================ */

function renderEmpty(title) {
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

function getSignalLevel(rssi) {
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

function showMessage(element, message) {
    element.textContent = message;
    element.classList.remove("hidden");
}

function hideMessage(element) {
    element.textContent = "";
    element.classList.add("hidden");
}

/* ============================================================
   HTML ESCAPE
   ============================================================ */

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}