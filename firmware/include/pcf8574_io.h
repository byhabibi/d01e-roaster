#pragma once
#include <Arduino.h>

// Shared PCF8574 @ 0x20 — P2 RS485 DE, P3 cooling (D01E_Electrical §6.2)
void pcf8574Begin();
void pcf8574WritePort(uint8_t shadow);
uint8_t pcf8574ReadShadow();

void pcf8574SetPin(uint8_t pin, bool high);
void pcf8574SetRs485RxMode();
void pcf8574SetRs485TxMode();

void pcf8574SetCooling(bool on);
