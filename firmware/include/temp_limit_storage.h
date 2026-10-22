#pragma once
#include <Arduino.h>

void tempLimitBegin();
uint16_t tempLimitGet();
bool tempLimitSet(uint16_t celsius);
bool tempLimitIsSafe(float btC);
