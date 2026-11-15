#pragma once

#include <Arduino.h>

// ============================================================
// TIMER DATA
// ============================================================

struct Timer
{
    String id;
    String name;

    uint32_t durationMs = 0;

    String matrixEffect;
    String finishMatrixEffect;

    String cobEffect;
    String audioEffect;
    String finishAudioEffect;
};