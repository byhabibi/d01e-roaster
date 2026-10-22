#include "air_control.h"
#include "panel_interlock.h"
#include "config.h"
static uint8_t s_airPct = 0;
static uint32_t s_lastPollMs = 0;
static bool s_airEnabled = false;
static bool s_airOverride = false;
static uint8_t s_airOverridePct = 0;

static int readPotAdcFiltered() {
    long sum = 0;
    for (int i = 0; i < AIR_ADC_SAMPLES; i++) {
        sum += analogRead(PIN_POT_AIR);
    }
    return (int)(sum / AIR_ADC_SAMPLES);
}

static uint8_t adcToPct(int adc) {
    if (adc <= (int)POT_ADC_MIN) {
        return 0;
    }
    if (adc >= (int)POT_ADC_MAX) {
        return 100;
    }
    uint32_t span = (uint32_t)POT_ADC_MAX - (uint32_t)POT_ADC_MIN;
    return (uint8_t)(((uint32_t)(adc - (int)POT_ADC_MIN) * 100U) / span);
}

static void applyExhaustPwm(uint8_t pct) {
    uint32_t maxDuty = (1U << AIR_LEDC_RES_BITS) - 1U;
    uint32_t duty = (uint32_t)pct * maxDuty / 100U;
    ledcWrite(AIR_LEDC_CHANNEL, duty);
}

static void setAirPct(uint8_t pct) {
    if (!panelAirEnabled()) {
        pct = 0;
    }
    if (pct == s_airPct) {
        return;
    }    s_airPct = pct;
    applyExhaustPwm(pct);
    Serial.printf("[AIR] %u%% (GPIO2 PWM)\n", pct);
}

void airBegin() {
    pinMode(PIN_POT_AIR, INPUT);

    analogReadResolution(12);
#if defined(CONFIG_IDF_TARGET_ESP32)
    analogSetPinAttenuation(PIN_POT_AIR, ADC_11db);
#endif

    ledcSetup(AIR_LEDC_CHANNEL, AIR_LEDC_FREQ_HZ, AIR_LEDC_RES_BITS);
    ledcAttachPin(PIN_EXHAUST_PWM, AIR_LEDC_CHANNEL);
    ledcWrite(AIR_LEDC_CHANNEL, 0);

    s_airPct = 0;
    s_lastPollMs = 0;
    s_airEnabled = false;
}

void airSetEnabled(bool enabled) {
    s_airEnabled = enabled;
    if (!enabled && !s_airOverride) {
        setAirPct(0);
    }
}

void airSetOverride(bool on, uint8_t pct) {
    s_airOverride = on;
    if (on) {
        if (pct > 100) {
            pct = 100;
        }
        s_airOverridePct = pct;
        setAirPct(pct);
    } else if (!s_airEnabled) {
        setAirPct(0);
    }
}

uint8_t airGetPct() {
    return s_airPct;
}

void airPoll(uint32_t nowMs, void (*onDisplayPct)(uint8_t pct)) {
    if (s_airOverride) {
        if (s_airPct != s_airOverridePct) {
            setAirPct(s_airOverridePct);
        }
        return;
    }

    if (!s_airEnabled) {
        if (s_airPct != 0) {
            setAirPct(0);
        }
        return;
    }

    if (s_lastPollMs != 0 && (nowMs - s_lastPollMs) < AIR_POLL_MS) {
        return;
    }
    s_lastPollMs = nowMs;

    uint8_t pct = adcToPct(readPotAdcFiltered());
    if (pct != s_airPct) {
        setAirPct(pct);
        if (onDisplayPct) {
            onDisplayPct(pct);
        }
    }
}

void airApplyPanelGate() {
    if (!panelAirEnabled() && s_airPct != 0) {
        s_airPct = 0;
        applyExhaustPwm(0);
        Serial.println(F("[AIR] forced OFF — panel AIR button"));
    }
}
