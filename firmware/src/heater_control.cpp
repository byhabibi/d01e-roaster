#include "heater_control.h"
#include "panel_interlock.h"
#include "config.h"

static uint8_t s_heatPct = 0;
static bool s_outputOn = false;

#if HEATER_HW_V36
static uint32_t s_burstAnchorMs = 0;
#endif

void heaterBegin() {
    pinMode(PIN_HEATER_EN, OUTPUT);
    digitalWrite(PIN_HEATER_EN, LOW);
    s_heatPct = 0;
    s_outputOn = false;
#if HEATER_HW_V36
    s_burstAnchorMs = millis();
#endif
}

void heaterSetPct(uint8_t pct) {
    if (pct > 100) {
        pct = 100;
    }
    s_heatPct = pct;

#if !HEATER_HW_V36
    if (pct > 0 && pct < 100) {
        Serial.printf("[HEAT] %u%% UI only — HW v3.5 aktif penuh hanya 0%% / 100%%\n", pct);
    }
#endif
}

uint8_t heaterGetPct() {
    return s_heatPct;
}

bool heaterOutputOn() {
    return s_outputOn;
}

static void driveHeaterPin(bool on) {
    if (on == s_outputOn) {
        return;
    }
    s_outputOn = on;
    digitalWrite(PIN_HEATER_EN, on ? HIGH : LOW);
    Serial.printf("[HEAT] GPIO13 %s (setpoint %u%%)\n", on ? "ON" : "OFF", s_heatPct);
}

#if HEATER_HW_V36
static bool burstWindowOn(uint32_t nowMs) {
    if (s_heatPct >= 100) {
        return true;
    }
    if (s_heatPct == 0) {
        return false;
    }

    uint32_t onMs = (uint32_t)s_heatPct * HEAT_CYCLE_MS / 100;
    if (onMs < HEAT_MIN_ON_MS) {
        onMs = HEAT_MIN_ON_MS;
    }
    if (onMs > HEAT_CYCLE_MS) {
        onMs = HEAT_CYCLE_MS;
    }

    uint32_t pos = nowMs % HEAT_CYCLE_MS;
    return pos < onMs;
}
#endif

void heaterApply(bool safetyOk) {
    if (!panelHeaterEnabled() || !safetyOk || s_heatPct == 0) {
        driveHeaterPin(false);
        return;
    }

#if HEATER_HW_V36
    driveHeaterPin(burstWindowOn(millis()));
#else
    driveHeaterPin(s_heatPct >= 100);
#endif
}
