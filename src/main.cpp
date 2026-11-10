
#include <Arduino.h>

#include "./managers/Setting/SettingsManager.h"
#include "./managers/Led/LightingManager.h"
#include "./Config.h"

// ============================================================
// MANAGERS
// ============================================================

SettingsManager settings;
LightingManager lighting(settings);

// ============================================================
// SERIAL
// ============================================================

constexpr uint32_t SERIAL_BAUDRATE = 115200;

char serialBuffer[128];

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

void printMenu();
void printStatus();

void processCommand(char* command);

void testCOB(char** args);
void testMatrix(char** args);
void testAlarm(char** args);

void setParameter(
    SettingsManager::Param param,
    int value
);

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial0.begin(SERIAL_BAUDRATE);

    delay(500);

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println("       SMARTCLOCK LIGHTING TEST");
    Serial0.println("========================================");

    // --------------------------------------------------------
    // SETTINGS
    // --------------------------------------------------------

    Serial0.println("[Main] Initializing SettingsManager...");

    if (!settings.begin())
    {
        Serial0.println("[Main] ERROR: SettingsManager failed!");

        while (true)
            delay(1000);
    }

    Serial0.println("[Main] SettingsManager initialized");

    // --------------------------------------------------------
    // LIGHTING
    // --------------------------------------------------------

    Serial0.println("[Main] Initializing LightingManager...");

    if (!lighting.begin())
    {
        Serial0.println("[Main] ERROR: LightingManager failed!");

        while (true)
            delay(1000);
    }

    Serial0.println("[Main] LightingManager initialized");

    // --------------------------------------------------------
    // MENU
    // --------------------------------------------------------

    printMenu();

    Serial0.println("[Main] System ready");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    settings.update();

    lighting.update();

    // --------------------------------------------------------
    // SERIAL COMMANDS
    // --------------------------------------------------------

    while (Serial0.available())
    {
        static uint8_t index = 0;

        const char c = Serial0.read();

        if (c == '\n' || c == '\r')
        {
            if (index == 0)
                continue;

            serialBuffer[index] = '\0';

            processCommand(serialBuffer);

            index = 0;
        }
        else if (index < sizeof(serialBuffer) - 1)
        {
            serialBuffer[index++] = c;
        }
    }
}

// ============================================================
// MENU
// ============================================================

void printMenu()
{
    Serial0.println();
    Serial0.println("=============== COMMANDS ===============");

    Serial0.println();
    Serial0.println("GENERAL:");

    Serial0.println("help");
    Serial0.println("status");

    Serial0.println();
    Serial0.println("COB:");

    Serial0.println("cob on");
    Serial0.println("cob off");
    Serial0.println("cob br <0-255>");
    Serial0.println("cob eff <number>");
    Serial0.println("cob spd <0-255>");

    Serial0.println();
    Serial0.println("MATRIX:");

    Serial0.println("matrix on");
    Serial0.println("matrix off");
    Serial0.println("matrix br <0-255>");
    Serial0.println("matrix eff <number>");
    Serial0.println("matrix spd <0-255>");

    Serial0.println();
    Serial0.println("ALARM:");

    Serial0.println("alarm start");
    Serial0.println("alarm matrix <brightness> <R> <G> <B>");
    Serial0.println("alarm cob <brightness>");
    Serial0.println("alarm stop");

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println();
}

// ============================================================
// COMMAND PROCESSING
// ============================================================

