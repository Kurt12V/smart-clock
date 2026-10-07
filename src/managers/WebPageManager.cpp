#include "WebPageManager.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

WebPageManager::WebPageManager()
{
}


// ============================================================
// ROUTES
// ============================================================

void WebPageManager::setupRoutes(
    WebServer& server
)
{
    server.serveStatic(
        "/css/",
        LittleFS,
        "/css/"
    );

    server.serveStatic(
        "/js/",
        LittleFS,
        "/js/"
    );

    server.serveStatic(
        "/assets/",
        LittleFS,
        "/assets/"
    );

    server.on(
        "/favicon.ico",
        HTTP_GET,
        [&server, this]()
        {
            sendFile(
                server,
                "/favicon.ico"
            );
        }
    );
}


// ============================================================
// ROOT
// ============================================================

void WebPageManager::handleRoot(
    WebServer& server
)
{
    if (!sendFile(
        server,
        "/index.html"
    ))
    {
        server.send(
            404,
            "text/plain",
            "index.html not found"
        );
    }
}


// ============================================================
// NOT FOUND
// ============================================================

bool WebPageManager::handleNotFound(
    WebServer& server
)
{
    const String uri = server.uri();

    if (
        !uri.startsWith("/css/") &&
        !uri.startsWith("/js/") &&
        !uri.startsWith("/assets/") &&
        uri != "/favicon.ico"
    )
    {
        return false;
    }

    if (sendFile(
        server,
        uri
    ))
    {
        return true;
    }

    server.send(
        404,
        "text/plain",
        "Static file not found"
    );

    return true;
}


// ============================================================
// SEND FILE
// ============================================================

bool WebPageManager::sendFile(
    WebServer& server,
    const String& path
)
{
    if (!LittleFS.exists(path))
        return false;

    File file = LittleFS.open(
        path,
        "r"
    );

    if (!file)
        return false;

    const char* contentType =
        getContentType(path);

    server.streamFile(
        file,
        contentType
    );

    file.close();

    return true;
}


// ============================================================
// CONTENT TYPE
// ============================================================

const char* WebPageManager::getContentType(
    const String& path
) const
{
    if (path.endsWith(".html"))
        return "text/html; charset=utf-8";

    if (path.endsWith(".css"))
        return "text/css; charset=utf-8";

    if (path.endsWith(".js"))
        return "application/javascript; charset=utf-8";

    if (path.endsWith(".json"))
        return "application/json; charset=utf-8";

    if (path.endsWith(".svg"))
        return "image/svg+xml";

    if (path.endsWith(".png"))
        return "image/png";

    if (path.endsWith(".jpg") ||
        path.endsWith(".jpeg"))
    {
        return "image/jpeg";
    }

    if (path.endsWith(".ico"))
        return "image/x-icon";

    if (path.endsWith(".woff"))
        return "font/woff";

    if (path.endsWith(".woff2"))
        return "font/woff2";

    return "application/octet-stream";
}