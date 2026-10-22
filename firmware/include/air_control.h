#pragma once
#include <Arduino.h>

void airBegin();
void airSetEnabled(bool enabled);
void airSetOverride(bool on, uint8_t pct);
void airPoll(uint32_t nowMs, void (*onDisplayPct)(uint8_t pct));
uint8_t airGetPct();
void airApplyPanelGate();