void processCommand(char* command)
{
    char* args[6] = {};

    uint8_t count = 0;

    char* token = strtok(command, " ");

    while (token != nullptr && count < 6)
    {
        args[count++] = token;

        token = strtok(nullptr, " ");
    }

    if (count == 0)
        return;

    Serial0.println();
    Serial0.print("[Main] Command: ");
    Serial0.println(args[0]);

    // --------------------------------------------------------
    // HELP
    // --------------------------------------------------------

    if (strcmp(args[0], "help") == 0)
    {
        printMenu();
        return;
    }

    // --------------------------------------------------------
    // STATUS
    // --------------------------------------------------------

    if (strcmp(args[0], "status") == 0)
    {
        printStatus();
        return;
    }

    // --------------------------------------------------------
    // COB
    // --------------------------------------------------------

    if (strcmp(args[0], "cob") == 0)
    {
        testCOB(args);
        return;
    }

    // --------------------------------------------------------
    // MATRIX
    // --------------------------------------------------------

    if (strcmp(args[0], "matrix") == 0)
    {
        testMatrix(args);
        return;
    }

    // --------------------------------------------------------
    // ALARM
    // --------------------------------------------------------

    if (strcmp(args[0], "alarm") == 0)
    {
        testAlarm(args);
        return;
    }

    Serial0.println("[Main] Unknown command");
}

// ============================================================
// COB TEST
// ============================================================

void testCOB(char** args)
{
    if (args[1] == nullptr)
        return;

    // ON

    if (strcmp(args[1], "on") == 0)
    {
        setParameter(
            SettingsManager::Param::COB_ENABLED,
            1
        );

        Serial0.println("[Test] COB ON");
        return;
    }

    // OFF

    if (strcmp(args[1], "off") == 0)
    {
        setParameter(
            SettingsManager::Param::COB_ENABLED,
            0
        );

        Serial0.println("[Test] COB OFF");
        return;
    }

    if (args[2] == nullptr)
        return;

    const int value = atoi(args[2]);

    // BRIGHTNESS

    if (strcmp(args[1], "br") == 0)
    {
        setParameter(
            SettingsManager::Param::COB_BRIGHTNESS,
            value
        );

        Serial0.print("[Test] COB brightness: ");
        Serial0.println(value);

        return;
    }

    // EFFECT

    if (strcmp(args[1], "eff") == 0)
    {
        setParameter(
            SettingsManager::Param::COB_EFFECT,
            value
        );

        Serial0.print("[Test] COB effect: ");
        Serial0.println(value);

        return;
    }

    // SPEED

    if (strcmp(args[1], "spd") == 0)
    {
        setParameter(
            SettingsManager::Param::COB_SPEED,
            value
        );

        Serial0.print("[Test] COB speed: ");
        Serial0.println(value);

        return;
    }

    Serial0.println("[Test] Unknown COB command");
}

// ============================================================
// MATRIX TEST
// ============================================================

void testMatrix(char** args)
{
    if (args[1] == nullptr)
        return;

    // ON

    if (strcmp(args[1], "on") == 0)
    {
        setParameter(
            SettingsManager::Param::MATRIX_ENABLED,
            1
        );

        Serial0.println("[Test] MATRIX ON");
        return;
    }

    // OFF

    if (strcmp(args[1], "off") == 0)
    {
        setParameter(
            SettingsManager::Param::MATRIX_ENABLED,
            0
        );

        Serial0.println("[Test] MATRIX OFF");
        return;
    }

    if (args[2] == nullptr)
        return;

    const int value = atoi(args[2]);

    // BRIGHTNESS

    if (strcmp(args[1], "br") == 0)
    {
        setParameter(
            SettingsManager::Param::MATRIX_BRIGHTNESS,
            value
        );

        Serial0.print("[Test] Matrix brightness: ");
        Serial0.println(value);

        return;
    }

    // EFFECT

    if (strcmp(args[1], "eff") == 0)
    {
        setParameter(
            SettingsManager::Param::MATRIX_EFFECT,
            value
        );

        Serial0.print("[Test] Matrix effect: ");
        Serial0.println(value);

        return;
    }

    // SPEED

    if (strcmp(args[1], "spd") == 0)
    {
        setParameter(
            SettingsManager::Param::MATRIX_SPEED,
            value
        );

        Serial0.print("[Test] Matrix speed: ");
        Serial0.println(value);

        return;
    }

    Serial0.println("[Test] Unknown MATRIX command");
}

