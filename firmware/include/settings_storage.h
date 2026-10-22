#pragma once
#include <Arduino.h>

void settingsBegin();
bool settingsGetUseFahrenheit();
void settingsSetUseFahrenheit(bool fahrenheit);
uint32_t settingsGetBatchCount();
void settingsIncrementBatch();
void settingsFormatBatch(char *out, size_t len);
