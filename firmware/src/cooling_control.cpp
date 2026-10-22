#include "cooling_control.h"
#include "pcf8574_io.h"
#include "config.h"

static bool s_coolingOn = false;

void coolingBegin() {
    pcf8574Begin();
    pcf8574SetRs485RxMode();
    s_coolingOn = false;
    pcf8574SetCooling(false);
}

void coolingSet(bool on) {
    if (on == s_coolingOn) {
        return;
    }
    s_coolingOn = on;
    pcf8574SetCooling(on);
    Serial.printf("[COOL] fan %s\n", on ? "ON" : "OFF");
}

bool coolingIsOn() {
    return s_coolingOn;
}
