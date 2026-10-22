#include "panel_interlock.h"
#include "config.h"

static bool s_drumOn = false;
static bool s_heaterOn = false;
static bool s_airOn = false;
static bool s_safe = true;

static bool readBtnActive(uint8_t pin) {
    int level = digitalRead(pin);
#if PANEL_BTN_ACTIVE_LOW
    return level == LOW;
#else
    return level == HIGH;
#endif
}

static bool readEstopSafe() {
#if PANEL_ESTOP_ENABLED
    int level = digitalRead(PIN_ESTOP_SENSE);
#if PANEL_ESTOP_UNSAFE_HIGH
    return level == LOW;
#else
    return level == HIGH;
#endif
#else
    (void)PIN_ESTOP_SENSE;
    return true;
#endif
}

void panelBegin() {
    pinMode(PIN_BTN_DRUM, INPUT_PULLUP);
    pinMode(PIN_BTN_HEATER, INPUT_PULLUP);
    pinMode(PIN_BTN_AIR, INPUT_PULLUP);
#if PANEL_ESTOP_ENABLED
    pinMode(PIN_ESTOP_SENSE, INPUT_PULLUP);
#endif

    pinMode(PIN_DRUM_EN, OUTPUT);
    digitalWrite(PIN_DRUM_EN, LOW);

    s_drumOn = false;
    s_heaterOn = false;
    s_airOn = false;
    s_safe = true;
    panelPoll();
}

void panelPoll() {
    bool safe = readEstopSafe();
    if (safe != s_safe) {
        s_safe = safe;
        Serial.printf("[PANEL] E-Stop %s\n", safe ? "OK" : "TRIPPED");
    }

    bool drum = safe && readBtnActive(PIN_BTN_DRUM);
    if (drum != s_drumOn) {
        s_drumOn = drum;
        Serial.printf("[PANEL] DRUM button %s\n", drum ? "ON" : "OFF");
    }
    digitalWrite(PIN_DRUM_EN, drum ? HIGH : LOW);

    bool heater = safe && readBtnActive(PIN_BTN_HEATER);
    if (heater != s_heaterOn) {
        s_heaterOn = heater;
        Serial.printf("[PANEL] HEATER button %s\n", heater ? "ON" : "OFF");
    }

    bool air = safe && readBtnActive(PIN_BTN_AIR);
    if (air != s_airOn) {
        s_airOn = air;
        Serial.printf("[PANEL] AIR button %s\n", air ? "ON" : "OFF");
    }
}

bool panelIsSafe() {
    return s_safe;
}

bool panelDrumEnabled() {
    return s_drumOn;
}

bool panelHeaterEnabled() {
    return s_heaterOn;
}

bool panelAirEnabled() {
    return s_airOn;
}
