#pragma once

#include <Arduino.h>

#include "./managers/CobLedManager.h"
#include "./managers/LedMatrixManager.h"

// ============================================================
// LIGHT SYSTEM
//
// High-level lighting controller.
//
// Owns:
//   - COB lighting through CobLedManager
//   - LED Matrix through LedMatrixManager
//
// App / WebServer / Bluetooth / AlarmManager should work
// with LightSystem instead of accessing individual managers.
//
// ============================================================

class LightSystem
{
public:

    // ========================================================
    // OUTPUT
    // ========================================================

    enum class Output : uint8_t
    {
        None   = 0,
        Matrix = 1,
        Cob    = 2,
        Both   = 3
    };

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    LightSystem(
        CobLedManager& cob,
        LedMatrixManager& matrix
    );

    // ========================================================
    // LIFECYCLE
    // ========================================================

    bool begin();

    void update();

    // ========================================================
    // MASTER
    // ========================================================

    void setEnabled(bool enabled);

    bool isEnabled() const;

    // ========================================================
    // OUTPUT
    // ========================================================

    void setOutput(Output output);

    Output output() const;

    bool matrixEnabled() const;

    bool cobEnabled() const;

    void enableMatrix(bool enabled);

    void enableCob(bool enabled);

    // ========================================================
    // GLOBAL BRIGHTNESS
    //
    // 0..255
    // ========================================================

    void setBrightness(uint8_t brightness);

    uint8_t brightness() const;

    // ========================================================
    // MATRIX
    // ========================================================

    LedMatrixManager& matrix();

    const LedMatrixManager& matrix() const;

    // ========================================================
    // COB
    // ========================================================

    CobLedManager& cob();

    const CobLedManager& cob() const;

    // ========================================================
    // MATRIX BRIGHTNESS
    // ========================================================

    void setMatrixBrightness(uint8_t brightness);

    uint8_t matrixBrightness() const;

    // ========================================================
    // COB BRIGHTNESS
    // ========================================================

    void setCobBrightness(uint8_t brightness);

    uint8_t cobBrightness() const;

    // ========================================================
    // ALL LIGHTS
    // ========================================================

    void on();

    void off();

    void toggle();

    // ========================================================
    // MATRIX
    // ========================================================

    void matrixOn();

    void matrixOff();

    // ========================================================
    // COB
    // ========================================================

    void cobOn();

    void cobOff();

    // ========================================================
    // STATE
    // ========================================================

    bool isMatrixOn() const;

    bool isCobOn() const;

private:

    CobLedManager& _cob;
    LedMatrixManager& _matrix;

    bool _initialized;

    bool _enabled;

    bool _matrixEnabled;
    bool _cobEnabled;

    uint8_t _brightness;

    uint8_t _matrixBrightness;
    uint8_t _cobBrightness;

    // ========================================================
    // INTERNAL
    // ========================================================

    void applyState();

    void applyMatrixState();

    void applyCobState();
};