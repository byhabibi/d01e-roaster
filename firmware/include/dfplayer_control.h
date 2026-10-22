#pragma once
#include <Arduino.h>

void dfplayerBegin();
void dfplayerSetVolume(uint8_t vol);
void dfplayerPlayTrack(uint16_t track);
void dfplayerStop();
