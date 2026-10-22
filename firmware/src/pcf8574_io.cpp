#include "pcf8574_io.h"
#include "config.h"
#include <Wire.h>

// PCF8574: 1 = pin high-Z (default pulled high), 0 = sink LOW
static uint8_t s_shadow = 0xFF;

static void writePort() {
    Wire.beginTransmission(PCF8574_ADDR);
    Wire.write(s_shadow);
    Wire.endTransmission();
}

void pcf8574Begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    s_shadow = 0xFF;
    writePort();
}

void pcf8574WritePort(uint8_t shadow) {
    s_shadow = shadow;
    writePort();
}

uint8_t pcf8574ReadShadow() {
    return s_shadow;
}

void pcf8574SetPin(uint8_t pin, bool high) {
    if (pin > 7) {
        return;
    }
    if (high) {
        s_shadow |= (uint8_t)(1U << pin);
    } else {
        s_shadow &= (uint8_t) ~(1U << pin);
    }
    writePort();
}

void pcf8574SetRs485RxMode() {
    // P2 HIGH → Q6 inverter → MAX3485 DE low → receive
    pcf8574SetPin(PCF8574_PIN_RS485_DE, true);
}

void pcf8574SetRs485TxMode() {
    pcf8574SetPin(PCF8574_PIN_RS485_DE, false);
}

void pcf8574SetCooling(bool on) {
    if (on) {
        s_shadow &= (uint8_t) ~(1U << PCF8574_PIN_COOL);
    } else {
        s_shadow |= (uint8_t)(1U << PCF8574_PIN_COOL);
    }
    writePort();
}
