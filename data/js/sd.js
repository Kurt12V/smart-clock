// ============================================================
// SMARTCLOCK SD
// ============================================================

import { state } from "./state.js";
import { apiFetch } from "./api.js";
import {
    setConnection
} from "./ui.js";

import {
    renderAudioFiles
} from "./audio.js";


// ============================================================
// LOAD SD
// ============================================================

export async function loadSD()
{
    if (state.sdRequest)
    {
        return;
    }


    state.sdRequest = true;


    const list =
        document.getElementById(
            "fileList"
        );


    if (list)
    {
        list.innerHTML =
            '<div class="empty">Loading...</div>';
    }


    try
    {
        const response =
            await apiFetch(
                "/api/sd"
            );


        const data =
            await response.json();


        state.files =
            Array.isArray(data.files)
                ? data.files
                : [];


        renderFiles();

        renderAudioFiles();

        setConnection(true);
    }
    catch (error)
    {
        console.error(
            "[SD] Load failed:",
            error
        );


        if (list)
        {
            list.innerHTML =
                '<div class="empty">SD unavailable</div>';
        }


        setConnection(false);
    }
    finally
    {
        state.sdRequest = false;
    }
}


// ============================================================
// FILTER
// ============================================================

export function filterFiles()
{
    renderFiles();
}


// ============================================================
// RENDER FILES
// ============================================================

export function renderFiles()
{
    const list =
        document.getElementById(
            "fileList"
        );


    if (!list)
    {
        console.warn(
            "[SD] #fileList not found"
        );

        return;
    }


    const input =
        document.getElementById(
            "searchInput"
        );


    const query =
        input
            ? input.value
                .toLowerCase()
                .trim()
            : "";


    const filtered =
        state.files.filter(
            file =>
                String(file.path || "")
                    .toLowerCase()
                    .includes(query)
        );


    if (!filtered.length)
    {
        list.innerHTML =
            '<div class="empty">No files</div>';

        return;
    }


    const fragment =
        document.createDocumentFragment();


    filtered.forEach(
        file =>
        {
            const row =
                document.createElement("div");

            row.className =
                "file";


            const path =
                document.createElement("div");

            path.className =
                "file-path";

            path.textContent =
                file.path;


            const size =
                document.createElement("div");

            size.className =
                "file-size";


            size.textContent =
                file.type === "dir"
                    ? "DIR"
                    : formatSize(
                        Number(
                            file.size || 0
                        )
                    );


            row.appendChild(path);
            row.appendChild(size);

            fragment.appendChild(row);
        }
    );


    list.replaceChildren(
        fragment
    );
}


// ============================================================
// FORMAT SIZE
// ============================================================

function formatSize(bytes)
{
    if (bytes < 1024)
    {
        return bytes + " B";
    }


    if (bytes < 1024 * 1024)
    {
        return (
            bytes / 1024
        ).toFixed(1) +
        " KB";
    }


    return (
        bytes /
        1024 /
        1024
    ).toFixed(1) +
    " MB";
}
