#include "color_sensor_control.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_TCS34725.h>

static Adafruit_TCS34725 s_tcs(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
static bool s_present = false;
static bool s_enabled = false;

void colorSensorBegin() {
    s_present = s_tcs.begin(TCS34725_I2C_ADDR);
    if (!s_present) {
        Serial.println(F("[COLOR] TCS34725 not found (J_COLOR DNP?)"));
        return;
    }
    colorSensorSet(false);
    Serial.println(F("[COLOR] TCS34725 ready"));
}

void colorSensorSet(bool on) {
    if (!s_present) {
        if (on) {
            Serial.println(F("[COLOR] sensor not installed"));
        }
        return;
    }
    if (on == s_enabled) {
        return;
    }
    s_enabled = on;
    if (on) {
        s_tcs.enable();
    } else {
        s_tcs.disable();
    }
    Serial.printf("[COLOR] sensor %s\n", on ? "ON" : "OFF");
}

bool colorSensorIsOn() {
    return s_enabled;
}

bool colorSensorPresent() {
    return s_present;
}