// ============================================================
// ALARM TEST
// ============================================================

void testAlarm(char** args)
{
    if (args[1] == nullptr)
        return;

    // START

    if (strcmp(args[1], "start") == 0)
    {
        Serial0.println("[Test] Starting alarm override");

        lighting.beginAlarmOverride();

        return;
    }

    // STOP

    if (strcmp(args[1], "stop") == 0)
    {
        Serial0.println("[Test] Stopping alarm override");

        lighting.endAlarmOverride();

        return;
    }

    // ALARM MATRIX

    if (strcmp(args[1], "matrix") == 0)
    {
        if (
            args[2] == nullptr ||
            args[3] == nullptr ||
            args[4] == nullptr ||
            args[5] == nullptr
        )
        {
            Serial0.println(
                "[Test] Usage: alarm matrix <brightness> <R> <G> <B>"
            );

            return;
        }

        const uint8_t brightness = constrain(atoi(args[2]), 0, 255);
        const uint8_t red        = constrain(atoi(args[3]), 0, 255);
        const uint8_t green      = constrain(atoi(args[4]), 0, 255);
        const uint8_t blue       = constrain(atoi(args[5]), 0, 255);

        lighting.setAlarmMatrix(
            brightness,
            red,
            green,
            blue
        );

        Serial0.println("[Test] Alarm matrix updated");

        return;
    }

    // ALARM COB

    if (strcmp(args[1], "cob") == 0)
    {
        if (args[2] == nullptr)
            return;

        const uint8_t brightness =
            constrain(atoi(args[2]), 0, 255);

        lighting.setAlarmCob(brightness);

        Serial0.println("[Test] Alarm COB updated");

        return;
    }

    Serial0.println("[Test] Unknown ALARM command");
}

// ============================================================
// SET PARAMETER
// ============================================================

void setParameter(
    SettingsManager::Param param,
    int value
)
{
    const bool changed = settings.set(
        param,
        value
    );

    if (changed)
    {
        Serial0.print("[Settings] ");
        Serial0.print(settings.paramName(param));
        Serial0.print(" = ");
        Serial0.println(settings.get(param));
    }
    else
    {
        Serial0.println("[Settings] Value unchanged");
    }
}

// ============================================================
// STATUS
// ============================================================

void printStatus()
{
    Serial0.println();
    Serial0.println("=============== STATUS =================");

    Serial0.println();
    Serial0.println("[COB]");

    Serial0.print("Enabled: ");
    Serial0.println(
        settings.get(SettingsManager::Param::COB_ENABLED)
    );

    Serial0.print("Brightness: ");
    Serial0.println(
        settings.get(SettingsManager::Param::COB_BRIGHTNESS)
    );

    Serial0.print("Effect: ");
    Serial0.println(
        settings.get(SettingsManager::Param::COB_EFFECT)
    );

    Serial0.print("Speed: ");
    Serial0.println(
        settings.get(SettingsManager::Param::COB_SPEED)
    );

    Serial0.println();
    Serial0.println("[MATRIX]");

    Serial0.print("Enabled: ");
    Serial0.println(
        settings.get(SettingsManager::Param::MATRIX_ENABLED)
    );

    Serial0.print("Brightness: ");
    Serial0.println(
        settings.get(SettingsManager::Param::MATRIX_BRIGHTNESS)
    );

    Serial0.print("Effect: ");
    Serial0.println(
        settings.get(SettingsManager::Param::MATRIX_EFFECT)
    );

    Serial0.print("Speed: ");
    Serial0.println(
        settings.get(SettingsManager::Param::MATRIX_SPEED)
    );

    Serial0.println();
    Serial0.println("[ALARM]");

    Serial0.print("Override: ");
    Serial0.println(
        lighting.isAlarmOverrideActive() ? "ACTIVE" : "INACTIVE"
    );

    Serial0.println();
    Serial0.println("========================================");
    Serial0.println();
}
