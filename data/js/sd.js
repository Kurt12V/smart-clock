import { state } from "./state.js";
import { apiFetch } from "./api.js";
import { setConnection } from "./ui.js";
import { renderAudioFiles } from "./audio.js";

async function loadSD() {

    if (state.sdRequest) {
        return;
    }

    state.sdRequest = true;

    const list =
        document.getElementById(
            "fileList"
        );

    list.innerHTML =
        '<div class="empty">Loading...</div>';

    try {

        const response =
            await apiFetch(
                "/api/sd",
                {},
                2500
            );

        const data =
            await response.json();

        state.files =
            Array.isArray(
                data.files
            )
                ? data.files
                : [];

        renderFiles();

        renderAudioFiles();

        setConnection(true);

    } catch (e) {

        list.innerHTML =
            '<div class="empty">SD unavailable</div>';

    } finally {

        state.sdRequest = false;

    }
}

function filterFiles() {

    renderFiles();

}

function renderFiles() {

    const list =
        document.getElementById(
            "fileList"
        );

    const input =
        document.getElementById(
            "searchInput"
        );

    const query =
        input.value
            .toLowerCase()
            .trim();

    const filtered =
        state.files.filter(
            f =>
                String(f.path)
                    .toLowerCase()
                    .includes(query)
        );

    if (!filtered.length) {

        list.innerHTML =
            '<div class="empty">No state.files</div>';

        return;

    }

    const fragment =
        document.createDocumentFragment();

    filtered.forEach(
        f => {

            const row =
                document.createElement(
                    "div"
                );

            row.className =
                "file";

            const path =
                document.createElement(
                    "div"
                );

            path.className =
                "file-path";

            path.textContent =
                f.path;

            const size =
                document.createElement(
                    "div"
                );

            size.className =
                "file-size";

            size.textContent =
                f.type === "dir"
                    ? "DIR"
                    : formatSize(
                        Number(
                            f.size || 0
                        )
                    );

            row.appendChild(
                path
            );

            row.appendChild(
                size
            );

            fragment.appendChild(
                row
            );

        }
    );

    list.replaceChildren(
        fragment
    );
}

function formatSize(bytes) {

    if (bytes < 1024) {

        return bytes + " B";

    }

    if (
        bytes <
        1024 * 1024
    ) {

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
