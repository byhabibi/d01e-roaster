#pragma once
#include <Arduino.h>

// HEATER_HW_V36=1 → burst fire (GPIO13 SSR + PCF8574 K1_HOLD). Default 0 = v3.5 PCB.
#ifndef HEATER_HW_V36
#define HEATER_HW_V36 0
#endif

void heaterBegin();
void heaterSetPct(uint8_t pct);
uint8_t heaterGetPct();
bool heaterOutputOn();

// safetyOk: TC bean valid, no thermal fault (extend later)
void heaterApply(bool safetyOk);
